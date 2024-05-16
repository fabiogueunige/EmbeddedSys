/*
 * File:   main.c
 * Author: andre
 *
 * Created on May 16, 2024, 9:47 AM
 */


#include "xc.h"
#include "uartlib.h"
#include "timer.h"
#include <stdio.h>

void printValue (float );
float battery_status (float );


int main(void) {
    
    // set all the pin as digital, then set 1 in the code when need to use it as analog
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    tr_uart_config();
    
    // ADC Configuration (manual sampling and manual conversion)
    // select the length of TAD
    AD1CON3bits.ADCS = 8; // suggested value is 8 for our board
    // select how the sampling should start
    AD1CON1bits.ASAM = 0; // sampling begins when the SAMP bit is set (manual)
    // select how sampling end and conversion begin( remember to set SAMP)
    AD1CON1bits.SSRC = 0; // chose manual (for automatic set = 7)
    // select how many channel use
    AD1CON2bits.CHPS = 0; // use only channel 0 
    // select the positive input for to the channel 
    AD1CHS0bits.CH0SA = 0b01011; // Channel 0 positive input is AN11
    //set the input pins as analog (value 1): ANSELA???
    ANSELBbits.ANSB11 = 0x0001;
    
    //MUST BE LAST THING TO DO:
    AD1CON1bits.ADON = 1; // turn ADC on
    
    // set the timer 1 to restart the sampling
    tmr_setup_period (TIMER1, 200);
    float buffer_value;
    float volt;
    

    
    while(1)
    {
        if(AD1CON1bits.SAMP == 1){
            AD1CON1bits.SAMP = 0; 
        }
        else
        {            
            AD1CON1bits.SAMP = 1; 
        }
        
        if (AD1CON1bits.DONE == 1)
        {
            buffer_value = ADC1BUF0;
            volt = battery_status (buffer_value);
            printValue (volt);           
        }
        
        
        while(!tmr_wait_period(TIMER1));
    }
    
    
    
    //TODO: UART
    
    
    
    return 0;
}


float battery_status (float value){
    float value_volt;

    value_volt = (value / 1024) * 24;
    value_volt *= 3;
    
    return value_volt;
}

void printValue (float val)
{
    char buff[30];

    sprintf(buff,"$ADC,%.3f*", val); 
    
    // write the values of the ADC register
    for(int i = 0; buff[i] != 0; i++)
    {
        U1TXREG = buff[i]; 
        while (U1STAbits.UTXBF == 1);
    }   
    
}
