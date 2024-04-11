/*
 * File:   main.c
 * Author: andre
 *
 * Created on March 28, 2024, 6:15 PM
 */


#include "xc.h"
#include "timer.h"
#include <math.h>

#define BAUD_RATE 9600

int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    // remap the pin for the UART communication
    // in tx associo il pin ad una specifica funzionalità, in questo caso U1TX
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX)
    // in ricezione leggo il valore che arriva sul pin e lo associo (ad un registro)
    RPINR18bits.U1RXR = 0b1001011; // virtual remap for input, assign directly function to pin rx of UART
    
    //U1MODE = 0x8000;
    // set the baud rate
    U1BRG = round((FCY /(16LL * BAUD_RATE))-1);
    U1BRG = 468;
    U1MODEbits.UARTEN = 1; // enable uart
    U1STAbits.UTXEN = 1;
       
    U1TXREG = 'c';
    //U1TXREG = U1RXREG;

    
    
    while(1)
    {
        tmr_wait_ms(TIMER1, 200);
        U1TXREG = U1RXREG;
    }
    
    
             
    
    
    
    
    
    return 0;
}
