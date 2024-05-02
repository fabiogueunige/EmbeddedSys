/*
 * File:   main.c
 * Author: Chiappe, Di Donna, Guelfi, Utegaliyeva
 *
 * Created on April 23, 2024, 12:48 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"
#include <stdio.h>


#define URTBR 468 // uart baud rate
#define DIMSPI 5 // counter 
#define MAGFREQ 4
#define UARTFREQ 20
#define DIMUB 60

void myfunction(int ,int ); // function that use 7 ms to be completed
int spi_write (unsigned int addr); // SPI writing function
void print(int); // printing 
void printGrad(int); // printing 
void printImu(int, int, int );
int magnAcquisition (int , int , int); // function to acquire x,y,x data from magnetometer
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void);

/*
int spw (unsigned int addr, int value)
{
    int data; // the value to return to the function
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd (becouse space fo only a char)
    data = SPI1BUF; // the byte sent from the slave
    while (SPI1STATbits.SPITBF == 1); // wait until tx buffer is not full
    SPI1BUF = value;
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd
    data = SPI1BUF;
    // U1TXREG = data;
    
    return data;
}*/
// global variables (indixes)
int cnt = 0;
int ind_uart = 0, ind_spi = 0, ind_tr = 0, ind_buf_write = 0, ind_buf_read = 0;
int x[DIMSPI], y[DIMSPI], z[DIMSPI]; // value to save from the magnetometer
int xavg = 0, yavg = 0, zavg = 0;
char UBuffer[DIMUB];


int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // SPI setting
    SPI1CON1bits.MSTEN = 1; // master mode 
    SPI1CON1bits.MODE16 = 0; // 8 bit mode
    SPI1CON1bits.PPRE =  0;// setting the primary prescaler (pg 342)
    SPI1CON1bits.SPRE = 5; // setting the secondary prescaler
    SPI1CON1bits.CKP = 1; // complete!!!!!!
    SPI1STATbits.SPIEN = 1; // enable the SPI 
    
    
    // Remap for SPI (Magnetometer)
    // Setting the input/output ode
    TRISAbits.TRISA1 = 1; // set the pin RA1/RPI17 as input (because MISO)
    TRISFbits.TRISF12 = 0; // set the pin RF12/RP108 as output (because CLOCK)
    TRISFbits.TRISF13 = 0; // set the pin RF13/RP109 as output (because MOSI)
    
    // Functionality associated to the pin setting
    RPINR20bits.SDI1R = 0b0010001; // (functionality = pin) RPI117 associated to the SPI Data Input (SDI1)
    RPOR12bits.RP109R = 0b000101; // (pin = functionality) RP109 tied to SPI1 Data output (SDO1) 
    RPOR11bits.RP108R = 0b000110; // RP108 tied to SPI1 Clock output (SCK1)    
    
    // Selecting all as inactive to have only one chip select active, so selected to 0.
    TRISBbits.TRISB3 = 0; // Selecting the port of CS1 (accelerometer) as output
    TRISBbits.TRISB4 = 0; // Selecting the port of CS2 (gyroscope) as output
    TRISDbits.TRISD6 = 0; // Selecting the port of CS3 (magnetometer) as output
    LATBbits.LATB3 = 1; // accelerometer set to 1 (inactive)
    LATBbits.LATB4 = 1; // gyroscope set to 1 (inactive)
    LATDbits.LATD6 = 1; // magnetometer set to 1 (inactive)
    
    // UART Setting configuration
    // U1BRG = round((FCY /(16LL * BAUD_RATE))-1); 
    U1BRG = URTBR; // setting the baud rate register directly in integer to avoid floating value
    U1MODEbits.UARTEN = 1; // enable UART
    
    // Pin remap
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) 
    RPINR18bits.U1RXR = 0b1001011; // Receiver functionality to the register
    
    // Enabling the transmission
    U1STAbits.UTXEN = 1; // UART transmission enable
    
    // Magnetometer activation
    // Going on sleep mode and waiting before to go on active (needed for SPI)
    LATDbits.LATD6 = 0; // To advise we are sending data
    // spw(0x4B, 0x01);
    spi_write(0x4B);
    spi_write(0x01);
    LATDbits.LATD6 = 1; // To alert the end of the communication
    tmr_wait_ms (TIMER3, 2);
    
    // Going on active mode changing the magnetometer output data rate
    LATDbits.LATD6 = 0;
    // spw(0x4C, 0b00110000);
    spi_write(0x4C);
    spi_write(0b00110000);
    LATDbits.LATD6 = 1;
    tmr_wait_ms (TIMER3, 2);
        
    // timer setup
    tmr_setup_period (TIMER2, 10); // for main
    // timer 3 for the supsension // to go in active mode
    // timer 5 used for the function of 7 ms
    
    // Setting of the interrupt
    IFS0bits.U1TXIF = 0; // resetting U1TX interrupt flag
    IEC0bits.U1TXIE = 1; // enabling U1TX interrupt 
   
    
    LATDbits.LATD6 = 0;
    spi_write(0x42 | 0x80); // So next time we will read the lsb of x magnetometer
    LATDbits.LATD6 = 1;
    
    while(1)
    {
        
        myfunction(TIMER5, 7);
        
        if (cnt % MAGFREQ == 0) // read the magnetometer
        {
            
            // disable the interrupts while acquiring data from spi
            
            // acquiring x values from the magnetometer
            x[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8);
                       
            // acquiring y values from the magnetometer
            y[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8);
 
            // acquiring z values from the magnetometer
            z[ind_spi] = magnAcquisition(0x00, 0x42 | 0x80, 0xFE);
            
            // control on spi index of the circular buffer
            if (ind_spi >= DIMSPI)
            {
                ind_spi = 0;
            }
            else {
                ind_spi++;
            }
                     
            
        }
        if ((cnt % UARTFREQ == 0) && (cnt != 0))
        {
            cnt = 0;
            for (int i = 0; i < DIMSPI; i++)
            {
                xavg += x[i];
                yavg += y[i];
                zavg += z[i];
            }
            xavg /= DIMSPI;
            yavg /= DIMSPI;
            zavg /= DIMSPI;
            printImu(xavg, yavg, zavg);
            
            // implement print on circular buffer
        }
        cnt ++;
        while(!tmr_wait_period(TIMER2));
    }
    return 0;
}

