/*
 * File:   circularbufferlib.c
 * Author: Chiappe, Di Donna, Guelfi, Utegaliyeva
 *
 * Created on 14 giugno 2024, 16.04
 */


#include "xc.h"
#include "circularbufferlib.h"

void fifo_init(fifo* cb)
{
    cb->head = 0;
    cb->tail = 0;
    for (int k = 0; k < DIMFIFOWRITE; k++)
    {
        cb->msg[k] = '\0';
    }
}

void fifo_pwm_init(fifo_pwm* cb)
{
    cb->head = 0;
    cb->tail = 0;
    for (int k = 0; k < MAX_COMMANDS; k++)
    {
        cb->msg[k][0] = 0;
        cb->msg[k][1] = 0;
    }
}