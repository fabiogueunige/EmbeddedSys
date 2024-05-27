/*
 * File:   main.c
 * Author: fabio
 *
 * Created on May 23, 2024, 12:49 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"
#include <stdio.h>
#include "uartlib.h"
#include "pwmlib.h"

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void); // timer interrupt for debouncing
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void); 

int bttcounter = 0;

int main(void) {
    
    // set all the pin as digital, then set 1 in the code when need to use it as analog
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
        
    // Mapping INT1 to RE8 pin of left button
    RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal
    INTCON2bits.GIE = 1; // set global interrupt enable
    
    // setting the pwm
    pwmConfig(); // standard pwm configuration
    
    whstop();
    
    IFS1bits.INT1IF = 0; // clear interrupt flag
    IEC1bits.INT1IE = 1; // enable interrupt
    
    while(1)
    {
        
    }
    return 0;
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // reset interrupt flag
    IEC0bits.T1IE = 0;
    T1CONbits.TON = 0; // stop the timer
    
    // code for the interrupt
    if (PORTEbits.RE8 == 1) // to check if the contact was request
    {
        if (bttcounter == 0)
        {
            moveForward (WHMOVESTD);
            bttcounter ++;
        }
        else 
        {
            whstop();
            bttcounter = 0;
        }
    }
    // Reactivating the button interrupt
    IFS1bits.INT1IF = 0; //put to zero the flag of the interrupt
    IEC1bits.INT1IE = 1; // enable the interrupt
}

//interrupt associated to button 2
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void){
    //interrupt code  
    IEC1bits.INT1IE = 0; // disable the interrupt
    IFS1bits.INT1IF = 0; //put to zero the flag of the interrupt
    
    // activating the interrupt
    tmr_setup_period(TIMER1, 10); // setup the timer
    IEC0bits.T1IE = 1;
}


