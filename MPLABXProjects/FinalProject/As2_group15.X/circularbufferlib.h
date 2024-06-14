#ifndef CB_H
#define	CB_H

#define DIMFIFOWRITE 500

typedef struct {
    int head;
    int tail;
    char msg [DIMFIFOWRITE]; 
} fifo;

void fifo_init();

#endif	/* CB_H */

