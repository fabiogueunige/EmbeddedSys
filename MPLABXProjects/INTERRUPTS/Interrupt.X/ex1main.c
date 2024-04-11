/*
 * File:   ex1main.c
 * Author: fabio
 *
 * Created on 11 aprile 2024, 10.11
 */


#include "xc.h"
#include "timer.h"

/*
 void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void); // timer interrupt
 */

/* main exercise 3.1
 * // tmr_setup_period (TIMER2, 20); // init of the timer
    // TRISAbits.TRISA0 = 1; // set the pin as input
    TRISGbits.TRISG9 = 0; // write

    while(1)
    {   
        tmr_wait_ms(TIMER1,200); // led 2
        // blink led 1
        TRISAbits.TRISA0 = 1; // set pin  to read 
        if (PORTAbits.RA0 == 0) // reading the pin
        {
            TRISAbits.TRISA0 = 0; // set pin to output
            LATAbits.LATA0 = 1; // write on the pin to turn on the led
            // tmr_wait_period(TIMER2);
        }
        else 
        {
            TRISAbits.TRISA0 = 0; // set the pin as output
            LATAbits.LATA0 = 0; // write on thhe pin to off the led   
        }
        tmr_wait_ms(TIMER2, 50);
    } 
 * 
 * 
 * 
 * interrupt exercise 3.1
 * void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // reset interrupt flag
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
        LATGbits.LATG9 = 0; // write on thhe pin to off the led   
    }
}
 */
