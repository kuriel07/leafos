#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include <string.h>
//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.kui0101a/armlink_chdcgbjd.htm
/*
Image$$region_name$$Base	Execution address of the region.
Image$$region_name$$Length	Execution region length in bytes excluding ZI length.
Image$$region_name$$Limit	Address of the byte beyond the end of the non-ZI part of the execution region.
Image$$region_name$$RO$$Base	Execution address of the RO output section in this region.
Image$$region_name$$RO$$Length	Length of the RO output section in bytes.
Image$$region_name$$RO$$Limit	Address of the byte beyond the end of the RO output section in the execution region.
Image$$region_name$$RW$$Base	Execution address of the RW output section in this region.
Image$$region_name$$RW$$Length	Length of the RW output section in bytes.
Image$$region_name$$RW$$Limit	Address of the byte beyond the end of the RW output section in the execution region.
Image$$region_name$$ZI$$Base	Execution address of the ZI output section in this region.
Image$$region_name$$ZI$$Length	Length of the ZI output section in bytes.
Image$$region_name$$ZI$$Limit	Address of the byte beyond the end of the ZI output section in the execution region.
*/

extern char Image$$ER_IROM1$$RO$$Base[];
extern uint32_t Image$$RW_IRAM1$$Length;
extern uint32_t Image$$RW_IRAM1$$RW$$Length;
extern uint32_t Image$$LR_IROM1$$Limit;
extern uint32_t Load$$LR$$LR_IROM1$$Base; // Address of the load region.
extern uint32_t Load$$LR$$LR_IROM1$$Length; // Length of the load region in bytes.
extern uint32_t Load$$ER_RO$$Limit; // Address of the byte beyond the end of the load region.
extern uint32_t Load$$ER_IROM1$$RO$$Limit;
extern uint32_t Load$$ER_IROM1$$Limit;
extern uint32_t Image$$ER_IROM1$$Limit;
extern uint32_t Image$$RW_IRAM1$$Length;
extern char Image$$ER_IROM2$$RO$$Base[];

//extern uint32 _conf_base;
//extern unsigned Image$$ER_IRAM1$$RW$$Size;
//extern unsigned Image$$ER_IROM1$$ZI$$Limit;
//uint8 * gba_base_flash;// = (char *) ((Image$$RO$$Limit + 0x200) & (0 - 0x200));		//bounded array

void if_flash_init() {
	/*FLASH_PrefetchBufferCmd(ENABLE);
	FLASH_InstructionCacheCmd(ENABLE);
	FLASH_DataCacheCmd(ENABLE);
    FLASH_InstructionCacheReset();
    FLASH_DataCacheReset();*/
}

