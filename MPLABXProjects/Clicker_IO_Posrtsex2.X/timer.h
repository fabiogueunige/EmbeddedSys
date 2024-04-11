#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
// TODO Insert appropriate #include <>
#define TIMER1 1
#define TIMER2 2
#define FCY 72000000

/*ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    int pinValue = 0;
    TRISAbits.TRISA0 = 0; //otput
    
    tmr_setup_period (TIMER1, 200); // init of the timer
    
    while(1)
    {   
        if (pinValue == 0) // reading the pin
        {            
            LATAbits.LATA0 = 1; // write on thhe pin to on the led
            pinValue = 1;
        }
        else 
        {
            LATAbits.LATA0 = 0; // write on thhe pin to off the led
            pinValue = 0;
        }
        tmr_wait_period(TIMER1); 
    }    
    return 0;*/

void tmr_setup_period(int timer, int ms);
int tmr_wait_period(int timer);
void tmr_wait_ms(int timer, int ms);
// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

