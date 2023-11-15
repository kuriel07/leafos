/*
 * core.c
 *
 *  Created on: Nov 27, 2010
 *      Author: Agus Purwanto
 */
#include "..\..\defs.h"
#include "..\inc\os.h"
#include "..\inc\os_core.h"
#include "..\inc\os_config.h"
#include "..\inc\os_mach.h"
#include "..\inc\os_msg.h"
#include <string.h>
#include "..\..\toolkit\inc\tk_apis.h"

#define SYSTEM_STACK_SIZE	128

OSTask * _active_task;
OSTask * _highest_task, * _lowest_task;
#if (MPU_ARCH == MPU_ARCH_16)
uint16 _os_max_tick = 0;
#elif (MPU_ARCH == MPU_ARCH_32)
uint32 _os_max_tick = 0;
#endif

//local prototypes
OSTask * os_get_task_by_pid(uchar pid);
OSTask * os_get_previous_task(OSTask * task);

void os_idle_loop(void);
void os_idle_task(void);
void os_interpret_command(uchar c);
void os_system_idle_task(void);

static uint8 os_msg_count(os_task * task) {
	os_message * msg ;
	uint8 cnt = 0;
	msg = task->messages;
	if(msg == NULL) return 0;
	while(msg != NULL) {
		cnt++;
		msg = msg->next;
	}
	return cnt;
}

//extern void IsrTimer0(void);
uint32 gi_task_utilization = 0;
os_task * g_prev_task = NULL;

__weak void os_tick_callback(void) {
	
}

#if RTK_ENABLE_MONITOR
#include "stdio.h"
uint8 gba_mon_buf[RTK_MONITOR_BUFSIZE];
uint8 gba_mon_len = 0;
extern void os_mon_callback(uint8 *, uint16);
#endif

