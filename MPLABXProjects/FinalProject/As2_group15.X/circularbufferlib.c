/*
 * File:   circularbufferlib.c
 * Author: fabio
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