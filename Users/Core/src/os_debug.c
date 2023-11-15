#include "..\..\defs.h"
#include "..\inc\os.h"
#include "..\inc\os_core.h"
#include "..\inc\os_config.h"
#include "..\inc\os_mach.h"
#include "..\inc\os_msg.h"
#include <string.h>
#include "..\..\toolkit\inc\tk_apis.h"

#if OS_DEBUG_ENABLED

extern OSTask * _active_task;

uint32 os_debug_entry_w(uint32 func, char * funcname, uint32 line, char * module) {
	uint32 csr;
	if(_active_task == NULL) return -1;												//no task currently running
	if(_active_task->dbg_index >= OS_DEBUG_STACK_SIZE ) return -1;		//already at stack limit
	csr = os_enter_critical();
	_active_task->dbg_stack[_active_task->dbg_index++] = (uint32)funcname;
	os_exit_critical(csr);
	return (uint32)funcname;
}

void os_debug_exit_w(uint32 dbg) {
	uint32 csr;
	if(dbg == (uint32)-1) return ;													// invalid dbg pointer
	if(_active_task == NULL) return;												//no task currently running
	if(_active_task->dbg_index == 0) return;										//prevent stack underflow
	csr = os_enter_critical();
	_active_task->dbg_stack[--_active_task->dbg_index] = 0;					//clear stack
	os_exit_critical(csr);
}

#endif