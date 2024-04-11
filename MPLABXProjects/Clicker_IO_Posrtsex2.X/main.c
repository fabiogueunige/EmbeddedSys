/*
 * File:   main.c
 * Author: andre
 *
 * Created on March 28, 2024, 3:58 PM
 */


#include "xc.h"
#include "timer.h"

//interrupt associated to button 2
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void){
    
    //interrupt code  
    IFS1bits.INT1IF = 0; //put to zero the flag
            
    if(IEC0bits.T1IE == 1){ //if the interrupt associated to the timer is enabled
        IEC0bits.T1IE = 0; //disable the interrupt associated to the timer
        //TRISGbits.TRISG9 = 0; // set pin to output
        //LATGbits.LATG9 = 1; // write on the pin to turn on the led
    }
    else{
        //TRISGbits.TRISG9 = 0; // set pin to output
        //LATGbits.LATG9 = 0; // write on the pin to turn on the led
        IEC0bits.T1IE = 1; //enable interrupt of the timer    
       //  IFS0bits.T1IF = 1; // put 1 flag interrupt flag
        
    }
   
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // reset interrupt flag
    // Codice da eseguire quando si verifica l'interrupt
    TRISGbits.TRISG9 = 1; // set pin  to read 
    if (PORTGbits.RG9 == 0) // reading the pin
    {
        TRISGbits.TRISG9 = 0; // set pin to output
        LATGbits.LATG9 = 1; // write on the pin to turn on the led
    }
    else 
    {
        TRISGbits.TRISG9 = 0; // set the pin as output
        LATGbits.LATG9 = 0; // write on the pin to off the led   
    }
}


int main(void) {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    tmr_setup_period(TIMER1, 50); // setup the timer T1, expires every 200 ms

    // Mapping INT1 to RE8 pin of left button
    RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal connecting to remappable pin
    INTCON2bits.GIE = 1; // set global interrupt enable
    // managing the button
    IEC1bits.INT1IE = 1; // enable interrupt
    IFS1bits.INT1IF = 0; // clear interrupt flag
    // menaging the timer
    IEC0bits.T1IE = 1; // enable interrupt flag for timer t1
    IFS0bits.T1IF = 0; // reset interrupt flag
    
   
    while(1){
       
    }

    return 0;
}
