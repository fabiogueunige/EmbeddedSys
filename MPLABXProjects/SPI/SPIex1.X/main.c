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
    
    while(1)
    {
        
    }
    return 0;
}

int spi_write (unsigned int addr)
{
    SPI1CONbits.MSTEN = 1; // master mode 
    SPI1CONbits.MODE16 = 0; // 8 bit mode
    SPI1CONbits.PPRE =  6;// setting the primary prescaler
    SPI1CONbits.SPRE = 0; // setting the secodary prescaler
    SPI1STATbits.SPIEN = 1; // enable the SPI
    
    
}
