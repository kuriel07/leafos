/*!\file 		timer.h
 * \brief     	timer driver 
 * \details   	timer driver for interval management
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \pre       	
 * use timer 2
 * provide mechanism to create interval with 1 second different
\verbatim	
********************************************************************
1.0
 * initial release (2016.03.18)
********************************************************************
\endverbatim
 */
 
#ifndef IF_TIMER__H
#define IF_TIMER__H
#include "..\..\defs.h"
#include "..\..\config.h"
typedef struct timer_context timer_context;
typedef struct timer_context * timer_context_p;

struct timer_context {
	uint8 magic;
	timer_context * next;
	uint32 curtimer;
	uint32 cmptimer;
	void (* callback)(void * params);
	void * params;
};

void * if_timer_init(void * display);
void * if_timer_create(void (* callback)(void * params), void * params, uint32 tick);
void if_timer_delete(void * timer);
void if_timer_set_tick(uint16 tick);		//set current timer tick, (from callback only)
uint8 if_timer_sleep(void);
void if_timer_wake(void) ;
#endif
