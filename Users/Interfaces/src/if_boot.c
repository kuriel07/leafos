#include <stdio.h>
//#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"

extern FLASH_ProcessTypeDef pFlash;
#ifndef FLASH_SECTOR_8
#define FLASH_SECTOR_8     ((uint32_t)8U)  /*!< Sector Number 8   */
#endif

HAL_StatusTypeDef if_boot_erase(FLASH_EraseInitTypeDef *pEraseInit)
{
	  HAL_StatusTypeDef status = HAL_ERROR;
	  uint32_t index = 0U;
		/* Process Locked */
		__HAL_LOCK(&pFlash);
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) ;
		/* Check FLASH End of Operation flag  */
		if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
		{
			/* Clear FLASH End of Operation pending bit */
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
		}
#ifdef STM32F407xx
	  if(__HAL_FLASH_GET_FLAG((FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
							   FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR)) != RESET)
#endif
#ifdef STM32F746xx
	  if(__HAL_FLASH_GET_FLAG((FLASH_FLAG_ALL_ERRORS)) != RESET)
#endif
	  {
		/*Save the error code*/
		//FLASH_SetErrorCode();
		return HAL_ERROR;
	  }
      /* Check the parameters */
      //assert_param(IS_FLASH_NBSECTORS(pEraseInit->NbSectors + pEraseInit->Sector));

	  /* Erase by sector by sector to be done*/
	  for(index = pEraseInit->Sector; index < (pEraseInit->NbSectors + pEraseInit->Sector); index++)
	  {
			/* If the previous operation is completed, proceed to erase the sector */
			CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
			FLASH->CR |= FLASH_PSIZE_BYTE;
			CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
			FLASH->CR |= FLASH_CR_SER | (index << POSITION_VAL(FLASH_CR_SNB));
			FLASH->CR |= FLASH_CR_STRT;
			/* Wait for last operation to be completed */
			//status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
			while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) ;
			/* Check FLASH End of Operation flag  */
			if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
			{
				/* Clear FLASH End of Operation pending bit */
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
			}
#ifdef STM32F407xx
		  if(__HAL_FLASH_GET_FLAG((FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
								   FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR)) != RESET)
#endif
#ifdef STM32F746xx
		if(__HAL_FLASH_GET_FLAG((FLASH_FLAG_ALL_ERRORS)) != RESET)
#endif
		  {
			/*Save the error code*/
			//FLASH_SetErrorCode();
			return HAL_ERROR;
		  }
			/* If the erase operation is completed, disable the SER and SNB Bits */
			CLEAR_BIT(FLASH->CR, (FLASH_CR_SER | FLASH_CR_SNB));
	  }
	  /* Process Unlocked */
	  __HAL_UNLOCK(&pFlash);
}

int main(void) {
	extern void _Reset_Handler();
	void (* load)(void) = (void (*)(void))_Reset_Handler;
	load();
}

//void __svc(1) if_load_new_firmware() {
void __SVC_1 () {          
	uint8 * ptr;
	uint32 i;
	bfd_info * b_info = (bfd_info *)0x08080000;
	FLASH_EraseInitTypeDef FLASH_EraseInitStructure;
	uint32 sector_error;
	uint32 base_addr;
	
	if((FLASH->CR & FLASH_CR_LOCK) == SET) return;
	if(b_info->status == 0xFF) return;
	/* Authorize the FLASH Registers access */
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	
	//erase application sectors
	FLASH_EraseInitStructure.Sector = FLASH_SECTOR_4;
	FLASH_EraseInitStructure.TypeErase = FLASH_TYPEERASE_SECTORS;
	FLASH_EraseInitStructure.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	//FLASH_EraseInitStructure.Banks = FLASH_BANK_1;
	FLASH_EraseInitStructure.NbSectors = 4;
	if_boot_erase(&FLASH_EraseInitStructure);
	//set destination and source pointer
	base_addr = 0x08010000;
	ptr = (uint8 *)0x08080000 + sizeof(bfd_info);
	pFlash.Lock = HAL_UNLOCKED;
	//start burst data for memcpy
	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_PSIZE_BYTE;
	FLASH->CR |= FLASH_CR_PG;
	for(i=0x10000;i < b_info->len;i++) {
		*(__IO uint8_t*)base_addr++ = ptr[i];
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) ;
		/* Check FLASH End of Operation flag  */
		if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
		{
			/* Clear FLASH End of Operation pending bit */
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
		}
	} 
	base_addr = 0x08010000;
	for(i=0x10000;i < b_info->len;i++) {
		if(*(__IO uint8_t*)base_addr++ != ptr[i]) {
				while(1);			//byte failed
		}
	} 
	/* If the program operation is completed, disable the PG Bit */
	FLASH->CR &= (~FLASH_CR_PG);  
	
	//clear downloaded firmware data
	FLASH_EraseInitStructure.Sector = FLASH_SECTOR_8;
	FLASH_EraseInitStructure.TypeErase = FLASH_TYPEERASE_SECTORS;
	FLASH_EraseInitStructure.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	//FLASH_EraseInitStructure.Banks = FLASH_BANK_1;
	FLASH_EraseInitStructure.NbSectors = 4;
	if_boot_erase(&FLASH_EraseInitStructure);
	
	//lock flash
	FLASH->CR |= FLASH_CR_LOCK;
	pFlash.Lock = HAL_LOCKED;
	//b_info.status = 0x01;
	//b_info.len = content_length;
	//memcpy(b_info.signature, sign, IF_KRONOS_SIGNATURE_LEN);
	
	//kron_load();
	//while(1);
	NVIC_SystemReset();
	while(1);
}


/**
* @brief This function handles System service call via SWI instruction.
*/

__asm void SVC_Handler (void) {
		PRESERVE8

        IMPORT  SVC_Count
        IMPORT  SVC_Table

        MRS     R0,MSP                 // ; Read PSP
        LDR     R1,[R0,#24]            // ; Read Saved PC from Stack
        SUBS    R1,R1,#2                //; Point to SVC Instruction
        LDRB    R1,[R1]                 //; Load SVC Number
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */
SVC_User
        PUSH    {R4,LR}                 //; Save Registers
        LDR     R2,=SVC_Count
        LDR     R2,[R2]
        CMP     R1,R2
        BHI     SVC_Done                //; Overflow

        LDR     R4,=SVC_Table-4
        LSLS    R1,R1,#2
        LDR     R4,[R4,R1]              //; Load SVC Function Address
        MOV     LR,R4

        LDMIA   R0,{R0-R3,R4}           //; Read R0-R3,R12 from stack
        MOV     R12,R4
        BLX     LR                      //; Call SVC Function

        MRS     R4,PSP                  //; Read PSP
        STMIA   R4!,{R0-R3}            // ; Function return values
SVC_Done
        POP     {R4,PC}                 //; RETI
  /* USER CODE END SVCall_IRQn 1 */
		ALIGN
}