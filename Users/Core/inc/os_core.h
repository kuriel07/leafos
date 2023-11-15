/*
 * kernel.h
 * this is machine independent code for os engine
 *  Created on: Nov 27, 2010
 *      Author: Agus Purwanto
 */
 //added : os_debug APIs (2017.06.21)

#ifndef _MINOS_CORE_H_
#include "..\..\defs.h"
#include "os_config.h"

#define OS_DEBUG_ENABLED				1
#define OS_DEBUG_STACK_SIZE			24

#define OS_TASK_STAT_INVALID		0x01
#define OS_TASK_STAT_VALID			0x00
#define OS_TASK_STAT_SYSTEM			0x80
#define OS_ASSERT_NULL(x) (x == NULL)

typedef struct os_thread OSThread;
typedef struct os_thread os_thread;
struct os_thread {				 		//total size 24 bytes
	lp_void sys_stack;					//stack pointer address (logical) 
	lp_void	vector;  					//start vector address/current vector address (used by ARM arch) 
	lp_void stack;						//stack base (for sys_stack)
	lp_void user_stack;					
	uint32 stack_size;
	//uint16 priority;				//task priority on preemption active
	uint16 tid; 			   		//thread identifier
	uint16 state;					//current task state (DORMANT, READY, WAIT, RUNNING) 
	uint16 utilization;
	uint8 stack_util;
#if (MPU_ARCH == MPU_ARCH_16)
	int32 counter_tick;
#elif (MPU_ARCH == MPU_ARCH_32)
	int32 counter_tick;
#endif
	struct os_thread * next;			//pointer to next thread
};

typedef struct os_task OSTask;
typedef struct os_task os_task;			//future naming convention
#define OS_TASK_MAX_NAME_SIZE		32
struct os_task
{  
	//thread properties
	os_thread base_class; 					//base class
	lp_void base_tlb;					//base mmu translation look aside buffer
	lp_void heap; 						//heap base
	//end of thread properties	   
	uchar tag;						//always 0x5A
	uchar status;						//task status
	uint16 pid;						//current task id (PID = Process Identifier)
	uint16 priority;				//task priority on preemption active
	void * context;
	//uint16 num_thread;			 
	//struct os_thread * base;			//pointer to base thread
	//struct os_thread * current;		//pointer to current running thread on this task
	struct os_task * next;
	struct os_message * messages;
	uchar name[OS_TASK_MAX_NAME_SIZE];
#if OS_DEBUG_ENABLED
	uint8 dbg_index;
	uint32 dbg_stack[OS_DEBUG_STACK_SIZE];
#endif
};

typedef struct os_task_info os_task_info;
_ALIGN1_ struct os_task_info {
	uint8 pid;
	uint16 cpu_util;		//cpu utilization
	uint8 stack_util;	//stack utilization
	uint8 msg_count;	//message count
	uint8 str_len;
};

/*******************************************************************************************/
/* void os_init();                                                                        */
/* parameter : (void), return : (void)                                                     */
/*******************************************************************************************/
void os_init(void * ctx);
/*******************************************************************************************/
/* void os_force_switch(pid);                                                             */
/* parameter : pid, return : (void)                                                        */
/*******************************************************************************************/
void os_force_switch(uchar pid);
#if (MPU_ARCH == MPU_ARCH_16)
/*******************************************************************************************/
/* void _os_create_task(task_vector, priority, stack_size, timeout);                       */
/* parameter : task_vector, priority, stack_size, timeout, return : (void)                 */
/*******************************************************************************************/
os_task * os_create_task(void * context, void (* task_vector), const char * name, uint16 priority, uint16 stack_size, uint16 timeout);
/*******************************************************************************************/
/* uint16 os_tick_isr(sp);                                                                */
/* parameter : stack pointer, return : stack pointer                                       */
/*******************************************************************************************/
os_task * os_tick_isr(uint16 sp, uint16 lp);
/*******************************************************************************************/
/* uint32 os_wait(tick);                                                                  */
/* parameter : tick to wait, return : (void)                                               */
/*******************************************************************************************/
void os_wait(uint16 tick);
#elif (MPU_ARCH == MPU_ARCH_32)
/*******************************************************************************************/
/* void os_create_task(task_vector, priority, stack_size, timeout);                       */
/* parameter : task_vector, priority, stack_size, timeout, return : (void)                 */
/*******************************************************************************************/
os_task * os_create_task(void * context, void (* task_vector), const char * name, uint16 priority, uint32 stack_size);
OSTask * os_create_task_static(void * context, void (* task_vector), const char * name, uint16 priority, lp_void stack, uint32 stack_size) ;
/*******************************************************************************************/
/* uint32 os_tick_isr(sp);                                                                */
/* parameter : stack pointer, return : stack pointer                                       */
/*******************************************************************************************/
os_task * os_tick_isr(uint32 sp, uint32 lp);
/*******************************************************************************************/
/* uint32 os_wait(tick);                                                                  */
/* parameter : tick to wait, return : (void)                                               */
/*******************************************************************************************/
void os_wait(uint32 tick);
#endif

