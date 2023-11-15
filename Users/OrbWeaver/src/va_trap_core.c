#include "..\..\defs.h"
#include "..\..\config.h"
#include <string.h>
#ifndef _BSAPIS__H	
#include "..\inc\va_trap_apis.h"
#endif
#include "..\..\interfaces\inc\if_apis.h"

CONST BSSystemInterface g_bsFmDriver = { 
	if_flash_code_read, if_flash_code_write, { { NULL, 0 }, 0, 0x8000 }
};					  //kernel

CONST BSSystemInterface g_bsFdDriver = { 
	if_flash_data_read, if_flash_data_write, { { NULL, 0 }, 0, 0x4000 }
};					  //kernel

CONST BSSystemInterface * g_aBsHandle [] = {
	&g_bsFmDriver, 			//kernel flash code driver
	&g_bsFdDriver				//kernel flash data driver
};
		
BSSystemInterface * bsGetInterfaceP(void * handle) _REENTRANT_ {
	return (BSSystemInterface *)g_aBsHandle[((BSAbstractionHandle *)handle)->api_id];
}

/*!
******************************************************************************
\section  BYTE bsInitContext(BYTE driver, BSAbstractionHandle * handle)
\brief    initialize an BSAbstractionHandle for bootstrapping purpose, automatically called by bsInitAllocContext
\param 	  driver a driver identifier BS_FLASH_DRIVER, BS_MAZE_DRIVER, BS_STORAGE_DRIVER, BS_RAM_DRIVER, BS_FILE_DRIVER (IN) 
\param	  handle handle to specified BSAbstractionHandle (OUT)
\return   zero on success, -1 on failed
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

initialize an BSAbstractionHandle for bootstrapping purpose, automatically called by bsInitAllocContext

\endverbatim
******************************************************************************
*/
BYTE bsInitContext(BYTE driver, BSAbstractionHandle * handle) _REENTRANT_ {
	//mmMemCpy((void *)handle, (void *)&g_aBsHandle[driver]->handle, sizeof(BSAbstractionHandle));
	if(g_aBsHandle[driver] == NULL) return -1;
	handle->handle = ((BSAbstractionHandle *)&(((BSSystemInterface *)g_aBsHandle[driver])->handle))->handle;
	handle->api_id = driver;
	return 0;
} 

/*!
******************************************************************************
\section  BYTE bsInitAllocContext(BYTE driver, BSAllocHandle * handle, ptr_u start, ptr_u end)
\brief    initialize an BSAllocHandle for bootstrapping purpose
\param 	  driver a driver identifier BS_FLASH_DRIVER, BS_MAZE_DRIVER, BS_STORAGE_DRIVER, BS_RAM_DRIVER, BS_FILE_DRIVER (IN) 
\param	  handle handle to specified BSAllocHandle (OUT)
\param	  start start address of allocation (IN)
\param	  end end address of allocation	(IN)
\return   zero on success, -1 on failed
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

initialize an BSAllocHandle for bootstrapping purpose

\endverbatim
******************************************************************************
*/
BYTE bsInitAllocContext(BYTE driver, BSAllocHandle * handle, DWORD start, DWORD end) _REENTRANT_ {
	BYTE ret;
 	if((ret = bsInitContext(driver, &handle->base)) == 0) {
		handle->start = (ptr_u)start;
		handle->end = (ptr_u)end;
	}
	return ret;
} 
	  	
#if !BS_MACRO_ENABLED
uint16 bsHandleWriteW(void * handle, ptr_u offset, uint8 * buffer, WORD size) _REENTRANT_ {
  	//return ((BSAbstractionHandle *)handle)->write(((BSAbstractionHandle *)handle)->handle, offset, buffer, size);
	uint8 api = ((BSAbstractionHandle *)handle)->api_id;
	return (g_aBsHandle[api])->write(((BSAbstractionHandle *)handle)->handle, offset, buffer, size);
}

uint16 bsHandleReadW(void * handle, ptr_u offset, uint8 * buffer, WORD size) _REENTRANT_ {
  	//return ((BSAbstractionHandle *)handle)->read(((BSAbstractionHandle *)handle)->handle, offset, buffer, size); 		 
	uint8 api = ((BSAbstractionHandle *)handle)->api_id;
	return (g_aBsHandle[api])->read(((BSAbstractionHandle *)handle)->handle, offset, buffer, size);
}
#endif


