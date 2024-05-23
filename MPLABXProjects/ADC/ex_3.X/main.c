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
void printValue (float , float );
float battery_conversion (float );

int main(void) {
    // set all the pin as digital, then set 1 in the code when need to use it as analog
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // enable the infra-red sensor
    TRISBbits.TRISB4 = 0; // set the enable pin as output
    LATBbits.LATB4 = 1; //set the high value for enable the infra-red sensor
    
    tr_uart_config();
    
    // ADC Configuration (scan mode)
    // select the length of TAD
    AD1CON3bits.ADCS = 64; // set to maximum
    // select how the sampling should start
    AD1CON1bits.ASAM = 1; // automatic sampling start
    // select how many tad is long the sample 
    AD1CON3bits.SAMC = 31; // set to maximum sample time
    // select how sampling end and conversion begin( remember to set SAMP)
    AD1CON1bits.SSRC = 7; // chose automatic conversion (for manual set = 0)
    // select how many channel use
    
    AD1CON2bits.SMPI = 0b001; // set the SMPI (sample per interrupt) to take both
    // pin datas
    
    //AD1CON2bits.CHPS = 0; // use only channel 0 
    AD1CON2bits.CSCNA = 1; // activate scan mode
   
    // select the scan pin
    AD1CSSLbits.CSS5 = 1; // Enable AN5 for scan (infra-red)
    AD1CSSLbits.CSS11 = 1; // Enable AN11 for scan (battery)

    
    //set the input pin as analog (value 1)
    ANSELBbits.ANSB5 = 0x0001;
    ANSELBbits.ANSB11 = 0x0001;

    
    //MUST BE LAST THING TO DO:
    AD1CON1bits.ADON = 1; // turn ADC on
    
    tmr_setup_period(TIMER1, 1);
    int counter;
    float battery_data;
    float infraRed_data;
    
    while(1){
        
        infraRed_data = ADC1BUF0; // read data from infrared
        battery_data = ADC1BUF1; // read data from battery
        
        if(counter == 100){
            counter = 0;
            battery_data = battery_conversion(battery_data);
            infraRed_data = bit2volt (infraRed_data);
            infraRed_data = volt2cm(infraRed_data);
            printValue(infraRed_data, battery_data);
        }
        counter ++;
        
        
        while(!tmr_wait_period(TIMER1));        
        
    }
    return 0;
}
float battery_conversion (float value){
    float value_volt;

    value_volt = (value / 1024);
    value_volt *= 3;
    
    return value_volt;
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

void printValue (float val, float val2)
{
    char buff[40];

    sprintf(buff,"$SENS,%.3f,%.3f*", val, val2); 
    
    // write the values of the ADC register
    for(int i = 0; buff[i] != 0; i++)
    {
        U1TXREG = buff[i]; 
        while (U1STAbits.UTXBF == 1);
    }   
    
}