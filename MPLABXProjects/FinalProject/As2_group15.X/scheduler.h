#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include <xc.h> // include processor files - each processor file is guarded.  

typedef struct {
    int n; // counter for the task
    int N; // end of the counter (total number of repetitions)
    int enable; // if to activate it or not (1 = active)
    void (*f)(void *);
    void* params;
} heartbeat;

void scheduler(heartbeat schedInfo[], int nTasks);

#endif	/* SCHEDULER_H */