#if (MPU_ARCH == MPU_ARCH_16)
OSTask * os_tick_isr(uint16 sp, uint16 lp) {
	static uint16 _os_tick = 0;
#elif (MPU_ARCH == MPU_ARCH_32)
OSTask * os_tick_isr(uint32 sp, uint32 lp) {
	static uint32 _os_tick = 0;
#endif
	static OSTask * _task_iterator;
	uchar pid;
#if	RTK_ENABLE_PREEMPTION
	OSTask * _task_candidate = NULL;
#endif
#if RTK_ENABLE_MONITOR
	static uint16 wcntr = 0;
	uint8 * info_p;
	os_task_info info;
	uint8 max_stack;
	uint8 i, lrc;
	uint32 ttl_utilization = 0;
#endif
	os_tick_callback();
	g_prev_task = _active_task;
	if(sp != 0) {		//no exception found, save current task program counter
		if(((OSThread *)_active_task)->state != OS_STATE_DORMANT) {
			((OSThread *)_active_task)->sys_stack = sp;
		}
		((OSThread *)_active_task)->vector = lp;
	} else { 			//exception for this task, switch to next task
		_active_task = _active_task->next;
	}
	_os_tick++;
	//save stack pointer
	_task_iterator = _highest_task;
	while(_task_iterator != NULL) {
		pid = _task_iterator->pid;
		if(((OSThread *)_task_iterator)->state == OS_STATE_WAIT) {
			if(((OSThread *)_task_iterator)->counter_tick > 0) {
				((OSThread *)_task_iterator)->counter_tick--;
				//((OSThread *)_task_iterator)->counter_tick -= _task_iterator->num_thread;					//count down tick
				//enable preemption
#if RTK_ENABLE_PREEMPTION
				if(((OSThread *)_task_iterator)->counter_tick <= 0) {	
					((OSThread *)_task_iterator)->counter_tick = 0;	
					((OSThread *)_task_iterator)->state = OS_STATE_READY;
				}
#endif
			} else {
				((OSThread *)_task_iterator)->counter_tick = 0;
				((OSThread *)_task_iterator)->state = OS_STATE_READY;
			}
		}
		if(_task_candidate == NULL) {
			if(((OSThread *)_task_iterator)->state == OS_STATE_READY || 
				((OSThread *)_task_iterator)->state == OS_STATE_DORMANT || 
				((OSThread *)_task_iterator)->state == OS_STATE_RUNNING) {
				_task_candidate = _task_iterator;
			}
		}
		if(_task_iterator->tag != 0x5A) return _active_task;	  //invalid tag, tcb might be corrupted
		//end of preemption
		_task_iterator = _task_iterator->next;
	}
#if RTK_ENABLE_MONITOR
	((os_thread *)_task_candidate)->utilization++;
	wcntr ++;
	if(wcntr == 1000) {			//send os monitoring data after 1000 ticks
		_task_iterator = _highest_task;
		gba_mon_len = 0;
		while(_task_iterator != NULL) {
			info_p = gba_mon_buf + gba_mon_len + 2;
			gi_task_utilization = 0;
			ttl_utilization += ((os_thread *)_task_iterator)->utilization;
			if(_task_iterator->pid != 0xFFFF) gi_task_utilization += ((os_thread *)_task_iterator)->utilization;
			info_p[0] = _task_iterator->pid;
			info_p[1] = os_msg_count(_task_iterator);
			info_p[2] = ((os_thread *)_task_iterator)->utilization >> 8;
			info_p[3] = ((os_thread *)_task_iterator)->utilization & 0x0FF;
			//calculate maximum stack utilization
			max_stack = 100 - ((((os_thread *)_task_iterator)->sys_stack - ((os_thread *)_task_iterator)->stack) * 100 / ((os_thread *)_task_iterator)->stack_size);
			if(max_stack > ((os_thread *)_task_iterator)->stack_util) 
				((os_thread *)_task_iterator)->stack_util = max_stack;
			info_p[4] = ((os_thread *)_task_iterator)->stack_util;
			info_p[5] = strlen((const char *)_task_iterator->name);
			if(info_p[5] > 6) info_p[5] = 6;
			//memcpy(gba_mon_buf + gba_mon_len + 2, &info, sizeof(os_task_info));
			memcpy(info_p + 6, _task_iterator->name, info_p[5]);
			gba_mon_len += (6 + info_p[5]);
			((os_thread *)_task_iterator)->utilization = 0;		//clear task utilization counter
			_task_iterator = _task_iterator->next;
		}
		gba_mon_buf[0] = 0xDB;
		gba_mon_buf[1] = (gba_mon_len + 1);
		lrc = 0;
		for(i=0;i<gba_mon_len;i++) lrc ^= gba_mon_buf[2 + i];
		gba_mon_buf[2 + gba_mon_len] = lrc;
		gba_mon_len += 3;		//tag-len-data[len]-lrc
		wcntr = 0;
		gi_task_utilization = (gi_task_utilization * 100) / ttl_utilization;
	}
#endif
	//end of preemption
	if(_task_candidate != NULL) _active_task = _task_candidate;
	((OSThread *)_active_task)->state = OS_STATE_RUNNING;				//update state to running, (2018.08.07)
	return _active_task; 
}

uint16 os_get_cpu_util() {
	return gi_task_utilization;
}

void os_idle_loop() {
	uint16 i = 0;
	i = i;
	return;
}

void os_idle_task(void) {
	while(1) {
		//((OSThread *)_active_task)->current_period = 0;		//no timeout
		//os_idle_loop();
	}
}

void os_system_idle_task(void) {   
	uint32 psw;
	while(1) {
		os_wait(500);
		//psw = os_enter_critical();
		if(gba_mon_len != 0) {
			//os_mon_callback((uint8 *)gba_mon_buf, gba_mon_len);
			gba_mon_len =0;
		}
		//os_debug_callback((uint8 *)"aaa", 3);
		//os_exit_critical(psw);
	}
}

void os_init(void * ctx) {
	//initialize memory allocator
	OSTask * task;
	//initialize timer interrupt
	os_init_machine(ctx);
#if RTK_USE_MIDGARD
	m_init_alloc();
#endif
	_active_task = NULL;
	_highest_task = _lowest_task = _active_task;
	//create lowest priority task (idle task)
	task = os_create_task(NULL, os_idle_task, "idle", 0xffff, 512);		//minimum stack set for 512 in-case hardware interrupt occured
	//abc = malloc(sizeof(OSTask));
	task = os_create_task(NULL, os_system_idle_task, "sys", 0xfffe, 512);
	_active_task = _highest_task;
	_os_max_tick = 2560;
}

OSTask * os_create_system_task_static(uchar pid, void (* task_vector), uint16 priority, lp_void stack, uint32 stack_size) {
	OSTask * new_task;
	OSTask * task_iterator;
	OSTask * prev_task = NULL;
	lp_void * stack_pointer;		//prevent memory access alignment error on several machine
	//create new task at heap, use midgard memory manager
	new_task = (OSTask *)os_alloc(sizeof(OSTask));
	//initialize task stack pointer to heap max pointer
	if(stack == NULL) {
		stack_size += SYSTEM_STACK_SIZE;
		((OSThread *)new_task)->stack = (lp_void)os_alloc(stack_size);
	} else ((OSThread *)new_task)->stack = stack;
	((OSThread *)new_task)->sys_stack = ((OSThread *)new_task)->stack + stack_size - sizeof(void *);
	((OSThread *)new_task)->stack_size = stack_size;
	stack_pointer = (lp_void *)((OSThread *)new_task)->sys_stack; 
	*(stack_pointer) = (lp_void)0x01000000;  			//xPSR
	*(--stack_pointer) = (lp_void)task_vector;			//PC
	*(--stack_pointer) = (lp_void)0;							//LR
	*(--stack_pointer) = (lp_void)0;							//R12
	*(--stack_pointer) = (lp_void)0;							//R3
	*(--stack_pointer) = (lp_void)0;							//R2
	*(--stack_pointer) = (lp_void)0;							//R1
	*(--stack_pointer) = (lp_void)0;							//R0  
	*(--stack_pointer) = (lp_void)0xFFFFFFF9; 			//linked register 
	((OSThread *)new_task)->sys_stack = (lp_void)stack_pointer;
	//new_task->Stack -= 4;
	//main thread initialization	  
	((OSThread *)new_task)->tid = 0;
	((OSThread *)new_task)->state = OS_STATE_DORMANT;
	((OSThread *)new_task)->vector = (lp_void)task_vector;
	((OSThread *)new_task)->next = NULL; 
	((OSThread *)new_task)->counter_tick = 0; 
	((OSThread *)new_task)->stack_util = 0; 

	//default tlb use system TLB  
	//new_task->base_tlb = (lp_void)_MMUTT_STARTADDRESS;
	//default malloc, use system malloc
	//new_task->heap = (lp_void)_HEAP_STARTADDRESS;
	//set task state to dormant		   
	new_task->tag = 0x5A;
	new_task->status = OS_TASK_STAT_SYSTEM | OS_TASK_STAT_VALID;
	new_task->pid = pid;
	new_task->priority = priority;
	new_task->next = NULL;
	new_task->messages = NULL;				//no message available
	//_os_max_tick += timeout;				//calculate all tasks period
	os_assign_new_task(new_task);
	//_active_task = _highest_task;
#if OS_DEBUG_ENABLED
	new_task->dbg_index = 0;
	memset(new_task->dbg_stack, 0 , sizeof(new_task->dbg_stack));
#endif
	return new_task;
}

OSTask * os_create_system_task(uchar pid, void (* task_vector), uint16 priority, uint32 stack_size) {
	return os_create_system_task_static(pid, task_vector, priority, NULL, stack_size + 4096);
}

#if (MPU_ARCH == MPU_ARCH_16)
OSTask * os_create_task(void * context, void (* task_vector), const char * name, uint16 priority, uint16 stack_size) {	
#elif (MPU_ARCH == MPU_ARCH_32)
OSTask * os_create_task(void * context, void (* task_vector), const char * name, uint16 priority, uint32 stack_size) {
#endif	
	static uint16 g_wpid = 15;
	os_task * new_task;
	uint32 psw;
	//create new task at heap, use midgard memory manager
	psw = os_enter_critical();
	new_task = os_create_system_task(g_wpid++ , task_vector, priority, stack_size);
	if(new_task != NULL) {
		new_task->context = context;
		strncpy((char *)new_task->name, (const char *)name, OS_TASK_MAX_NAME_SIZE - 1);
		((OSThread *)new_task)->sys_stack = os_save_context_safe(((OSThread *)new_task)->sys_stack, ((OSThread *)new_task)->user_stack, (lp_void)task_vector);
	}
	os_exit_critical(psw);
	return new_task;
}


#if (MPU_ARCH == MPU_ARCH_16)
OSTask * os_create_task_static(void * context, void (* task_vector), const char * name, uint16 priority, lp_void stack, uint16 stack_size) {	
#elif (MPU_ARCH == MPU_ARCH_32)
OSTask * os_create_task_static(void * context, void (* task_vector), const char * name, uint16 priority, lp_void stack, uint32 stack_size) {
#endif	
	static uint16 g_wpid = 15;
	os_task * new_task;
	uint32 psw;
	//create new task at heap, use midgard memory manager
	psw = os_enter_critical();
	new_task = os_create_system_task_static(g_wpid++ , task_vector, priority, stack, stack_size);
	if(new_task != NULL) {
		new_task->context = context;
		strncpy((char *)new_task->name, (const char *)name, OS_TASK_MAX_NAME_SIZE - 1);
		((OSThread *)new_task)->sys_stack = os_save_context_safe(((OSThread *)new_task)->sys_stack, ((OSThread *)new_task)->user_stack, (lp_void)task_vector);
	}
	os_exit_critical(psw);
	return new_task;
}

os_task * os_find_task_by_name(const char * name) {
	os_task * task_iterator;
	task_iterator = _highest_task;
	while(task_iterator != NULL) {
		if(strcmp((char *)task_iterator->name, name) == 0) {
			return task_iterator;
		}
		//iterate next task
		task_iterator = task_iterator->next;
	}
	return NULL;
}

void os_wait(uint32 ms) {
	//tick = tick * 10;
	//((OSThread *)_active_task)->current_period = ((OSThread *)_active_task)->max_period;
	void * ptr;
	((OSThread *)_active_task)->counter_tick = (int32)ms;
	((OSThread *)_active_task)->state = OS_STATE_WAIT;
	//os_tick();
	while(((OSThread *)_active_task)->state == OS_STATE_WAIT);
}


void os_suspend(void) {
	//tick = tick * 10;
	//((OSThread *)_active_task)->current_period = ((OSThread *)_active_task)->max_period;
	((os_thread *)_active_task)->state = OS_STATE_HALT;
	//os_tick();
	while(((os_thread *)_active_task)->state == OS_STATE_HALT);
}

void os_resume(os_task * task) {
	if(task == NULL) return;
	if(((os_task *)task)->tag != 0x5A) {
		//invalid task context
		while(1);
	}
	((os_thread *)task)->state = OS_STATE_READY;
}

//select task from pcb using pid
os_task * os_find_task_by_pid(uchar pid) {
	os_task * task_iterator;
	task_iterator = _highest_task;
	while(task_iterator != NULL) {
		if(task_iterator->pid == pid) {
			return task_iterator;
		}
		//iterate next task
		task_iterator = task_iterator->next;
	}
	return NULL;
}

//get previous task on pcb
OSTask * os_get_previous_task(OSTask * task) {
	OSTask * task_iterator;
	task_iterator = _highest_task;
	while(task_iterator != NULL) {
		if(task_iterator->next == task) {
			return task_iterator;
		}
		//iterate next task
		task_iterator = task_iterator->next;
	}
	return NULL;
}

//assign new task on pcb depend on its priority
void os_assign_new_task(OSTask * task) {
	OSTask * task_iterator;
	OSTask * prev_task = NULL;
	task_iterator = _highest_task;
	while(task_iterator != NULL) {
		if((task_iterator)->priority > (task)->priority) break;
		prev_task = task_iterator;
		task_iterator = task_iterator->next;
	}
	if(prev_task != NULL) {
		prev_task->next = task;
	} else {
		_highest_task = task;
	}
	if(task_iterator != NULL) {
		task->next = task_iterator;
	} else {
		_lowest_task = task;
	}
}

void os_change_priority(OSTask * task, uint16 priority) {
	OSTask * current_task;
	OSTask * previous_task;
	//enter critical section here
	register uint32 sval;
	if(task == NULL) return;
	sval = os_enter_critical();
	current_task = task;
	previous_task = os_get_previous_task(current_task);
	if(previous_task != NULL) { 
		previous_task->next = current_task->next;
		current_task->next = NULL;
		current_task->priority = priority;
		os_assign_new_task(current_task);
	}
	//exit critical section here
	os_exit_critical(sval);
}

void os_kill_active_task(void) {
	os_kill_task(_active_task);
	_active_task = NULL;
}

//delete task on kernel time (might buggy), must also delete all messages and free logical table
void os_kill_task(os_task * task) {
	OSTask * current_task;
	OSTask * previous_task;
	OSThread * thread_iterator = NULL;
	register uint32 sval;
	//OSThread * thread_ptr;
	//enter critical section here
	sval = os_enter_critical();			//prevent any task switching here.....
	//prevent task to delete self (active task)	will cause os_tick_isr run unpredictable
	//if(task != _active_task) {	 		
	current_task = task;	//os_get_task_by_pid(pid);
	previous_task = os_get_previous_task(current_task);
	if(previous_task != NULL) {
		previous_task->next = current_task->next;
	} else {
		_highest_task = current_task->next;
	}
	//deallocate base tlb from block manager, so it can be used for new task, automatically deleting all messages
	//lbm_deallocate(current_task);			
#if RTK_USE_MIDGARD	 
	m_free((void *)((OSThread *)current_task)->stack);
	m_free(current_task);
#else
	free((void *)((OSThread *)current_task)->stack);		 
	free(current_task);
#endif
	//}
	//exit critical section here
	os_exit_critical(sval);
}

//get pid of current running task
uchar os_getpid(void) {
	return _active_task->pid;
}

os_task * os_get_active_task(void) {
	return _active_task;
}

void * os_get_context(void) {
	return _active_task->context;
}

//undefine macro
#undef malloc
#undef free
//redefine to external function
extern void * malloc(size_t size);
extern void free(void *);

extern int __user_heap_size();
int _heap_usage = 0;

void * os_alloc(size_t size) {
	uint32 psw = os_enter_critical();
	void * ptr = malloc((size + 4) & ~3);
	char * cptr = ptr;
	if(ptr != NULL) {
		_heap_usage += *((int *)(cptr - sizeof(int)));
		//memset(ptr, 0, size);
	}
	os_exit_critical(psw);
	return ptr;
}

void os_free(void * ptr) {
	char * cptr = ptr;
	uint32 psw = os_enter_critical();
	if(cptr != NULL) {
		_heap_usage -= *((int *)(cptr - sizeof(int)));
		free(cptr);
	}
	os_exit_critical(psw);
}

void * os_realloc(void * ptr, size_t size) {
	if(ptr != NULL) os_free(ptr);
	return os_alloc(size);
}

int os_heap_usage(void) {
	int usage = (_heap_usage * 100) / __user_heap_size();
	return usage;
}
