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
#define DIMUB 50

void myfunction(int ,int ); // function that use 7 ms to be completed
int spi_write (unsigned int addr); // SPI writing function
void printGrad(float); // printing 
void printImu(float, float, float);
float magnAcquisition (int , int , int, int); // function to acquire x,y,x data from magnetometer
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void);


void spi_config();
void init_config();
void tr_uart_config();
void chip_selecting();

// global variables (indixes)
int cnt = 0;
int ind_spi = 0, ind_buf_write = 0, ind_buf_read = 0;
float x[DIMSPI], y[DIMSPI], z[DIMSPI]; // value to save from the magnetometer
float xavg = 0, yavg = 0, zavg = 0;
char UBuffer[DIMUB];


int main(void) {
     
    init_config();
    
    spi_config();
    
    chip_selecting();

    
    // Remap for SPI (Magnetometer)
    // Setting the input/output ode
    TRISAbits.TRISA1 = 1; // set the pin RA1/RPI17 as input (because MISO)
    TRISFbits.TRISF12 = 0; // set the pin RF12/RP108 as output (because CLOCK)
    TRISFbits.TRISF13 = 0; // set the pin RF13/RP109 as output (because MOSI)
    
    // Functionality associated to the pin setting
    RPINR20bits.SDI1R = 0b0010001; // (functionality = pin) RPI117 associated to the SPI Data Input (SDI1)
    RPOR12bits.RP109R = 0b000101; // (pin = functionality) RP109 tied to SPI1 Data output (SDO1) 
    RPOR11bits.RP108R = 0b000110; // RP108 tied to SPI1 Clock output (SCK1)    
    
    tr_uart_config();
    
    // Pin remap
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) 
    RPINR18bits.U1RXR = 0b1001011; // Receiver functionality to the register
    
    // Enabling the transmission and uart
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // UART transmission enable
    // Setting of the interrupt for trasmission
    IFS0bits.U1TXIF = 0; // resetting U1TX interrupt flag
    IEC0bits.U1TXIE = 0; // disabling U1TX interrupt 
    
    // Magnetometer activation
    // Going on sleep mode and waiting before to go on active (needed for SPI)
    LATDbits.LATD6 = 0; // To advise we are sending data
    spi_write(0x4B);
    spi_write(0x01);
    LATDbits.LATD6 = 1; // To alert the end of the communication  
    tmr_wait_ms (TIMER3, 2);
    
    // Going on active mode changing the magnetometer output data rate
    LATDbits.LATD6 = 0;
    spi_write(0x4C);
    spi_write(0b00110000);
    LATDbits.LATD6 = 1;
    tmr_wait_ms (TIMER3, 2);
    
    float grad = 0;
    
    // timer setup
    tmr_setup_period (TIMER2, 10); // for main
    // timer 3 for the supsension // to go in active mode
    // timer 5 used for the function of 7 ms
    int chip_val = 0;
    LATDbits.LATD6 = 0;
    spi_write(0x40 | 0x80);
    chip_val = spi_write(0x00);
    LATDbits.LATD6 = 1;
    while (U1STAbits.UTXBF != 0); 
    
    IEC0bits.U1TXIE = 1; // activate the interrupt for the uart trasmission
        
    while(1)
    {
        
        myfunction(TIMER5, 7);
        
        if (cnt % MAGFREQ == 0) // read the magnetometer every 25Hz, 40ms
        {
            LATDbits.LATD6 = 0;
            spi_write(0x42 | 0x80); // So next time we will read the lsb of x magnetometer
            
            // acquiring x values from the magnetometer
            x[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8, 8);
            
                       
            // acquiring y values from the magnetometer
            y[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8, 8);
 
            // acquiring z values from the magnetometer
            z[ind_spi] = magnAcquisition(0x00, 0x00, 0xFE, 2);
            LATDbits.LATD6 = 1;

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
            
            // gradient computation
            grad = atan2 (xavg, yavg);
            grad = (grad * 180.0) / M_PI;
            printGrad(grad);
            
        }
        cnt ++;
        // check if deadline missed
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

void printImu(float x, float y, float z)
{
    char buff[30];

    sprintf(buff,"$MAG,%.2f,%.2f,%.2f*", x, y, z); 
    
    // copy the men value comes from spi inside the circular buffer
    for(int i = 0; buff[i] != 0; i++)
    {
        UBuffer[ind_buf_write] = buff[i];
        
        // every char write in the buffer augment the index
        if (ind_buf_write >= DIMUB)
        {
            ind_buf_write = 0;
        }
        else 
        {
            ind_buf_write++;
        }    
    }   
    // send the value trough UART  
    if (U1STAbits.UTXBF == 0){
        IFS0bits.U1TXIF = 1; 
        //IEC0bits.U1TXIE = 1;
    }
}

float magnAcquisition (int addr, int next_addr, int mask_lsb, int divider)
{
    int msb = 0, lsb = 0, full = 0;


    lsb = spi_write(addr);
    msb = spi_write(next_addr);


    lsb = lsb & mask_lsb; //  mask for lsb to cancel the bits
    
    msb = (msb << 8); // msb shifted of 8 of left
    
    full = msb | lsb; // the union of lsb and msb shifted
    full = full / divider; // divide the value by 8
    
    return full;
}

void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void)
{   
     //set the flag to zero
    IFS0bits.U1TXIF = 0;
    
    while((U1STAbits.UTXBF != 1) && (ind_buf_read != ind_buf_write))
    {
        U1TXREG = UBuffer[ind_buf_read];
        
        if (ind_buf_read >= DIMUB)
        {
            ind_buf_read = 0;
            
        }
        else if (ind_buf_read < DIMUB )
        {
            ind_buf_read++;
        }
    }
    /*else{
        IEC0bits.U1TXIE = 0;
    }*/
}



void spi_config()
{
    // SPI setting
    SPI1CON1bits.MSTEN = 1; // master mode 
    SPI1CON1bits.MODE16 = 0; // 8 bit mode
    SPI1CON1bits.PPRE =  0;// setting the primary prescaler (pg 342)
    SPI1CON1bits.SPRE = 5; // setting the secondary prescaler
    SPI1CON1bits.CKP = 1; // complete!!!!!!
    SPI1STATbits.SPIEN = 1; // enable the SPI 
}

void tr_uart_config()
{
    // UART Setting configuration
    // U1BRG = round((FCY /(16LL * BAUD_RATE))-1); 
    U1BRG = URTBR; // setting the baud rate register directly in integer to avoid floating value
    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 0;
}

void chip_selecting()
{  
    // Selecting all as inactive to have only one chip select active, so selected to 0.
    TRISBbits.TRISB3 = 0; // Selecting the port of CS1 (accelerometer) as output
    TRISBbits.TRISB4 = 0; // Selecting the port of CS2 (gyroscope) as output
    TRISDbits.TRISD6 = 0; // Selecting the port of CS3 (magnetometer) as output
    LATBbits.LATB3 = 1; // accelerometer set to 1 (inactive)
    LATBbits.LATB4 = 1; // gyroscope set to 1 (inactive)
    LATDbits.LATD6 = 1; // magnetometer set to 1 (inactive)
}

void init_config(){
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
}

void printGrad(float value)
{
    char buffer[15];

    sprintf(buffer,"$YAW,%.2f", value); 
    
    for(int i =0; buffer[i] != 0; i++)
    {
        UBuffer[ind_buf_write] = buffer[i];
        
        // every char write in the buffer augment the index
        if (ind_buf_write >= DIMUB)
        {
            ind_buf_write = 0;
        }
        else 
        {
            ind_buf_write++;
        }   
    }
}
