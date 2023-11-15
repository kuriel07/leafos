#ifndef _MINOS_MACHINE_H
//this is machine dependent code for atmega series
//#include <avr/interrupt.h>
////#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
//#include <avr/pgmspace.h>
//#include "2440addr.h"
#include "..\..\defs.h"

/*register int r0 __asm("r0");
register int r1 __asm("r1");
register int r2 __asm("r2");
register int r3 __asm("r3");
register int r4 __asm("r4");   
register int r5 __asm("r5");
register int r6 __asm("r6");
register int r7 __asm("r7");
register int r8 __asm("r8");
register int r9 __asm("r9");   
register int r10 __asm("r10");
register int r11 __asm("r11");
register int r12 __asm("r12");
register int r13 __asm("r13");
register int r14 __asm("r14");
register int r15 __asm("r15");
register int sp __asm("sp");
register int lr __asm("lr");
register int pc __asm("pc"); */

#define XTAL 12000000
#define KERNEL_TICK ((XTAL / 5000) - 128)

/** START OF KERNEL INTERNAL PROCESS, FOR ATMEGA SERIES PORTABILITY PURPOSE **/
//initialize timer and interrupt timer

extern void __irq os_isr_timer(void);

//force kernel to switch task  (set flag on interrupt timer by software)
#define os_tick() {\
}



uint32 os_enter_critical(void);
void os_exit_critical(uint32 val);
void os_clear_int_flag(void);

void os_init_machine(void * ctx);
void os_start(void);
/*#define os_init_machine() { \
	pISR_TIMER0 = (unsigned)IsrTimer0;		\
    ClearPending(BIT_TIMER0);				\
    rINTMSK &= ~(BIT_TIMER0);				\
} */


/** END OF KERNEL INTERNAL PROCESS, FOR ATMEGA SERIES PORTABILITY PURPOSE **/
uint32 os_save_context_safe(uint32 istack, uint32 ustack, uint32 vector);
void os_mon_putc(uchar c);
uchar os_mon_getc(void);

#define _MINOS_MACHINE_H
#endif
