/*
 * File:   timer.c
 * Author: fabio
 *
 * Created on March 14, 2024, 12:50 PM
 */


#include "xc.h"
#include "timer.h"


void tmr_setup_period(int timer, int ms)
{
    float prescaler = 256.0;
    float toSec = 1000.0;
    
    if(timer == TIMER1){
     // set the prescaler 3:256
        T1CONbits.TCKPS = 3;
        TMR1 = 0; // reset timer count
        IFS0bits.T1IF = 0; // set the flag to zero  
        PR1 = ((((float) FCY) / prescaler) * (((float) ms) / toSec)); //((144 M/2)/256) x0.2
        T1CONbits.TON = 1; // start timer
       
    }else if(timer == TIMER2){
        T2CONbits.TCKPS = 3; // set the prescaler 0:0, 1:8, 2:64, 3:256        T1CONbits.TCKPS = 1;
        TMR2 = 0; // reset timer count
        IFS0bits.T2IF = 0; // set the flag to zero  
        PR2 = ((((float) FCY) / prescaler) * (((float) ms) / toSec));
        T2CONbits.TON = 1; // start timer
        //}    
    }   
}

int tmr_wait_period(int timer)
{
    // return 1 if timer has expired, otherwise returns 0
    if(timer == TIMER1){
        if (IFS0bits.T1IF == 1)
        {
            IFS0bits.T1IF = 0;
            return 1;
        } 
    }else if(timer == TIMER2){
        if (IFS0bits.T2IF == 1)
        {
            IFS0bits.T2IF = 0;
            return 1;
        }
    }    
    return 0;
}

void tmr_wait_ms(int timer, int ms)
{
    tmr_setup_period(timer, ms);
    while(tmr_wait_period(timer) != 1);  
}