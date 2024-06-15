/*
 * File:   main.c
 * Author: Chiappe, Di Donna, Guelfi, Utegaliyeva
 *
 * Created on April 23, 2024, 12:48 PM
 */


#include "xc.h"
#include <math.h>
#include "timer.h"
#include "initlib.h"
#include "spilib.h"
#include "uartlib.h"
#include <stdio.h>


#define DIMSPI 5 // counter for spi
#define MAGFREQ 4 // couinter for magnetometer
#define UARTFREQ 20 // counter for the uart
#define DIMUB 50 // dimension of the circular buffer


void printGrad(float); // printing the grad
void printImu(float, float, float);  // print the value of the mean on the three axis 
float magnAcquisition (int , int , int, int); // function to acquire x,y,x data from magnetometer
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void);

// global variables (indixes)
int ind_spi = 0, ind_buf_write = 0, ind_buf_read = 0;
float x[DIMSPI], y[DIMSPI], z[DIMSPI]; // value to save from the magnetometer
float xavg = 0, yavg = 0, zavg = 0;
char UBuffer[DIMUB];


int main(void) {
     
    init_config();
    
    spi_config();
    
    chip_selecting();

    // variable definition
    int cnt = 0; // counter for...
    float grad = 0; // 
    
    // Remap for SPI (Magnetometer)
    // Setting the input/output ode
    TRISAbits.TRISA1 = 1; // set the pin RA1/RPI17 as input (because MISO)
    TRISFbits.TRISF12 = 0; // set the pin RF12/RP108 as output (because CLOCK)
    TRISFbits.TRISF13 = 0; // set the pin RF13/RP109 as output (because MOSI)
    
    // Functionality associated to the pin setting
    RPINR20bits.SDI1R = 0b0010001; // (functionality = pin) RPI117 associated to the SPI Data Input (SDI1)
    RPOR12bits.RP109R = 0b000101; // (pin = functionality) RP109 tied to SPI1 Data output (SDO1) 
    RPOR11bits.RP108R = 0b000110; // RP108 tied to SPI1 Clock output (SCK1)    
    
    // function for configure the uart in transmission
    tr_uart_config();
       
    // Setting of the interrupt for transmission
    IFS0bits.U1TXIF = 0; // resetting U1TX interrupt flag
    IEC0bits.U1TXIE = 0; // disabling U1TX interrupt 
    
    // Magnetometer activation
    // Going on sleep mode and waiting before to go on active (needed for SPI)
    LATDbits.LATD6 = 0; // To advise we are sending data
    spi_write(0x4B);
    spi_write(0x01);
    LATDbits.LATD6 = 1; // To alert the end of the communication  
    tmr_wait_ms (TIMER3, 2);
    
    // Going on active mode changing the magnetometer output data rate
    LATDbits.LATD6 = 0;
    spi_write(0x4C); 
    spi_write(0b00110000); // instruction for change the data rate at 25hz
    LATDbits.LATD6 = 1;
    tmr_wait_ms (TIMER3, 2);
        
    // timer setup
    tmr_setup_period (TIMER2, 10); // for syncronize the main
    
    IEC0bits.U1TXIE = 1; // activate the interrupt for the uart transmission
        
    while(1)
    {
        
        myfunction(TIMER5, 7); 
        
        if (cnt % MAGFREQ == 0) // read the magnetometer every 25Hz, 40ms
        {
            LATDbits.LATD6 = 0; // chip select associated at magnetometer 
            spi_write(0x42 | 0x80); // So next time we will read the lsb of x magnetometer
            
            // acquiring x values from the magnetometer
            x[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8, 8);
                     
            // acquiring y values from the magnetometer
            y[ind_spi] = magnAcquisition(0x00, 0x00, 0xF8, 8);
 
            // acquiring z values from the magnetometer
            z[ind_spi] = magnAcquisition(0x00, 0x00, 0xFE, 2);
            LATDbits.LATD6 = 1; // chip select

            // control on spi index of the circular buffer
            if (ind_spi >= DIMSPI) 
            {
                ind_spi = 0;
            }
            else {
                ind_spi++;
            }
        }
        // enter in this if every 200 ms
        if ((cnt % UARTFREQ == 0) && (cnt != 0))
        {
            cnt = 0;
            // calulate the mean of the last 5 value acquired from magnetometer
            for (int i = 0; i < DIMSPI; i++)
            {
                xavg += x[i]; 
                yavg += y[i];
                zavg += z[i];
            }
            xavg /= DIMSPI;
            yavg /= DIMSPI;
            zavg /= DIMSPI;
            // function for 
            printImu(xavg, yavg, zavg);
            
            // gradient computation
            grad = atan2 (yavg, xavg);
            grad = (grad * 180.0) / M_PI;
            printGrad(grad);
            
        }
        cnt ++;
        
        while(!tmr_wait_period(TIMER2));
    }
    return 0;
}

// function that give as input the mean of x, y, z and put it inside the circular buffer. 
// If the buffer of uart is empty set the uart flag to 1
void printImu(float x, float y, float z)
{
    char buff[30];

    sprintf(buff,"$MAG,%.2f,%.2f,%.2f*", x, y, z); 
    
    // copy the mean value comes from spi inside the circular buffer
    for(int i = 0; buff[i] != 0; i++)
    {
        if (ind_buff_write != ind_buff_read -1) 
        {
            UBuffer[ind_buf_write] = buff[i];

            if (ind_buf_write == DIMUB -1)
            {
                ind_buf_write = 0;
            }
            else 
            {
                ind_buf_write ++;
            }    
        }
    }   
    // to send the value trough UART trasmission
    if (U1STAbits.UTXBF == 0){  // check if the buffer is not full
        IFS0bits.U1TXIF = 1;  // set the flag to 1
    }
}

// function for acquiring data from magnetometer
float magnAcquisition (int addr, int next_addr, int mask_lsb, int divider)
{
    int msb = 0, lsb = 0, full = 0;

    lsb = spi_write(addr);
    msb = spi_write(next_addr);

    lsb = lsb & mask_lsb; //  mask for lsb to cancel the bits
    
    msb = (msb << 8); // msb shifted of 8 of left
    
    full = msb | lsb; // the union of lsb and msb shifted
    full = full / divider; // divide the value by 8
    
    return full;
}

// uart1 interrupt function 
void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(void)
{   
    //set the flag to zero
    IFS0bits.U1TXIF = 0;
    
    while((U1STAbits.UTXBF != 1) && (ind_buf_read != ind_buf_write))
    {
        U1TXREG = UBuffer[ind_buf_read]; // write the value of circula buffer isndie the uart1 buffer
        
        if (ind_buf_read >= DIMUB)
        {
            ind_buf_read = 0;
            
        }
        else if (ind_buf_read < DIMUB )
        {
            ind_buf_read++;
        }
    }
}

// function for put the value of the grad inside the circular buffer 
void printGrad(float value)
{
    char buffer[15];
    // conver the float into string
    sprintf(buffer,"$YAW,%.2f", value); 
    
    // copy the value of grad inside the circular buffer (Ubuffer)
    for(int i = 0; buffer[i] != 0; i++)
    {
        UBuffer[ind_buf_write] = buffer[i];

        if (ind_buf_write >= DIMUB)
        {
            ind_buf_write = 0;
        }
        else 
        {
            ind_buf_write++;
        }   
    }
    if (U1STAbits.UTXBF == 0){  // check if the buffer is not full
        IFS0bits.U1TXIF = 1;  // set the flag to 1
    }
}
