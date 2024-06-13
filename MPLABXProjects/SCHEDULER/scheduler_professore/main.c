
// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FPR = XT_PLL16          // Primary Oscillator Mode (XT w/PLL 16x)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "spi.h"
#include "sabertooth.h"
#include "uart.h"
#include "scheduler.h"
#include "peripherals.h"

#define GEAR_RATIO 29.75
#define MAX_TASKS 7

typedef struct {
    double setpoint; // degrees
    double positionFeedback; // degrees
    double encoder; // encoder counts
    double motorPercentage; // percentage of power
} control_data;

// global volatile data shared between the interrupt and one task
volatile double encoder;

void __attribute__((__interrupt__, __auto_psv__)) _QEIInterrupt(void)
{
	IFS2bits.QEIIF = 0;
	
	if (QEICONbits.UPDN == 1) {
		encoder += MAXCNT;
	} else {
		encoder -= MAXCNT;
	}
}

void task_update_feedback(void* param)
{
    control_data *cd = (control_data*) param;
    
    // disabling and renabling the interrupt because we are accessing 
    // a global variable
    IEC2bits.QEIIE = 0;
    cd->positionFeedback = (encoder + POSCNT) / GEAR_RATIO / 12.0 * 360.0;
    cd->encoder = encoder + POSCNT;
    IEC2bits.QEIIE = 1;
}

void task_send_uart(void* param)
{
    control_data *cd = (control_data*) param;
    
    char str[30];
    sprintf(str, "$FBK,%.1f,%.1f,%.1f*", cd->setpoint, cd->positionFeedback, cd->motorPercentage);
    uart_write_str(str);
}

void task_read_potentiometer(void* param)
{
    double *setpoint = (double*) param;
    while(!ADCON1bits.DONE);
    int potBits = ADCBUF0;
    *setpoint = 360 * (potBits / 1024.0);
}

void task_blink_led(void* param)
{
    LATBbits.LATB0 = !LATBbits.LATB0;
}

void task_pid_controller(void* param)
{
    control_data *cd = (control_data*) param;

    
    //ssabertooth_motor1_drive(cd->motorPercentage);
}

void task_open_loop_controller(void* param)
{
    control_data* cd = (control_data*) param;
    
  
       
    //sabertooth_motor1_drive(cd->motorPercentage);
}


void task_lcd(void* param)
{
    control_data *cd = (control_data*) param;
    
    spi_clear_first_row();
    spi_move_cursor(FIRST_ROW, 0);
    
    char str[17];
    sprintf(str, "%.1f", cd->setpoint);   
    spi_put_string(str);
    
    spi_clear_second_row();
    spi_move_cursor(SECOND_ROW, 0);

    sprintf(str, "P:%.1f u:%.1f", cd->positionFeedback, cd->motorPercentage);
    spi_put_string(str);
}

void task_button(void* param)
{
    static int prevButtonE8 = 1;
    static int prevButtonD0 = 1;
    control_data *cd = (control_data*) param;
    
    int currButton = PORTEbits.RE8;
    if (currButton == 0 && prevButtonE8 == 1) {
       encoder = 0;
       POSCNT = 0;
    }
    prevButtonE8 = currButton;
    
    currButton = PORTDbits.RD0;
    if (currButton == 0 && prevButtonD0 == 1) {
        if (cd->setpoint == 0)
            cd->setpoint = 360;
        else 
            cd->setpoint = 0;
    }
    prevButtonD0 = currButton;
}


int main(void) 
{
    tmr_wait_ms(TIMER2, 500); // required for the LCD micro
    tmr_wait_ms(TIMER2, 500); // required for the LCD micro
    
    // digital I/O configuration
    TRISBbits.TRISB0 = 0; // D3
    TRISBbits.TRISB1 = 0; // D4
    TRISEbits.TRISE8 = 1; // S5
    TRISDbits.TRISD0 = 1; // S6

    // main timer configuration
    tmr_setup_period(TIMER1, 5);

    // peripherals configuration
    spi_configure();
    qei_configuration();
    adc_configuration();
    uart_configuration();
    
	encoder = 0; // initialization of the global variable
    control_data controlData;
    
    // scheduler configuration
    heartbeat schedInfo[MAX_TASKS];
    
    schedInfo[0].n = 0;
    schedInfo[0].N = 1;
    schedInfo[0].f = task_read_potentiometer;
    schedInfo[0].params = (void*)(&controlData.setpoint);
    schedInfo[0].enable = 0;
    
    schedInfo[1].n = 0;
    schedInfo[1].N = 1;
    //schedInfo[1].f = task_pid_controller;
    schedInfo[1].f = task_open_loop_controller;
    schedInfo[1].params = (void*)(&controlData);
    schedInfo[1].enable = 1;
    
    schedInfo[2].n = 0;
    schedInfo[2].N = 100;
    schedInfo[2].f = task_blink_led;
    schedInfo[2].params = NULL;
    schedInfo[2].enable = 1;
    
    schedInfo[3].n = 0;
    schedInfo[3].N = 100;
    schedInfo[3].f = task_lcd;
    schedInfo[3].params = (void*)(&controlData);
    schedInfo[3].enable = 1;
    
    schedInfo[4].n = 0;
    schedInfo[4].N = 4;
    schedInfo[4].f = task_button;
    schedInfo[4].params = (void*)(&controlData);
    schedInfo[4].enable = 1;
    
    schedInfo[5].n = 0;
    schedInfo[5].N = 1;
    schedInfo[5].f = task_update_feedback;
    schedInfo[5].params = (void*)(&controlData);
    schedInfo[5].enable = 1;
    
    schedInfo[6].n = 0;
    schedInfo[6].N = 2;
    schedInfo[6].f = task_send_uart;
    schedInfo[6].params = (void*)(&controlData);
    schedInfo[6].enable = 1;
    
    
    // main loop
    while (1) {
        scheduler(schedInfo, MAX_TASKS);
        LATBbits.LATB1 = tmr_wait_period(TIMER1);
    }
    
    return 0;
}
