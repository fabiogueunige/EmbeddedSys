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
#include "adclib.h"
#include "pwmlib.h"
#include "scheduler.h"
#include "parser.h"
#include <stdio.h>

/* ################################################################
                        Parameter Definition
    ###################################################################*/

#define MAX_TASKS 2
#define WAIT_FOR_START 0
#define EXECUTE 1

/* ################################################################
                        Interrupt functions
    ###################################################################*/
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void); // timer interrupt for debouncing
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void); 

/* ################################################################
                        functions definition
    ###################################################################*/
void execute(); // to change the enable
void wait_for_start(); // to change the enable
void taskBlinkLedA0 (void* );
void taskBlinkIndicators (void* );
void taskADCSensing(void* );

void printValue (float , float );

/* ################################################################
                        global variables
    ###################################################################*/
typedef struct {
    float battery_data; // battery value
    float infraRed_data; // Infrared value
} data;


heartbeat schedInfo[MAX_TASKS]; // task of the scheduler
int state; // state of the state machine
int checkInterrupt; // boolean to know if button presseìd

data data_values; 

int main(void) 
{
    /* ################################################################
                            Init Config
    ###################################################################*/
    // 0 = digital; 1 = analog
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
       
    /* ################################################################
                                Timer setup
    ###################################################################*/
    tmr_setup_period(TIMER5, 1); // setup the period for the while loop
    
    /* ################################################################
                            State Initialization
    ###################################################################*/
    state = WAIT_FOR_START;
    checkInterrupt = 0;
    data_values.battery_data = 0.0;
    data_values.infraRed_data = 0.0;
    
    /* ################################################################
                        pin remap and setup of the led
    ###################################################################*/
    // LED A0 setup
    TRISAbits.TRISA0 = 0; // 0 = output, 1 = input 
    LATAbits.LATA0 = 1; // value as low, to write (to read is port)
    // indicatorts setup
    TRISBbits.TRISB8 = 0; // left indicator
    LATBbits.LATB8 = 1;
    TRISFbits.TRISF1 = 0; // right indicator
    LATFbits.LATF1 = 1;
    
    /* ################################################################
                        pin remap and setup of the buttons
    ###################################################################*/
    // Mapping INT1 to RE8 pin of left button
    RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal
    
    /* ################################################################
                        pin adc remap
    ###################################################################*/
    // Mapping battery (no need)
    // enable the infra-red sensor
    TRISBbits.TRISB9 = 0; // set the enable pin as output
    LATBbits.LATB9 = 1; //set the high value for enable the infra-red sensor

    /* ################################################################
                        Defining all the tasks
    ###################################################################*/
    heartbeat schedInfo[MAX_TASKS];
    // LEs A0 task config
    schedInfo[0].n = 0;
    schedInfo[0].N = 1000;
    schedInfo[0].f = taskBlinkLedA0;
    schedInfo[0].params = NULL;
    schedInfo[0].enable = 1;
    // indicator conig
    schedInfo[1].n = -2;
    schedInfo[1].N = 1000;
    schedInfo[1].f = taskBlinkIndicators;
    schedInfo[1].params = NULL;
    schedInfo[1].enable = 1;
    
    // ADC acquisition
    schedInfo[2].n = 0;
    schedInfo[2].N = 20;
    schedInfo[2].f = taskADCSensing;
    schedInfo[2].params = (void*)(&data_values);
    schedInfo[2].enable = 1;
    
    /* ################################################################
                        Peripheral configuration
    ###################################################################*/
    
    // Guarda dentro funzioni e controlla non manchi niente
    adc_scanmode_config(); // setup for the ADC (battery and InfraRed)
    
    // Controllare che non manchi ricezione, controlla se vanno bene gli interrupt (ric on, tr off))
    uart_config(); // uart trasmission and reception configuration

    // pwmConfig(); // standard pwm configuration
    
    /* ################################################################
                    Adc mode activation
    ###################################################################*/
    // select the scan pin
    AD1CSSLbits.CSS11 = 1; // Enable AN11 for scan (battery)
    AD1CSSLbits.CSS14 = 1; // Enable AN14 for scan (infra-red)
    
    ANSELBbits.ANSB11 = 0x0001; // battery input as analog value
    ANSELBbits.ANSB14 = 0x0001; // IR input as analog value
    
    //MUST BE LAST THING TO DO:
    AD1CON1bits.ADON = 1; // turn ADC on
    
    /* ################################################################
                        Interrupt activation
    ###################################################################*/
    INTCON2bits.GIE = 1; // set global interrupt enable 

    
    IFS1bits.INT1IF = 0; // clear interrupt flag
    IEC1bits.INT1IE = 1; // enable interrupt
    IFS0bits.T1IF = 0; // reset interrupt flag timer 1
    
    U1TXREG = 'c'; 
    
    while (1)
    {
        if (checkInterrupt == 1) // to enter only when the button has been pressed
        {
            if (state == WAIT_FOR_START)
            {
                // wait_for_interrupt ();
                schedInfo[1].enable = 1;
            }
            else
            {
                // execute();
                schedInfo[1].enable = 0; 
                LATBbits.LATB8 = 0;
                LATFbits.LATF1 = 0;
            }
            checkInterrupt = 0;
        }
        
        scheduler(schedInfo, MAX_TASKS);
        U1TXREG = data_values.battery_data; 

        
        while(!tmr_wait_period(TIMER5)); // wait for the end of the Hb
    }
    
    return 0;
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    // interrupt to change the status of state machine
    IFS0bits.T1IF = 0; // reset interrupt flag
    IEC0bits.T1IE = 0;
    T1CONbits.TON = 0; // stop the timer
    
    if (state == WAIT_FOR_START)
    {
        state = EXECUTE;
    }
    else 
    {
        state = WAIT_FOR_START;
    }
    checkInterrupt = 1;
    // Reactivating the button interrupt
    IFS1bits.INT1IF = 0; //put to zero the flag of the interrupt
    IEC1bits.INT1IE = 1; // enable the interrupt
}