uint16 if_flash_data_write(void * handle, uint32 offset, uint8 * buffer, uint16 length) {
	uint16 wrote = 0;
	FLASH_EraseInitTypeDef FLASH_EraseInitStructure;
	uint32 sector_error;
	uint16 i;
	uint8 ret = -1;
	//uint8 temp[IF_FLASH_SECTORSIZE];
	uint8 * temp = NULL;
	uint16 * ptr;
	uint16 fset;
	uint16 len;
	uint32 psw;
	uint8 * base_addr;
	if(offset > IF_FLASH_SECTORSIZE) return ret;
	if((offset + length) > IF_FLASH_SECTORSIZE) length = IF_FLASH_SECTORSIZE - offset;
	uint8 * cur_address = (uint8 *)0x8018000 + offset;
	psw = os_enter_critical();
	HAL_FLASH_Unlock();
	temp = os_alloc(IF_FLASH_SECTORSIZE);
	if(temp == NULL) goto finish_write;
#ifdef STM32F7
	cur_address = (uint8 *)0x8018000 + offset;
	memcpy(temp, (uint8 *)0x8018000, IF_FLASH_SECTORSIZE);
	base_addr = (uint8 *)0x8018000;
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
#endif
#ifdef STM32F4
	cur_address = (uint8 *)0x8004000 + offset;
	memcpy(temp, (uint8 *)0x8004000, IF_FLASH_SECTORSIZE);
	base_addr = (uint8 *)0x8004000;
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	FLASH_EraseInitStructure.Banks = FLASH_BANK_1;
#endif
	if(length != 0) {
		ptr = (uint16 *)temp;
#ifdef STM32F7
		FLASH_EraseInitStructure.Sector = FLASH_SECTOR_3;
#endif
#ifdef STM32F4
		FLASH_EraseInitStructure.Sector = FLASH_SECTOR_1;
#endif
		FLASH_EraseInitStructure.TypeErase = FLASH_TYPEERASE_SECTORS;
		FLASH_EraseInitStructure.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		FLASH_EraseInitStructure.NbSectors = 1;
		HAL_FLASHEx_Erase(&FLASH_EraseInitStructure, &sector_error);
		if(sector_error != 0xFFFFFFFFU) goto finish_write;
		memcpy(temp + offset, buffer, length);
		for(i=0;i<(IF_FLASH_SECTORSIZE>>1);i++) {
			//FLASH_ProgramHalfWord((uint32_t)base_addr + (i << 1), ptr[i]);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)base_addr + (i << 1), ptr[i]);
		}
		wrote += len;
	}
	ret = wrote;
	finish_write:
	if(temp != NULL) os_free(temp);
	HAL_FLASH_Lock();
	os_exit_critical(psw);
	return ret;
}

uint16 if_flash_data_read(void * handle, uint32 offset, uint8 * buffer, uint16 length) {
	if(offset > 0x4000) return 0;
	if((offset + length) > 0x4000) length = 0x4000 - offset;
#ifdef STM32F7
	memcpy(buffer, (uint8 *)0x8018000 + offset, length);
#endif
#ifdef STM32F4
	memcpy(buffer, (uint8 *)0x8004000 + offset, length);
#endif
	return length;
}

uint16 if_flash_code_read(void * handle, uint32 offset, uint8 * buffer, uint16 length) {
	if(offset > 0x4000) return 0;
	if((offset + length) > 0x4000) length = 0x4000 - offset;
	memcpy(buffer, (uint8 *)0x8008000 + offset, length);
	return length;
}

void if_flash_code_erase() {
	uint16 wrote = 0;
	FLASH_EraseInitTypeDef FLASH_EraseInitStructure;
	uint32 sector_error;
	uint16 i;
	uint16 fset;
	uint16 len;
	uint8 * base_addr;
	uint8 * cur_address = (uint8 *)0x8008000;
	///memcpy(temp, (uint8 *)0x8008000, IF_FLASH_SECTORSIZE);
	base_addr = (uint8 *)0x8008000;
	HAL_FLASH_Unlock();
#ifdef STM32F4
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	FLASH_EraseInitStructure.Banks = FLASH_BANK_1;
	FLASH_EraseInitStructure.Sector = FLASH_SECTOR_2;
#endif
#ifdef STM32F7
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	FLASH_EraseInitStructure.Sector = FLASH_SECTOR_1;
#endif
	FLASH_EraseInitStructure.TypeErase = FLASH_TYPEERASE_SECTORS;
	FLASH_EraseInitStructure.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	FLASH_EraseInitStructure.NbSectors = 1;
	HAL_FLASHEx_Erase(&FLASH_EraseInitStructure, &sector_error);
	finish_write:
	HAL_FLASH_Lock();
}

