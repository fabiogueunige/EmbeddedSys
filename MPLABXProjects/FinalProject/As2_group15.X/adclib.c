/*
 * File:   adclib.c
 * Author: fabio
 *
 * Created on June 13, 2024, 3:15 PM
 */


#include "xc.h"
#include <math.h>
#include "adclib.h"

/* !! ADC_scan_mode_config
* Only scan mode configuration:
 * remember to enable the pin as output (TRIS and LAT)
 * select the scan pin (e.g AD1CSSLbits.CSS5 = 1;)
 * set the input pin as analog (value 1) (e.g ANSELBbits.ANSB5 = 0x0001;)
 * As last thing turn on ADC (= AD1CON1bits.ADON = 1; // turn ADC on)
*/
void adc_scanmode_config()
{
    // ADC Configuration (scan mode)0
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
}

float battery_conversion (float value){
    float value_volt;

    value_volt = bit2volt(value); // controlla questa riga di codice
    value_volt *= 3;
    
    return value_volt;
}    


float bit2volt (float value){
    float value_volt;

    value_volt = (value / 1024) * 3.3;   
    return value_volt;
}

float volt2cm(float val){
    float cm, value;

    value = bit2volt(val);
    
    cm = 2.34 - (4.74 * value) + (4.06 * pow(value,2)) - (1.60 * pow(value,3)) + (0.24 * pow(value,4));
    cm *= 100.0;
    return cm; 
}

// ATTENZIONE!!!!
/*
 L'idea migliore usando ADC � di salvare i valori dei pin dell'adc non appena 
 * il "done" diventa 1 all'interno di un buffer per evitare che vengano sovrascritti da 
 * nuovi valori e perdere i precedenti
 */