void myfunction(int tmr, int tiempo)
{
    // wait 7 ms with Timer given
    tmr_wait_ms(tmr, tiempo);
}

int spi_write (unsigned int addr)
{
    int data; // the value to return to the function
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrived
    data = SPI1BUF; // the byte sent from the slave of the previous request
        
    return data;
}

void printImu(int x, int y, int z)
{
    char buff[30];

    sprintf(buff,"$MAG,%d,%d,%d*", x, y, z); 
    
    // copy the men value comes from spi inside the circular buffer
    for(int i = 0; buff[i] != 0; i++)
    {
        UBuffer[ind_buf_write] = buff[i];
        //while (U1STAbits.UTXBF != 0); // ask if we can use this register
        //U1TXREG = buff[i];
        
        // every char write in the buffer augment the index
        if (ind_buf_write >= DIMUB)
        {
            ind_buf_write ++;
        }
        else
        {
            ind_buf_write = 0;
        }    
    }   
    // send the value trough UART  
    if (U1STAbits.UTXBF == 0){
        IFS0bits.U1TXIF = 1;   
    }
}

int magnAcquisition (int addr, int next_addr, int mask_lsb)
{
    int msb = 0, lsb = 0, full = 0;
    LATDbits.LATD6 = 0;
    lsb = spi_write(addr);
    // LATDbits.LATD6 = 1;
    // LATDbits.LATD6 = 0;
    msb = spi_write(next_addr);
    LATDbits.LATD6 = 1;
    /*while (U1STAbits.UTXBF != 0); 
    U1TXREG = 'M';
    print(msb);
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';
    
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = 'L';
    print(lsb);*/
    lsb = lsb & mask_lsb; //  mask for lsb to cancel the bits
    /*
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';
     * */
    
    msb = (msb << 8); // msb shifted of 8 of left
    
    full = msb | lsb; // the union of lsb and msb shifted
    full = full / 8; // divide the value by 8
    /*while (U1STAbits.UTXBF != 0); 
    U1TXREG = 'F';
    print(full);
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';*/
    return full;
}

void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void)
{   
     //set the flag to zero
    IFS0bits.U1TXIF = 0;
    for (int i = 0; UBuffer[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = UBuffer[ind_buf_read];
        // 
        if (ind_buf_read >= DIMUB)
        {
            ind_buf_read ++;
        }
        else
        {
            ind_buf_read = 0;
        }    
    }
    
    
    /*
    grad = atan2 (yavg, xavg);
    printImu(xavg, yavg, zavg);
    printGrad(grad);*/
}

void printGrad(int value)
{
    char buff[15];

    sprintf(buff,"$YAW,%d*", value); 
    
    for(int i =0; buff[i] != 0; i++)
    {
        UBuffer[ind_buf_write] = buff[i];
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buff[i];
        if (ind_buf_write >= DIMUB)
        {
            ind_buf_write ++;
        }
        else
        {
            ind_buf_write = 0;
        }    
    }
}

void print(int stamp)
{
    char buff[20];
    sprintf(buff,"%d", stamp);
    for (int i = 0; buff[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buff [i];
    }
}