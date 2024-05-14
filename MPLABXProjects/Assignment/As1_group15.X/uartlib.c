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
}