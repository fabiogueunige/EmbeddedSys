 /* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef ADCLIB_H
#define	ADCLIB_H

#include <xc.h> // include processor files - each processor file is guarded.  

void adc_scanmode_config();
float battery_conversion (float ); 
float bit2volt (float );
float volt2cm(float );


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif 

#endif	

