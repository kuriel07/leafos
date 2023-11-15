#ifndef _OS_SEM_H
#include "..\..\defs.h"
#include "os.h"

typedef struct os_semaphore os_semaphore;
typedef struct os_semaphore {
	os_object base;
	uint16 sid;					//semaphore id
	int16 count;				//semaphore count
	struct os_semaphore * next;
} os_semaphore;

os_semaphore * os_create_semaphore(uint16 id, int16 count);
void os_free_semaphore(os_semaphore * sem);
os_status os_wait_semaphore(os_semaphore * sem, uint16 timeout);
os_status os_signal_semaphore(os_semaphore * sem);
#define _OS_SEM_H
#endif
