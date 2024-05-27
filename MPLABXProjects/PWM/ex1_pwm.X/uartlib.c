/*
 * File:   uartlib.c
 * Author: fabio
 *
 * Created on 14 maggio 2024, 11.23
 */


#include "xc.h"
#include "uartlib.h"

void tr_uart_config()
{
    // UART Setting configuration
    // U1BRG = round((FCY /(16LL * BAUD_RATE))-1); 
    U1BRG = URTBR; // setting the baud rate register directly in integer to avoid floating value
    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 0;
    
    // Pin remap
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) 
    RPINR18bits.U1RXR = 0b1001011; // Receiver functionality to the register
    
    // Enabling the transmission and uart
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // UART transmission enable
}