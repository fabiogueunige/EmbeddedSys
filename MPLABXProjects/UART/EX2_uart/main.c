/*
 * File:   main.c
 * Author: fabio
 *
 * Created on April 9, 2024, 4:26 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"

void myfunction(int ,int ); // function that use 7 ms to be completed
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void); // interrupt tp blink the led
void __attribute__((__interrupt__, __auto_psv__)) _T3Interrupt(void); // interrupt to syncronhize the myfunction

int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    // setup timer 2
    // tmr_setup_period(TIMER2, )
    // setup timer 3
    tmr_setup_period(TIMER3, 10); // timer to syncronyze the myfunction
    IEC0bits.T3IE = 1; // enable interrupt timer t3
    IFS0bits.T3IF = 0; // reset interrupt flag
    
    /*// remap the pin for the UART communication
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
    */
    
    while(1){
        if (IEC0bits.T3IE == 0)
        {
            IEC0bits.T3IE = 1;
            myfunction(TIMER1, 7);
        }
    }
    
    return 0;
}


void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0; // reset interrupt flag
    // Codice da eseguire quando si verifica l'interrupt
    TRISGbits.TRISG9 = 1; // set pin  to read 
    if (PORTGbits.RG9 == 0) // reading the pin
    {
        TRISGbits.TRISG9 = 0; // set pin to output
        LATGbits.LATG9 = 1; // write on the pin to turn on the led
        // tmr_wait_period(TIMER2);
    }
    else 
    {
        TRISGbits.TRISG9 = 0; // set the pin as output
        LATGbits.LATG9 = 0; // write on the pin to off the led   
    }
}

void __attribute__((interrupt, auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0; // reset interrupt flag
    IEC0bits.T3IE = 0;
    
    
    
}

void myfunction(int tmr, int tiempo){
    tmr_wait_ms(tmr, tiempo);
}