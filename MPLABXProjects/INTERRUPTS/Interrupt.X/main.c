/*
 * File:   main.c
 * Author: fabio
 *
 * Created on March 14, 2024, 5:41 PM
 */


#include "xc.h"
#include "timer.h"

// proptotipe of the interrupt functions
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void); // timer interrupt
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void); // mappable (butto in our case) interrupt

int main(void) {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    tmr_setup_period(TIMER1, 200);
    
    // Mapping INT1 to RE8 pin of left button
    RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal
    INTCON2bits.GIE = 1; // set global interrupt enable
    IFS1bits.INT1IF = 0; // clear interrupt flag
    IEC1bits.INT1IE = 1; // enable interrupt
    
    
    
    while (1)
    {
        /////////////////////////////////////////////////////////////
        TRISGbits.TRISG9 = 0; // set pin to output
        LATGbits.LATG9 = 1; // write on the pin to turn on the led
        /////////////////////////////////////////////////////////////////
    }     
    return 0;
}

//interupt associated to timer
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // reset interrupt flag
    // Code when the interrupt is executing
    TRISGbits.TRISG9 = 1; // set pin  to read 
    if (PORTGbits.RG9 == 0) // reading the pin
    {
        TRISGbits.TRISG9 = 0; // set pin to output
        LATGbits.LATG9 = 1; // write on the pin to turn on the led
    }
    else 
    {
        TRISGbits.TRISG9 = 0; // set the pin as output
        LATGbits.LATG9 = 0; // write on thhe pin to off the led   
    }
}

//interrupt associated to button 2
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void){
    //interrupt code  
    IFS1bits.INT1IF = 0; //put to zero the flag
            
    if(IEC0bits.T1IE = 1){ //if the interrupt associated to the timer is enabled
        IEC0bits.T1IE = 0; //disable the interrupt associated to the timer
    }
    else{
        IEC0bits.T1IE = 1; //enable interrupt of the timer
    }   
}
