/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SPILIB_H
#define	SPILIB_H

#include <xc.h> // include processor files - each processor file is guarded.  
// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

void spi_config();
void chip_selecting();
int spi_write(unsigned int );

// TODO Insert declarations

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

