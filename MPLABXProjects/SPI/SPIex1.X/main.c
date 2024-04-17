/*
 * File:   main.c
 * Author: fabio
 *
 * Created on April 16, 2024, 12:13 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"

int spi_write (unsigned int );

int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    // INTCON2bits.GIE = 1; // set global interrupt enable 
    // SPI configuration
    SPI1CON1bits.MSTEN = 1; // master mode 
    SPI1CON1bits.MODE16 = 0; // 8 bit mode
    SPI1CON1bits.PPRE =  6;// setting the primary prescaler
    SPI1CON1bits.SPRE = 0; // setting the secodary prescaler
    SPI1STATbits.SPIEN = 1; // enable the SPI
    
    // Remapping of the magnetometer
    // Setting (will check why like that)
    TRISAbits.TRISA1 = 1; // input
    TRISFbits.TRISF12 = 0; // output
    TRISFbits.TRISF13 = 0; // output
    
    // Reception Rx
    // before i set the functionallity and i assign the pin
    RPINR20bits.SDI1R = 0b0010001; // MISO associated to remappable input RPI17 
    RPOR12bits.RP109R = 0b000101; // look in (pg 215 where there are all the functionalities) looking for SDO1 
    RPOR11bits.RP108R = 0b000110; // clock SKC because synchronouse
    
    // remap UART1 pins
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) (remapUARTpin = funcionality)
    RPINR18bits.U1RXR = 0b1001011; // virtual remap for input, assign directly 
    // function to pinrx of UART (functionality = remappable input pin)
    
    // set the baud rate and UART
    // U1BRG = round((FCY /(16LL * BAUD_RATE))-1); // (better directly on integer)
    U1BRG = 468; // setting the boud rate register
    U1MODEbits.UARTEN = 1; // enable uart
    
    U1STAbits.UTXEN = 1; // UART trasmission enable
    
    int write_return = 0;
    
    PORTD.bits.RD6 = 0; // chip select, select the chip connected to the magnotometer
    // LATDbits.LATD6 = 0;
    write_return = spi_write(0x4B);
    while (U1STAbits.UTXBF != 0);
    U1TXREG = write_return;
    write_return = spi_write(0x01);
    // LATDbits.LATD6 = 1;
    PORTD.bits.RD6 = 1;

    while (U1STAbits.UTXBF != 0);
    U1TXREG = write_return;
    
    while(1)
    {
        
    }
    return 0;
}

int spi_write (unsigned int addr)
{
    int data; 
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd
    data = SPI1BUF; // the byte sent from the slave
    // while (SPI1STATbits.SPITBF == 1); // wait until tx buffer is not full
    return data;
}




