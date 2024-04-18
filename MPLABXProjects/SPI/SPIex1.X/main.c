/*
 * File:   main.c
 * Author: fabio
 *
 * Created on April 16, 2024, 12:13 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"
#include <string.h>

void print(int);
int spi_write (unsigned int, int , int*);


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
    U1BRG = 468; // setting the boud rate register
    U1MODEbits.UARTEN = 1; // enable uart
    
       
    U1STAbits.UTXEN = 1; // UART trasmission enable
    
    int chip_val = 0;
    int *bau;
    int x_lsb = 0;
    int x_msb = 0;
    
    TRISDbits.TRISD6 = 0; // set the pin as output
    //PORTDbits.RD6 = 0; ERROR is to read
    
    // going on sleep mode
    LATDbits.LATD6 = 0; // chip select, select the chip connected to the magnotometer
    chip_val = spi_write(0x4B, 0x01, bau);
    LATDbits.LATD6 = 1;
    
    tmr_wait_ms (TIMER1, 2);
    // going on active mode
    LATDbits.LATD6 = 0;
    chip_val = spi_write(0x4C , 0x00, bau);
    LATDbits.LATD6 = 1;
        
    // stamp the address of the chip selector
    tmr_wait_ms (TIMER1, 2);
    LATDbits.LATD6 = 0;
    // unsigned int a = 0x40 & 0x7F
    chip_val = spi_write(0x40 | 0x80, 0x00, bau);    
    print(chip_val);
    LATDbits.LATD6 = 1;
    
    // now we read the x-axis of the magnetometer and sending it at 10Hz
    tmr_setup_period(TIMER2, 100);
    
    
    while(1)
    {/*
        x_lsb = spi_write(0x42, 0x00, x_msb);
        // while (U1STAbits.UTXBF != 0); // ask if we can use this register
        /*U1TXREG = 'I';
        print(x_lsb);
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = 'S';// 
        // print(x_msb);
        x_lsb = x_lsb & 0xF8;
        // x_msb << 8; // msb shifted of 8 of left
        x_msb = (x_msb << 8) | x_lsb; // the union of lsb and msb shifted
        x_msb = x_msb / 8;
        // while (U1STAbits.UTXBF != 0); // ask if we can use this register
        // U1TXREG = 'F';
        print (x_msb);
        tmr_wait_period (TIMER2);
        */
    }
    return 0;
}

int spi_write (unsigned int addr, int value, int *trash_next)
{
    int data; // the value to return to the function
    while (SPI1STATbits.SPITBF == 1); // wait until the tx buffer is not full
    SPI1BUF = addr; // send the address of the device
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd (becouse space fo only a char)
    *trash_next = SPI1BUF; // the byte sent from the slave
    while (SPI1STATbits.SPITBF == 1); // wait until tx buffer is not full
    SPI1BUF = value;
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd
    data = SPI1BUF;
    // U1TXREG = data;
    
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



