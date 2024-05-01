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
#define CNTSTOP 5 // counter 

void myfunction(int ,int ); // function that use 7 ms to be completed
int spi_write (unsigned int, int , int*); // SPI writing function
void print(int); // printing 
void printGrad(int); // printing 
void printImu(int, int, int );
int magnAcquisition (int , int , int , int ); // function to acquire x,y,x data from magnetometer
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void);

// global variables
int count = 0;
int x[CNTSTOP], y[CNTSTOP], z[CNTSTOP]; // value to save to compute the mean



int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // SPI setting
    SPI1CON1bits.MSTEN = 1; // master mode 
    SPI1CON1bits.MODE16 = 0; // 8 bit mode
    SPI1CON1bits.PPRE =  0;// setting the primary prescaler (pg 342)
    SPI1CON1bits.SPRE = 5; // setting the secondary prescaler
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
    // variable definition
    int trash = 0;
    int xtmp = 0, ytmp = 0, ztmp = 0;

    // Going on sleep mode and waiting before to go on active (needed for SPI)
    LATDbits.LATD6 = 0; // To advise we are sending data
    spi_write(0x4B, 0x01, &trash);
    LATDbits.LATD6 = 1; // To alert the end of the communication
    tmr_wait_ms (TIMER3, 2);
    
    // Going on active mode changing the magnetometer output data rate
    LATDbits.LATD6 = 0;
    spi_write(0x4C , 0b00110, &trash); // chiedere
    LATDbits.LATD6 = 1;
    tmr_wait_ms (TIMER3, 2);
        
    // timer setup
    tmr_setup_period (TIMER1, 200); // timer 1 interrpt for UART
    tmr_setup_period (TIMER2, 10); // for main
    // timer 3 for the supsension // to go in active mode
    tmr_setup_period (TIMER4, 40); // timer to read from magnetometer
    // timer 5 used for the function
    
    // Setting of the interrupt
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    
    IFS1bits.T4IF = 1; // to have the read of the magnetometer at the first cycle
    
    while(1)
    {
        myfunction(TIMER5, 7);
        
        if (IFS1bits.T4IF == 1) // read the magnetometer
        {
            IFS1bits.T4IF = 0; // resetting the tmer flag
            
            count ++;
            // disablle the interrupts while acquiring data from spi
            IEC0bits.T1IE = 0;
            // acquiring x values from the magnetometer
            LATDbits.LATD6 = 0;
            x[count] = magnAcquisition(0x42, 0x00, 0x80, 0xF8);
            LATDbits.LATD6 = 1;
            
            // acquiring y values from the magnetometer
            LATDbits.LATD6 = 0;
            y[count] = magnAcquisition(0x44, 0x00, 0x80, 0xF8);
            LATDbits.LATD6 = 1;
            
            // acquiring z values from the magnetometer
            LATDbits.LATD6 = 0; 
            z[count] = magnAcquisition(0x46, 0x00, 0x80, 0xFE);
            LATDbits.LATD6 = 1;
            
            
            // Modifing the sum of the axis
            /*
            x[count] = xtmp;
            y[count] = ytmp;
            z[count] = ztmp;*/
            IEC0bits.T1IE = 1;
            
            // printImu(x[count], y[count], z[count]);
        }
        if (count >= CNTSTOP)
        {
            count  = 0;            
        }        
    }
    return 0;
}

void myfunction(int tmr, int tiempo)
{
    // wait 7 ms with Timer given
    tmr_wait_ms(tmr, tiempo);
}

int spi_write (unsigned int addr, int value, int *trash_next)
{
    int data; // the value to return to the function
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrived
    *trash_next = SPI1BUF; // the byte sent from the slave of the previous request
    while (SPI1STATbits.SPITBF == 1); // wait until tx buffer is not full
    SPI1BUF = value; // writing a value or giving the address
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrived
    data = SPI1BUF; // taking the information from the slave of the SPI
    
    return data;
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

void printImu(int x, int y, int z)
{
    char buff[100];

    sprintf(buff,"$MAG,%d,%d,%d*", x, y, z); 
    
    for(int i =0; buff[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buff[i];
    }
}

int magnAcquisition (int addr, int next_addr, int mask_ad, int mask_lsb)
{
    int msb = 0, lsb = 0, full = 0;
    lsb = spi_write(addr | mask_ad, next_addr, &msb);
    /*while (U1STAbits.UTXBF != 0); 
    U1TXREG = 'M';
    print(msb);
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';
    
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = 'L';
    print(lsb);*/
    lsb = lsb & mask_lsb; //  mask for lsb to cancel the bits
    
    /*while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';*/
    
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

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    int xavg = 0, yavg = 0, zavg = 0;
    int grad = 0;
    
    for (int i = 0; i < CNTSTOP; i++)
    {
        xavg += x[count];
        yavg += y[count];
        zavg += z[count];
    }
    
    xavg /= CNTSTOP;
    yavg /= CNTSTOP;
    zavg /= CNTSTOP;
    grad = atan2 (yavg, xavg);
    printImu(xavg, yavg, zavg);
    printGrad(grad);
}

void printGrad(int value)
{
    char buffer[100];

    sprintf(buffer,"$YAW,%d*", value); 
    
    for(int i =0; buffer[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buffer[i];
    }
}