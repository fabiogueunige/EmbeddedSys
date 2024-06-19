/*
 * File:   pwmlib.c
 * Author: Chiappe, Di Donna, Guelfi, Utegaliyeva
 *
 * Created on 27 maggio 2024, 12.37
 */


#include "xc.h"
#include "pwmlib.h"

void pwmRemap()
{
    OC1CON1 = 0; // good practice to clear off the control bits initially
    OC1CON2 = 0; // good practice to clear off the control bits initially
    
    // setting as output
    TRISDbits.TRISD1 = 0; // set as output
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0; // set as output
    TRISDbits.TRISD4 = 0; // set as output to move forward  
    
    
    // pin remap for pwm (H-bridge)
    RPOR0bits.RP65R = 0b010000; // RD1 corresponds to OC1 the remappable pin RD1 = RP65R
    RPOR1bits.RP66R = 0b010001; // RD2 corresponds to OC2 the remappable pin RD1 = RP66R
    RPOR1bits.RP67R = 0b010010; // RD3 corresponds to OC3 the remappable pin RD1 = RP67R
    RPOR2bits.RP68R = 0b010011; // RD4 corresponds to OC1 the remappable pin RD1 = RP68R
    
    // configure the output compare 
    OC1CON1bits.OCM = 0b000; // disable before to configure
    OC2CON1bits.OCM = 0b000; // disable before to configure
    OC3CON1bits.OCM = 0b000; // disable before to configure
    OC4CON1bits.OCM = 0b000; // disable before to configure
}

void pwmParametrization()
{
    // selct for clock for the output compare
    OC1CON1bits.OCTSEL = 0b111; // select the input clock  source for the OC1module 
    OC2CON1bits.OCTSEL = 0b111; // select the input clock  source for the OC1module 
    OC3CON1bits.OCTSEL = 0b111; // select the input clock  source for the OC1module 
    OC4CON1bits.OCTSEL = 0b111; // select the input clock  source for the OC1module 
     
    // as peripheral clock that is the same to microcontroller
    OC1CON1bits.OCM = 0b110; // edge aligned PWM mode (high when OCxTMR = 0, low for OCxTMR = OCxRS)
    OC2CON1bits.OCM = 0b110; // edge aligned PWM mode (high when OCxTMR = 0, low for OCxTMR = OCxRS)
    OC3CON1bits.OCM = 0b110; // edge aligned PWM mode (high when OCxTMR = 0, low for OCxTMR = OCxRS)
    OC4CON1bits.OCM = 0b110; // edge aligned PWM mode (high when OCxTMR = 0, low for OCxTMR = OCxRS)

    // set the peiod of the pwm at 10khz dividing fcy/10
    OC1RS = PERIOD;
    OC2RS = PERIOD;
    OC3RS = PERIOD;
    OC4RS = PERIOD;
}

void pwmConfig()
{
    // standard pwm configuration
    pwmRemap();
    pwmParametrization();
    whstop();  
}

void moveForward (int velocity)
{
    // function to move forward
    OC1R = WHNULL;
    OC2R = velocity;
    OC3R = WHNULL;
    OC4R = velocity;
}

void moveBack(int velocity)
{
    // function to move back
    OC1R = -velocity;
    OC2R = WHNULL;
    OC3R = -velocity;
    OC4R = WHNULL;
}

void whstop()
{
    // function to stop the wheel
    OC1R = WHNULL;
    OC2R = WHNULL;
    OC3R = WHNULL;
    OC4R = WHNULL;
}

void moveLeft(int velocity)
{
    // function to move left
    OC1R = WHNULL;
    OC2R = WHNULL;
    OC3R = WHNULL;
    OC4R = velocity;
}

void moveRight(int velocity)
{
    // function to move right
    OC1R = WHNULL;
    OC2R = velocity;
    OC3R = WHNULL;
    OC4R = WHNULL;
}

void input_move(int input, int mode)
{
    switch (input)
    {
        case FORWARD:
            moveForward(mode);
            break;
        case COUNT_ROTATION:
            moveLeft(mode);
            break;
        case CLOCKWISE_ROTATION:
            moveRight(mode);
            break;
        case BACKWARD:
            moveBack(mode);
            break;
        default:
            whstop();
            break;
    }
}
