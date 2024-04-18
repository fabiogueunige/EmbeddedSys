/*
 * File:   main.c
 * Author: fabio
 *
 * Created on April 9, 2024, 4:26 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"

void myfunction(int ,int ); // function that use 7 ms to be completed
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void); // interrupt tp blink the led
short int count = 0; // count for timer 2

// interrupt function for reception
void __attribute__((__interrupt__, __auto_psv__)) _U1RXInterrupt(void);
// global array of char to save he charcaters
char msg[3]; // array to save the char
int char_received = 0; // number of char received from RX

// part of T2 button
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void); // mappable (butto in our case) interrupt

// part of the timerr to check when outside 10ms the main code
void __attribute__((__interrupt__, __auto_psv__)) _T4Interrupt(void); 
void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void); // mappable (butto in our case) interrupt
int deadline_count = 0; // counter of missed deadlines of the main

int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
    
    // timer managing
    tmr_setup_period (TIMER2, 200);
    tmr_setup_period (TIMER4, 10);
    // timer 1 for myfunction
    // timer 2 for the blink of led2 (blink every two times the variable expired)
    // timer 3 to wait period 3ms
    // timer 4 to check to respect 100Hz
    IFS0bits.T3IF = 0; // reset interrupt flag
    IEC0bits.T2IE = 1; // enable interrupt timer 2
    IFS0bits.T2IF = 0; // reset flag timer 2
    IFS1bits.T4IF = 0; // reset interrupt flag
    IEC1bits.T4IE = 1; // eneble interrrupt flag
    
    // Mapping INT1 to RE8 pin of the left button T2 (button managing)
    RPINR0bits.INT1R = 0x58; // button address
    IFS1bits.INT1IF = 0; // clear the interrupt flag
    IEC1bits.INT1IE = 1; // enable interrupt
    
    // Mapping INT2 to RE9 pin of the left button T3 (button managing)
    RPINR1bits.INT2R = 0x59; // button address
    IFS1bits.INT2IF = 0; // clear the interrupt flag
    IEC1bits.INT2IE = 1; // enable interrupt
    
    // remap UART1 pins
    RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) (remapUARTpin = funcionality)
    RPINR18bits.U1RXR = 0b1001011; // virtual remap for input, assign directly 
    // function to pinrx of UART (functionality = remappable input pin)
    
    // set the baud rate and UART
    // U1BRG = round((FCY /(16LL * BAUD_RATE))-1); // (better directly on integer)
    U1BRG = 468; // setting the boud rate register
    U1MODEbits.UARTEN = 1; // enable uart
    
    U1STAbits.UTXEN = 1; // UART trasmission enable
    
    // enable for UART flag and interruptts
    IFS0bits.U1RXIF = 0; // setting the flag for reception to 0
    IEC0bits.U1RXIE = 1; // enable interrupt for UART 1 receiver
    
    // Making the led alive    
    TRISGbits.TRISG9 = 0; // set pin to output of led 2
    LATGbits.LATG9 = 1; // write on the pin to turn on the led  
    TRISAbits.TRISA0 = 0; // set pin to output
    LATAbits.LATA0 = 0; // write on the pin to turn on the led
    
    // Enable always after the setting
    
    
    while(1){
        myfunction(TIMER1, 7);
        // wait_ms wrong because every time you loose the 10ms period
        tmr_wait_ms(TIMER3, 3); // to have the 100Hz   
        // Check the deadline not through an interrupt but with the ruturn value
        // of the setup period
    }
    
    return 0;
}


void myfunction(int tmr, int tiempo)
{
    // wait 7 ms with Timer given
    tmr_wait_ms(tmr, tiempo);
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) 
{
    IFS0bits.T2IF = 0; // reset interrupt flag
  
    if (count == 2)
    {
        count = 0;
        TRISGbits.TRISG9 = 1; // set pin  to read 
        // Check the value on the letch without changing the reading and output of the pin
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
    else {
        count = count + 1;
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _T4Interrupt(void)
{
    IFS1bits.T4IF = 0; // cleaning the flag of the interrupt
    
    if (IFS0bits.T3IF == 0) // chek timer wait period
    {
        deadline_count++; // one more missed deadlines
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0; // setting the flag for reception to 0
    
    for (int i = 1; i >= 0; i--)
    {
        msg[i+1] = msg[i];
    }
    // do while to empty the buffer to not loose characters
    msg[0] = U1RXREG;
    char_received ++;
    
    if (msg[2] == 'L' && msg[1] == 'D')
    {
        if (msg[0] == '2'){
            if (IEC0bits.T2IE == 1){
                IEC0bits.T2IE = 0; // stop the blinking of the led2
            }
            else {
                IEC0bits.T2IE = 1; // restart the blinkink of the led2
            }
        }
        if (msg[0] == '1')
        {
            TRISAbits.TRISA0 = 1; // set pin  to read 
            if (PORTAbits.RA0 == 0) // reading the pin
            {
                TRISAbits.TRISA0 = 0; // set pin to output
                LATAbits.LATA0 = 1; // write on the pin to turn on the led
            }
            else 
            {
                TRISAbits.TRISA0 = 0; // set the pin as output
                LATAbits.LATA0 = 0; // write on thhe pin to off the led   
            }
        }
    }
}

void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
{
    IFS1bits.INT1IF = 0; //put to zero the flag
    // trasmission has to be done on the main and in this case is not useful the interrupt
    // the flag of the INT1 is ennough
    // Always check if the buffer is available to write the charcatters
    // If debouncing checked then use interrupt and use a globl variable to make 
    // the main write on the register)
    U1TXREG = 'C'; // can send only one charcters per time
    U1TXREG = '=';
    IEC0bits.U1RXIE = 0; // disable interrupt for UART 1 receiver
    // this is to access to the share memory
    U1TXREG = (char) char_received; // send tha number of characters sent
    IEC0bits.U1RXIE = 1; // enable interrupt for UART 1 receiver
}

void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void)
{
    IFS1bits.INT2IF = 0; // clear the interrupt flag
    // check other trasmission to see problems
    int val;
    U1TXREG = 'D'; // can send only one charcters per time
    U1TXREG = '=';
    val = round(deadline_count/256); 
    U1TXREG = (char) val;
    U1TXREG = (char) deadline_count; // send tha number of characters sent
}