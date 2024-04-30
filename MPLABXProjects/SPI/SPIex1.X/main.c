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
#include <stdio.h>

int spW (unsigned int addr);
void print(int);
void printImu(int);
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
    // SPI1CON1bits.CKP = 1; // Specify the idle value of the clock
    
    // Remapping of the magnetometer
    TRISAbits.TRISA1 = 1; // input
    TRISFbits.TRISF12 = 0; // output
    TRISFbits.TRISF13 = 0; // output
    
    // Reception Rx
    // before i set the functionallity and i assign the pin
    RPINR20bits.SDI1R = 0b0010001; // (pp 99 and 213) MISO associated to remappable input RPI17 
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
    int x_lsb = 0; // less significant bit
    int x_msb = 0; // most significant
    
    TRISBbits.TRISB3 = 0; 

    TRISBbits.TRISB4 = 0;

    TRISDbits.TRISD6 = 0; // set as output for the magnometer

    LATBbits.LATB3 = 1; 

    LATBbits.LATB4 = 1; 

    LATDbits.LATD6 = 1;
    //PORTDbits.RD6 = 0; ERROR is to read
    
    // going on sleep mode
    LATDbits.LATD6 = 0; // chip select, select the chip connected to the magnotometer
    chip_val = spi_write(0x4B, 0x01, bau);
    LATDbits.LATD6 = 1;
    /*
Quando si comunica tramite SPI, è comune utilizzare un pin CS per selezionare il dispositivo con cui si desidera comunicare. Il pin CS deve essere portato a basso prima di inviare dati al dispositivo e portato ad alto dopo la trasmissione per indicare al dispositivo che la comunicazione è terminata.

Quindi, LATDbits.LATD6 = 0; imposta il pin CS a basso prima di iniziare a scrivere dati al dispositivo SPI, mentre LATDbits.LATD6 = 1; lo riporta ad alto dopo che la comunicazione è terminata. Questo assicura che il dispositivo SPI sia correttamente selezionato durante la trasmissione dei dati e che sia deselezionato quando la trasmissione è completata.

Senza queste istruzioni, il dispositivo SPI potrebbe non ricevere correttamente i dati o potrebbe essere selezionato per un periodo più lungo del necessario, il che potrebbe portare a comportamenti imprevisti nella comunicazione.
     */
    
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
    {
        LATDbits.LATD6 = 0;
        // x_lsb = spi_write(0x42 | 0x80, 0x00, &x_msb);
        spW(0x42 | 0x80);
        x_lsb = spW(0x47 | 0x80);
        x_msb = spW(0x00);
        LATDbits.LATD6 = 1;
        // print(x_lsb);
        print(x_msb);
        x_lsb = x_lsb & 0xF8; //  
        x_msb = (x_msb << 8); // msb shifted of 8 of left
        // print(x_msb);
        x_msb = x_msb | x_lsb; // the union of lsb and msb shifted
        x_msb = x_msb / 8; // divide the value by 8
        //printImu (x_msb);
        while(!tmr_wait_period (TIMER2));
    }
    return 0;
}

int spW (unsigned int addr)
{
    int data;
    while (SPI1STATbits.SPITBF == 1); // wait until tx buffer is not full
    SPI1BUF = addr;
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrievd
    data = SPI1BUF;
    // U1TXREG = data;
    
    return data;
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
    int size;
    char buff[20];
    size = sprintf(buff,"%d", stamp);
    while (U1STAbits.UTXBF != 0); // ask if we can use this register
    U1TXREG = size;
    for (int i = 0; buff[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buff [i];
    }
}

void printImu(int stamp)
{
    char buff[20];
    char str[] = "$MAGX=";
    sprintf(buff,"%d", stamp); 
    
    for(int i =0; str[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = str[i];

    }
    for (int i = 0; buff[i] != 0; i++)
    {
        while (U1STAbits.UTXBF != 0); // ask if we can use this register
        U1TXREG = buff[i];
    }
    while (U1STAbits.UTXBF != 0); 
    U1TXREG = '*';
}