uint16 if_flash_code_write(void * handle, uint32 offset, uint8 * buffer, uint16 length) {
	uint16 wrote = 0;
	FLASH_EraseInitTypeDef FLASH_EraseInitStructure;
	uint32 sector_error;
	uint16 i;
	uint8 ret = -1;
	//uint8 temp[IF_FLASH_SECTORSIZE];
	uint8 * temp = NULL;
	uint16 * ptr;
	uint16 fset;
	uint16 len;
	uint32 psw;
	uint8 * base_addr;
	if(offset > 0x4000) return ret;
	if((offset + length) > 0x4000) length = 0x4000 - offset;
	uint8 * cur_address = (uint8 *)0x8008000 + offset;
	psw = os_enter_critical();
	HAL_FLASH_Unlock();
	temp = os_alloc(IF_FLASH_SECTORSIZE);
	if(temp == NULL) goto finish_write;
	memcpy(temp, (uint8 *)0x8008000, IF_FLASH_SECTORSIZE);
	base_addr = (uint8 *)0x8008000;
#ifdef STM32F4
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
#endif
#ifdef STM32F7
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
#endif
	if(length != 0) {
		ptr = (uint16 *)temp;
#ifdef STM32F4
		FLASH_EraseInitStructure.Sector = FLASH_SECTOR_2;
#endif
#ifdef STM32F7
		FLASH_EraseInitStructure.Sector = FLASH_SECTOR_1;
#endif
		FLASH_EraseInitStructure.TypeErase = FLASH_TYPEERASE_SECTORS;
		FLASH_EraseInitStructure.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		//FLASH_EraseInitStructure.Banks = FLASH_BANK_1;
		FLASH_EraseInitStructure.NbSectors = 1;
		HAL_FLASHEx_Erase(&FLASH_EraseInitStructure, &sector_error);
		if(sector_error != 0xFFFFFFFFU) goto finish_write;
		memcpy(temp + offset, buffer, length);
		for(i=0;i<(IF_FLASH_SECTORSIZE>>1);i++) {
			//FLASH_ProgramHalfWord((uint32_t)base_addr + (i << 1), ptr[i]);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)base_addr + (i << 1), ptr[i]);
		}
		wrote += len;
	}
	ret = length;
	finish_write:
	if(temp != NULL) os_free(temp);
	HAL_FLASH_Lock();
	os_exit_critical(psw);
	return ret;
}

#define ID_DBGMCU_IDCODE		0xE0042000
#define if_flash_get_rev()		(*(uint16_t *) (DBGMCU->IDCODE + 2))
#define if_flash_get_signature()	((*(uint16_t *) (ID_DBGMCU_IDCODE)) & 0x0FFF)

uint8 if_flash_get_esid(uint8 * buffer, uint8 length) {
	uint8 i;
	uint8 stm_buf[16];
	uint8 * ptr = (uint8 *)0x1FFF7A10;
	uint8 size = (length > 16)?16:length;
	uint16 signature = if_flash_get_signature();
	stm_buf[0] = 0xE0;		//etheron
	stm_buf[1] = 0x00;		//os specific
	stm_buf[2] = ((signature >> 8) & 0x0F);				//SHARD_OWL_SUPPORTED;	
	stm_buf[3] = signature & 0xFF;			//STM32F7
#ifdef STM32F7	
	stm_buf[2] |= 0x70;
	ptr = (uint8 *)0x1FF0F420;	
#endif
#ifdef STM32F4
	stm_buf[2] |= 0x40;
	ptr = (uint8 *)0x1FFF7A10;
#endif
#ifdef STM32F2
	stm_buf[2] |= 0x20;
	ptr = (uint8 *)0x1FFF7A10;
#endif
#ifdef STM32F1
	stm_buf[2] |= 0x10;
	ptr = (uint8 *)0x1FFFF7E8;
#endif
#ifdef STM32F3
	stm_buf[2] |= 0x30;
	ptr = (uint8 *)0x1FFFF7AC;
#endif
#ifdef STM32F0
	stm_buf[2] |= 0x00;
	ptr = (uint8 *)0x1FFFF7AC;
#endif
	for(i=12;i>0;i--) stm_buf[4 + (i-1)] = *ptr++;
	memcpy(buffer, stm_buf, size);
	return size;
}