/*
 * File:   timer.c
 * Author: fabio
 *
 * Created on March 7, 2024, 12:50 PM
 */


#include "xc.h"
#include "timer.h"


void tmr_setup_period(int timer, int ms)
{
    
    if(timer == TIMER1){
        T1CONbits.TCKPS = 3; // set the prescaler 0:0, 1:8, 2:64, 3:256
        TMR1 = 0; // reset timer count
        IFS0bits.T1IF = 0; // set the flag to zero  
        PR1 = 56250;
         // float cast_ms = (ms / 1000);
        // PR1 = (FCY / 256)* cast_ms; //((144 M/2)/256) x0.2
        T1CONbits.TON = 1; // start timer
    }else if(timer == TIMER2){
        T2CONbits.TCKPS = 3; // set the prescaler 0:0, 1:8, 2:64, 3:256
        TMR2 = 0; // reset timer count
        IFS0bits.T2IF = 0; // set the flag to zero  
        PR2 = 5625;
        T2CONbits.TON = 1; // start timer
    }      
}
void tmr_wait_period(int timer)
{
    while (1)
    {
        if(timer == TIMER1){
            if (IFS0bits.T1IF == 1)
            {
                IFS0bits.T1IF = 0;
                break;
            } 
        }else if(timer == TIMER2){
            if (IFS0bits.T2IF == 1)
            {
                IFS0bits.T2IF = 0;
                break;
            }
        }
        
    }
}

