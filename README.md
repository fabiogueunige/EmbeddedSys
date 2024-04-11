
# EmbeddedSys

Embedded System 

The repository is organized in this way:
The exercise done are inside the macrofolder with the name of the exercise and inside is divided with the proposal exercise.

## TIMER

This folder contain only the exercise three of the slides. This folder contains also the correct function `timer.c` with all the timer implemented: TIMER1, TIMER2, TIMER3, TIMER4, TIMER5.

Remember each time to import also the header 'timer.h' This header in this project contain also the define needed for timer, is importanto to include tin this header all the define needed for teh sepcific project.
In this folder there are present three functions:

- `tmr_setup_period(int timer, int ms)`:

This function set up the timer with the specific ms, maximum 200 ms, and start it. 

```c
T1CONbits.TCKPS = 3;
    TMR1 = 0; // reset timer count
    IFS0bits.T1IF = 0; // set the flag to zero  
    PR1 = ((((float) FCY) / prescaler) * (((float) ms) / toSec)); //((144 M/2)/256) x0.2
    T1CONbits.TON = 1; // start timer
```

- `tmr_wait_period(int timer)`

```c
 // return 1 if timer has expired, otherwise returns 0
    if(timer == TIMER1){
        if (IFS0bits.T1IF == 1)
        {
            IFS0bits.T1IF = 0;
            return 1;
        } 
    }else if(timer == TIMER2){
        if (IFS0bits.T2IF == 1)
        {
            IFS0bits.T2IF = 0;
            return 1;
        }
    }    
    return 0;
```

- `void tmr_wait_ms(int timer, int ms)`

```c
tmr_setup_period(timer, ms);
    while(tmr_wait_period(timer) != 1);  
```

### Timer.X

In this exercise are done the exercise 2 and 3 togheter. In this exercise the 'LED1' and 'LED2' will blink at different frequency. In this exercise we will use twice the function tmr_wait_ms and tmr_waipt_period for test both the functions.

## INTERRUPTS

thing to remember about remappable pin:

For the input pin to remap i associate a funcionality to a specific pin.

- **functionality = Remappable Input Pin**.

es: maps the U1RX to RPI20

```c
TRISbits.TRISA4 = 1 // RP120 is a RA4
RPINR11bits.U1RXR = 20
```

For the output, I do the duale, I associate the pin to a specific funcionality
- **remappable Out Pin = Functionality (Pheriperal Output)**.
For exampe, I want to assciate the U1TX on RP104

```c
RP0R11bits.RP104R = 1 // 1 = U1TX
```

- Example of remmaping the pin RE8 connected to button 2 with the interrupt INT1. IN this case is a case when the value arrive, press the button e wite activate a flag of the input, so we associate the  function INT1 eith the remappable out pin associate at this pin, that is R88, 0x58 in hesadecimal.\
RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal connecting to remappable pin


## UART RS232
About remappable pin:

- When trasmitt I associate the pin to a specific functionality.
- When I receive I read the value that arrived on the pin and I associate it to a register. 
