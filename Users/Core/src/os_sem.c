#include "..\..\defs.h"
#include "..\inc\os.h"
#include "..\inc\os_sem.h"

extern os_task * _active_task;
os_semaphore * os_task_create_semaphore(os_task * task, uint16 id, int16 count) {
	
	os_semaphore * sem = (os_semaphore *)os_task_alloc(task, sizeof(os_semaphore));
	
	os_init_object((os_object *)sem, OS_OBJECT_TYPE_SEMAPHORE, 0, sizeof(os_semaphore));
	sem->sid = id;
	sem->count = count;
	sem->next = NULL;
	return sem;
}

os_semaphore * os_create_semaphore(uint16 id, int16 count) {
	return os_task_create_semaphore(_active_task, id, count);
}

void os_free_semaphore(os_semaphore * sem) {
	os_task_free(_active_task, sem);
} 

os_status os_wait_semaphore(os_semaphore * sem, uint16 timeout) {
	if(sem == NULL) return OS_NO_ERR;
	if(sem->count < 0) {
		os_wait_object((os_object *)sem, timeout);
	}
	if(sem->count < 0) {
		return OS_SEMAPHORE_BUSY;
	}
	sem->count --;
	return OS_NO_ERR;	
}

os_status os_signal_semaphore(os_semaphore * sem) {
	sem->count ++;
	os_signal_object((os_object *)sem);
	return OS_NO_ERR;
}