/*
 * File:   main.c
 * Author: fabio
 *
 * Created on March 7, 2024, 12:44 PM
 */


#include "xc.h"
#include "timer.h"


int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    int ret;
    tmr_setup_period (TIMER1, 200); // init of the timer
    // tmr_setup_period(TIMER2, 20);
    // tmr_setup_period (TIMER2, 20); // init of the timer
    // TRISAbits.TRISA0 = 1; // set the pin as input
    TRISGbits.TRISG9 = 0; // write

    while(1)
    {   
        // turn on the ledfor 20 ms 
        tmr_wait_ms(TIMER2, 50);
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
        
        ret = tmr_wait_period(TIMER1);
        if (ret)
        {
            // TRISAbits.TRISA0 = 0; // set pin to output
            LATGbits.LATG9 = 1;
        }
        else {
            LATGbits.LATG9 = 0;
        }
    }    
    return 0;
}
/*
 * Exe 2.1
 // tmr_setup_period (TIMER2, 20); // init of the timer
    TRISAbits.TRISA0 = 1; // set the pin as input

    while(1)
    {   
        // turn on the ledfor 20ms 
        TRISAbits.TRISA0 = 1; // set pin  to read 
        if (PORTAbits.RA0 == 0) // reading the pin
        {
            TRISAbits.TRISA0 = 0; // set pin to output
            LATAbits.LATA0 = 1; // write on thhe pin to turn on the led
            // tmr_wait_period(TIMER2);
        }
        else 
        {
            TRISAbits.TRISA0 = 0; // set the pin as output
            LATAbits.LATA0 = 0; // write on thhe pin to off the led
            // 
            
        }   
        tmr_wait_period(TIMER1);
    }    
    return 0;
}*/