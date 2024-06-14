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
#include "circularbufferlib.h"
#include <stdio.h>

/* ################################################################
                        Parameter Definition
    ###################################################################*/

#define MAX_TASKS 5
#define WAIT_FOR_START 0
#define EXECUTE 1

/* ################################################################
                        Interrupt functions
    ###################################################################*/
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void); // timer interrupt for debouncing
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void);  // button interrupt
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void); // uart trasmission interrupt

/* ################################################################
                        functions definition
    ###################################################################*/
void taskBlinkLedA0 (void* ); // task for the led A0
void taskBlinkIndicators (void* ); // task for the indicators
void taskADCSensing(void* ); // task for the ADC sensing
void taskPrintBattery(void* ); // task for the battery print on circular buffer
void taskPrintInfrared (void* ); // task for the infrared print on circular buffer


/* ################################################################
                        global variables
    ###################################################################*/
typedef struct {
    float battery_data; // battery value
    float infraRed_data; // Infrared value
    fifo fifo_write; // circular buffer for trasmission
} data;


heartbeat schedInfo[MAX_TASKS]; // task of the scheduler
int state; // state of the state machine
int checkInterrupt; // boolean to know if button pressed

data data_values; // data structure for the values of the sensors

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
    state = WAIT_FOR_START; // initial state
    checkInterrupt = 0; // initial value for the interrupt
    data_values.battery_data = 0.0; // initial value for the battery
    data_values.infraRed_data = 0.0; // initial value for the infrared
    fifo_init(data_values.fifo_write); // initialization of the circular buffer
    
    /* ################################################################
                        Defining all the tasks
    ###################################################################*/
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
    schedInfo[2].n = -5;
    schedInfo[2].N = 1;
    schedInfo[2].f = taskADCSensing;
    schedInfo[2].params = (void*)(&data_values);
    schedInfo[2].enable = 1;
    
    // Print Battery
    schedInfo[3].n = -60;
    schedInfo[3].N = 1000;
    schedInfo[3].f = taskPrintBattery;
    schedInfo[3].params = (void*)(&data_values);
    schedInfo[3].enable = 1;

    // Print Infrared
    schedInfo[4].n = -10;
    schedInfo[4].N = 100;
    schedInfo[4].f = taskPrintInfrared;
    schedInfo[4].params = (void*)(&data_values);
    schedInfo[4].enable = 1;

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
    TRISAbits.TRISA3 = 0; // set the enable pin as output
    LATAbits.LATA3 = 1; //set the high value for enable the infra-red sensor
    
    /* ################################################################
                        Peripheral configuration
    ###################################################################*/
    
    adc_scanmode_config(); // setup for the ADC (battery and InfraRed)
    
    // Controllare che non manchi ricezione, controlla se vanno bene gli interrupt (ric on, tr off))
    uart_config(); // uart trasmission and reception configuration

    pwmConfig(); // standard pwm configuration
    
    /* ################################################################
                    Adc mode activation
    ###################################################################*/
    // select the scan pin
    AD1CSSLbits.CSS11 = 1; // Enable AN11 for scan (battery)
    AD1CSSLbits.CSS15 = 1; // Enable AN14 for scan (infra-red)
    
    ANSELBbits.ANSB11 = 0x0001; // battery input as analog value
    ANSELBbits.ANSB15 = 0x0001; // IR input as analog value (controlla rapporto 15 e pin)
    
    //MUST BE LAST THING TO DO:
    AD1CON1bits.ADON = 1; // turn ADC on
    
    /* ################################################################
                        Interrupt activation
    ###################################################################*/
    INTCON2bits.GIE = 1; // set global interrupt enable 

    
    IFS1bits.INT1IF = 0; // clear interrupt flag
    IEC1bits.INT1IE = 1; // enable interrupt
    IFS0bits.T1IF = 0; // reset interrupt flag timer 1

    IEC0bits.U1TXIE = 1; // enable U1TX interrupt 
    
    
    while (1)
    {
        if (checkInterrupt == 1) // to enter only when the button has been pressed
        {
            if (state == WAIT_FOR_START)
            {
                schedInfo[1].enable = 1; // enable the indicators
                whstop(); // stop the wheels
            }
            else
            {
                schedInfo[1].enable = 0; // disable the indicators
                LATBbits.LATB8 = 0;
                LATFbits.LATF1 = 0;
                moveForward(WHMOVESTD); // move the wheels forward
            }
            checkInterrupt = 0;
        }
        
        scheduler(schedInfo, MAX_TASKS);
         

        
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

// uart1 interrupt function 
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void)
{   
    //set the flag to zero
    IFS0bits.U1TXIF = 0;
    
    while((U1STAbits.UTXBF != 1) && (data_values.fifo_write.tail != data_values.fifo_write.head))
    {
        U1TXREG = data_values.fifo_write.msg[data_values.fifo_write.tail]; // write the value of circula buffer isndie the uart1 buffer
        
        if (data_values.fifo_write.tail >= DIMFIFOWRITE - 1)
        {
            data_values.fifo_write.tail = 0;
        }
        else
        {
            data_values.fifo_write.tail++;
        }
    }
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
    float batt_data = bit2volt((float) potBitsIr);
    cd->infraRed_data = volt2cm(batt_data);
}

void taskPrintBattery (void* param)
{
    data* cd = (data*) param;
    char buff[20];

    sprintf(buff,"$MBATT,%.3f*", cd->battery_data); 
    
    // write the values of the battery register
    for(int i = 0; buff[i] != 0; i++)
    {
        if (cd->fifo_write.head != cd->fifo_write.tail -1)
        {
            cd->fifo_write.msg[cd->fifo_write.head] = buff[i];
            
            if (cd->fifo_write.head >= DIMFIFOWRITE - 1)
            {
                cd->fifo_write.head = 0;
            }
            else
            {
                cd->fifo_write.head++;
            }
        }
    } 
    if (U1STAbits.UTXBF == 0)
    {
        IFS0bits.U1TXIF = 1;
    }
}

void taskPrintInfrared (void* param)
{
    data* cd = (data*) param;
    char buff[20];

    sprintf(buff,"$MDIST,%.3f*", cd->infraRed_data); 
    
    // write the values of the infrared register
    for(int i = 0; buff[i] != 0; i++)
    {
        if (cd->fifo_write.head != cd->fifo_write.tail -1)
        {
            cd->fifo_write.msg[cd->fifo_write.head] = buff[i];
            
            if (cd->fifo_write.head >= DIMFIFOWRITE - 1)
            {
                cd->fifo_write.head = 0;
            }
            else
            {
                cd->fifo_write.head++;
            }
        }
    }  
    if (U1STAbits.UTXBF == 0)
    {
        IFS0bits.U1TXIF = 1;
    }
}

