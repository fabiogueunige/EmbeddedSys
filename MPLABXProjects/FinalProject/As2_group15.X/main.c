/*
 * File:   main.c
 * Author: 
 * * Chiappe Andrea
 * * Di Donna Alberto
 * * Guelfi Fabio s5004782
 * * Utegaliyeva ...
 *
 * Created on June 13, 2024, 12:36 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"
#include "uartlib.h"
// #include "spilib.h"
#include "adclib.h"
#include "pwmlib.h"
#include <stdio.h>

int main(void) {
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // Guarda dentro funzioni e controlla non manchi niente
    adc_scanmode_config(); // setup for the ADC (battery and InfraRed)
    
    // Controll che non manchi ricezione, controlla se vanno bene gli interrupt (ric on, tr off))
    uart_config(); // uart trasmission and reception configuration

    pwmConfig(); // standard pwm configuration
    
    tmr_setup_period(TIMER5, 1); // setup the period for the while loop
    
    while (1)
    {
        
        
        
        while(!tmr_wait_period(TIMER5)); // wait for the end of the Hb
    }
    
    return 0;
}
