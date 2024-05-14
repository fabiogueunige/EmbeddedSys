/*
 * File:   spilib.c
 * Author: fabio
 *
 * Created on 14 maggio 2024, 11.24
 */

#include "xc.h"
#include "spilib.h"

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

int spi_write (unsigned int addr)
{
    
    int data; // the value to return to the function
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrived
    data = SPI1BUF; // the byte sent from the slave of the previous request
        
    return data;
}
