 /* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PWMLIB_H
#define	PWMLIB_H

#include <xc.h> // include processor files - each processor file is guarded.  


#define WHNULL 0
#define PERIOD 7200

#define WH_FAST 3600 // 50 per cent
#define WH_SLOW 5760 // 80 per cent

#define EMERGENCY_STOP 70
#define PRE_EMERGENCY_STOP 150

#define FORWARD 1
#define COUNT_ROTATION 2
#define CLOCKWISE_ROTATION 3
#define BACKWARD 4

void pwmRemap();
void pwmParametrization();
void pwmConfig(); // standard confiuration
void moveForward (int );
void moveLeft(int );
void moveRight(int );
void moveBack(int );
void whstop();

void input_move(int , int );

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif 

#endif	