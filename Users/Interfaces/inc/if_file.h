#ifndef IF_FILE__H
#include "defs.h"
#include "config.h"


typedef struct if_file {
	void * handle;
	uint32 offset;
	uint32 size;
	uint32 (* read)(struct if_file *, uint32 offset, uint8 * data, uint32 size);
	uint32 (* write)(struct if_file *, uint32 offset, uint8 * data, uint32 size);
	void (* close)(struct if_file *);
} if_file;

#define IF_FILE__H
#endif
