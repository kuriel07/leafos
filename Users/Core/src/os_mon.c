#include "..\..\defs.h"
#include "..\inc\os_mach.h"
#include "..\inc\os_core.h"
#include <stdio.h>


//Output Compare Match A Interrupt Vector
//round robin task switcher
/*void __irq IsrTimer0(void)
{
	//disable all interrupts
	//THERE MUST BE NO PROCESS HERE
	//ENTER CRITICAL
	register int _ssp __asm("r0");	   //saved stack pointer
	//mov r1,sp
	_save_context();
	_os_enter_critical();
	//START PROCESS
	//change task here
	__asm { 
		str sp, [r0] 
	}
	_ssp = _os_tick_isr(_ssp);	 
	__asm { 
		ldr sp, [r0]
	}			
	//while(1);
	//while(1);
	//END OF PROCESS
	//EXIT CRITICAL
	_os_exit_critical();
	_restore_context();
	//enable all interrupts
	//THERE MUST BE NO PROCESS HERE
	//asm volatile ( "sei" );	//EXIT CRITICAL
	//asm volatile ( "ret" );	//i don't believe this compiler
	__asm{
		mov pc,lr
	}
	//__asm("stmia	r0!,{r1-r8}");
} */

//static int uart_putchar(char c, FILE *stream) {}

//static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
//										 _FDEV_SETUP_WRITE);

static int uart_putchar(char c, FILE *stream)
{
  /*if (c == '\n')
	uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c; */
  return 0;
}

static int uart_getchar() {
	uchar c = 0;
	/*loop_until_bit_is_set(UCSRA, RXC);
	c = UDR;*/
	return c;
}

void os_mon_putc(uchar c) {
	//uart_putchar(c, &mystdout);
}

uchar os_mon_getc(void) {
	return uart_getchar();
}
