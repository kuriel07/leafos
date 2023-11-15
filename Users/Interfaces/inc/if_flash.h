#ifndef IF_FLASH__H
#define IF_FLASH__H

#include "..\..\defs.h"
#include "..\..\config.h"

#define IF_FLASH_PAGESIZE		0x200
#define IF_FLASH_SECTORSIZE	0x4000

void if_flash_init(void);
//configuration area (kron only) APIs
uint16 if_flash_data_write(void * handle, uint32 offset, uint8 * buffer, uint16 length);
uint16 if_flash_data_read(void * handle, uint32 offset, uint8 * buffer, uint16 length);

//code area (for device entity) APIs
uint16 if_flash_code_write(void * handle, uint32 offset, uint8 * data, uint16 length);
uint16 if_flash_code_read(void * handle, uint32 offset, uint8 * data, uint16 length);
void if_flash_code_erase(void);
uint8 if_flash_get_esid(uint8 * buffer, uint8 length);

#endif
