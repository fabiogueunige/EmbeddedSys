/*
 * File:   main.c
 * Author: andre
 *
 * Created on May 16, 2024, 3:20 PM
 */


#include "xc.h"
#include <math.h>
#include <stdio.h>
#include "uartlib.h"
#include "timer.h"

float bit2volt (float );
float volt2cm(float );
void printValue (float );




int main(void) {
    // set all the pin as digital, then set 1 in the code when need to use it as analog
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // enable the infra-red sensor
    TRISBbits.TRISB4 = 0; // set the enable pin as output
    LATBbits.LATB4 = 1; //set the high value for enable the infra-red sensor
    
    tr_uart_config();
    
    // ADC Configuration (manual sampling and automatic conversion)
    // select the length of TAD
    AD1CON3bits.ADCS = 8; // suggested value is 8 for our board
    // select how the sampling should start
    AD1CON1bits.ASAM = 0; // sampling begins when the SAMP bit is set (manual sampling)
    // select how many tad the sample 
    AD1CON3bits.SAMC = 16; // sample time 16 tad
    // select how sampling end and conversion begin( remember to set SAMP)
    AD1CON1bits.SSRC = 7; // chose automatic conversion (for manual set = 0)
    // select how many channel use
    AD1CON2bits.CHPS = 0; // use only channel 0 
    // select the positive input for to the channel 
    AD1CHS0bits.CH0SA = 0b00101; // Channel 0 positive input is AN5
    //set the input pin as analog (value 1)
    ANSELBbits.ANSB5 = 0x0001;
    
    //MUST BE LAST THING TO DO:
    AD1CON1bits.ADON = 1; // turn ADC on
    
    float infraRed_value;
    float infraRed_volt;
    float infraRed_cm;
    
    tmr_setup_period(TIMER1, 200);
    while(1){
        
        
        if (AD1CON1bits.DONE == 1)
        {
            infraRed_value = ADC1BUF0; // read the value on the buffer
            // convert the value in volt
            infraRed_volt = bit2volt(infraRed_value);
            infraRed_cm = volt2cm(infraRed_volt);  
            printValue(infraRed_cm);
            AD1CON1bits.DONE = 0;
        }
        
        if(AD1CON1bits.SAMP == 0)
        {
            AD1CON1bits.SAMP = 1; 
        }
        // wait 200 ms
        while(!tmr_wait_period(TIMER1));        
        
    }
    return 0;
}


float bit2volt (float value){
    float value_volt;

    value_volt = (value / 1024) * 3.3;    
    return value_volt;
}

float volt2cm(float value){
    float cm;
    
    cm = 2.34 - (4.74 * value) + (4.06 * pow(value,2)) - (1.60 * pow(value,3)) + (0.24 * pow(value,4));
    cm *= 100.0;
    return cm; 
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