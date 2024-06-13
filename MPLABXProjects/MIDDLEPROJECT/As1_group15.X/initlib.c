/*
 * File:   initlib.c
 * Author: fabio
 *
 * Created on 14 maggio 2024, 11.23
 */


#include "xc.h"
#include "initlib.h"


void init_config(){
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    INTCON2bits.GIE = 1; // set global interrupt enable 
}

void myfunction(int tmr, int tiempo)
{
    // wait 7 ms with Timer given
    tmr_wait_ms(tmr, tiempo);
}