//interrupt associated to button 2
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void){
    // Advertise button has been pressed
    // The change of the state will be done in T1 interrupt
    //interrupt code  
    IEC1bits.INT1IE = 0; // disable the interrupt
    IFS1bits.INT1IF = 0; //put to zero the flag of the interrupt
    
    // activating the interrupt
    tmr_setup_period(TIMER1, 50); // setup the timer (consider the timer)
    IEC0bits.T1IE = 1;
}

void taskBlinkLedA0 (void* param)
{
    LATAbits.LATA0 = !LATAbits.LATA0;
}

void taskBlinkIndicators (void* param)
{
    // left indicator
    LATBbits.LATB8 = !LATBbits.LATB8;
    // right indicator
    LATFbits.LATF1 = !LATFbits.LATF1;
}

void taskADCSensing(void* param)
{
    data* cd = (data*) param;
    while(!AD1CON1bits.DONE);
    int potBitsBatt = ADC1BUF0;
    int potBitsIr = ADC1BUF1;
    cd->battery_data = battery_conversion((float) potBitsBatt); 
    cd->infraRed_data = bit2volt((float) potBitsIr);
    cd->infraRed_data = volt2cm(cd->infraRed_data);
}

/*void printValue (float val, float val2)
{
    char buff[40];

    sprintf(buff,"$SENS,%.3f,%.3f*", val, val2); 
    
    // write the values of the ADC register
    for(int i = 0; buff[i] != 0; i++)
    {
        U1TXREG = buff[i]; 
        while (U1STAbits.UTXBF == 1);
    }   
    
}*/


















void execute() // to change the enable
{
    // modify the enable
    schedInfo[1].enable = 0; // deactivating the indicators
    
}

void wait_for_start()
{ 
/**
 * All motor must be 0 (stop)
 * Led A0 blink at 1HZ (1000 reps of while loop)
 * Left and right indicators blink at 1HZ (left = RB8, right = RF1)
 * 
 * Button RE8 pressed -> go new state
 */
    schedInfo[1].enable = 1; // activating the indicators
}