uchar os_getpid(void);
uint16 os_get_cpu_util() ;
os_task * os_get_active_task(void);
void * os_get_context(void);
void os_assign_new_task(OSTask * task);
void os_assign_new_thread(OSThread * thread);
void os_switch_thread(OSTask * task);
void os_change_priority(OSTask * task, uint16 priority);
void os_kill_task(os_task * task);
void os_force_switch(uchar pid);
void os_suspend(void);
void os_resume(os_task * task);
os_task * os_find_task_by_name(const char * name) ;
os_task * os_find_task_by_pid(uchar pid);
#define os_align_address(x)		(void *)(((ptr_u)x + 4) & ~(4-1))

os_task * os_create_user_task(uchar pid, uint32 stack_size, uint32 timeout, uchar * usercode, uint32 codesize);
os_thread * os_create_thread(void (* task_vector), uint16 priority, uint32 stack_size);


/*******************************************************************************************/
/* uint32 os_task_alloc(task, size);                                                       */
/* parameter : task, size to allocate, return : pointer to allocated memory                */
/*******************************************************************************************/
void * os_task_alloc(os_task * task, uint32 size);


/*******************************************************************************************/
/* uint32 os_task_free(task, size);                                                        */
/* parameter : task, size to allocate, return : none                                       */
/*******************************************************************************************/
void os_task_free(os_task * task, void * ptr);

/*******************************************************************************************/
/* uint32 os_alloc(size);                                                                  */
/* parameter : size to allocate, return : pointer to allocated memory                      */
/*******************************************************************************************/
void * os_alloc(uint32 size);

/*******************************************************************************************/
/* uint32 os_free(ptr);                                                                    */
/* parameter : pointer to allocated memory, return : (void)                                */
/*******************************************************************************************/
void os_free(void *ptr);
/*******************************************************************************************/
/* uint32 os_heap_start(ptr);                                                              */
/* parameter : pointer to start of heap, return : heap address                             */
/*******************************************************************************************/
void * os_heap_start(void);	
/*******************************************************************************************/
/* uint32 os_heap_usage();                                                              */
/* return : heap usage                        										     */
/*******************************************************************************************/
int os_heap_usage(void) ;

void os_load_application(uchar * path, void * fs);
void os_switch_virtual_address(os_task * task);
void os_kill_active_task(void);	
void os_switch_cpu_clock(uint8 mode);

#if OS_DEBUG_ENABLED
uint32 os_debug_entry_w(uint32 func, char * funcname, uint32 line, char * module);
void os_debug_exit_w(uint32 dbg);

#define OS_DEBUG_ENTRY(f)		uint32 dbg = os_debug_entry_w((uint32)f, TEXT(f), __LINE__, __FILE__)
#define OS_DEBUG_EXIT()				os_debug_exit_w(dbg)
#else
#define OS_DEBUG_ENTRY(f)
#define OS_DEBUG_EXIT()
#endif

/* internal task states */
#define OS_STATE_DORMANT			0x01
#define OS_STATE_READY			0x02
#define OS_STATE_RUNNING			0x04
#define OS_STATE_WAIT			0x08
#define OS_STATE_IDLE			0x10
#define OS_STATE_HALT			0x80

#define _MINOS_CORE_H_
#endif /* KERNEL_H_ */
