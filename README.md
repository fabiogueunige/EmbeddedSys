
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

- For first thing in important to enable the global interrupt enable:
```c
INTCON2bits.GIE = 1;
```

- Reset the flag in the specific register IFSxbits.---
- Enable teh interrupt in the register IECxbits.---- for the specific interrupt 
thing to remember about remappable pin:

For the input pin to remap i associate a funcionality to a specific pin.

es: maps the U1RX to RPI20

- **functionality = Remappable Input Pin**.

```c
TRISbits.TRISA4 = 1 // RP120 is a RA4
RPINR11bits.U1RXR = 20
```

For the output, I do the duale, I associate the pin to a specific funcionality
- **remappable Out Pin = Functionality (Pheriperal Output)**.
For exampe, I want to assciate the RP104 on U1TX

```c
RP0R11bits.RP104R = 1 // 1 = U1TX
```

- Example of remmaping the pin RE8 connected to button 2 with the interrupt INT1. IN this case is a case when the value arrive, press the button e wite activate a flag of the input, so we associate the  function INT1 eith the remappable out pin associate at this pin, that is R88, 0x58 in hesadecimal.

```c
RPINR0bits.INT1R = 0x58; // 0x58 is 88 in hexadecimal connecting to remappable pin
```



## UART RS232

Is a asynchronous communication.
- Full duplex 8 or 9 bit data trasmission
- One or two stop bits
- 4-deep FIFO transmitt data buffer
- Transmitt and receive interrupts
- Can trasmitt only one bytes (8 bit) 

#### REMAPPABLE PIN


- When trasmitt I associate the pin to a specific functionality.
- When I receive I read the value that arrived on the pin and I associate it to a register. 
  
```C
// remap UART1 pins
RPOR0bits.RP64R = 1; // remap the pin tx of UART1 (U1TX) (remappable output pin = funcionality)
RPINR18bits.U1RXR = 0b1001011; // remap the pin rx of UART1 (U1RX) (functionality = remappable input pin)
```

#### UART TRASMIT

for setup trasmission follow this passage:
* initialize the **UxBRG** register for the appropriate baud rate: ((FCY/BAUDRATE)/16)-1
* set the UxTXIE control bit in the corresponding interrupt enable control register IEC, this for enable the interrupt
* Enable the UART module by setting the **UARTEN** bit (UxMODE<15>)
* Enable the transmission by setting the **UTXEN** bit (UxSTA<10>), which will also set the **UxTXIF** bit., Remember to put to 0 im the interrupt function
* Load data into **UxTXREG** register (start trasmission)

#### UART RECEIVER
- Initialize the **UxBRG** register for the appropriate baud rate (see 17.3 “UART Baud Rate Generator”).
- If interrupts are desired, set the **UxRXIE** bit in the corresponding Interrupt Enable Control register (IEC). 
- Enable the UART module by setting the **UARTEN** bit (UxMODE<15>)
- Read data from the receive buffer **UxRXREG** that contains the character received