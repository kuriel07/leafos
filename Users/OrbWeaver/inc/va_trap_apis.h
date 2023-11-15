/*!\file 		BSApis.h
 * \brief     	Boot-Strapper APIs
 * \details   	Boot-Strapper APIs provide all necessary function to drivers
 * \author    	AGP
 * \version   	1.0
 * \date      	Created by Agus Purwanto on 11/15/15.
 * \pre       	
 * \bug       	
 * \warning   	
 * \copyright 	Securenetix.Pte.Ltd
\verbatim
1.0
 * initial release include BSAllocator
 * based on FSTlvAllocator, modified to suit AkashicFS
 * all read/write access must use bsHandleReadW and bsHandleWriteW	(2015.12.29)
 * changed BSAbstractionHandle, added BSSystemInterface	(2015.12.29)
1.1
 * added bsPopByIndex and BS_FIND_INDEX for bsFindObject (2016.03.16)
 * changed return value of bsInitContext and bsInitAllocContext to BYTE (2016.03.20)
 * fixed init/alloc allocator when allocation space > 64K (compiler optimization) (2016.03.22)
 * changed bsInitAllocContext(BYTE, BSAllocHandle *, ptr_u, ptr_u) to bsInitAllocContext(BYTE, BSAllocHandle *, DWORD, DWORD) (2016.03.22)
 * fixed: bsFindObject (BS_FIND_TAG) returning BS_TAG_NOT_FOUND when iterating finished (2016.04.11)
 * fixed: bsMarkObject check for BSHandleAllocator limit (2016.06.28)
 * fixed: bsIteratorInit, bsFindObject (BS_FIND_EMPTY), and bsIteratorNext (2016.07.30)
 * fixed: BS_FIND_EMPTY allocate new object shift end mark and restart iterating (2017.05.04)
\endverbatim
 */

#ifndef _DEFS__H
#include "..\..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\..\config.h"
#endif
#ifndef _BSAPIS__H

#define BS_FLASH_DRIVER			0
#define BS_MAZE_DRIVER			1
#define BS_STORAGE_DRIVER		2
#define BS_RAM_DRIVER			3
#define BS_FILE_DRIVER			4  

#define BS_FIND_EMPTY			1
#define BS_FIND_TAG				2
#define BS_FIND_ALL				3
#define BS_FIND_ALLOCATED		4
#define BS_FIND_DEFRAG			5
#define BS_FIND_OFFSET 			6
#define BS_FIND_INDEX			7

#define BS_MACRO_ENABLED		0
#define BS_TAG_NOT_FOUND		(ptr_u)-1

typedef _ALIGN1_ struct BSAbstractionHandle {
	//abstraction layer	handle
	struct BSAbstractionHandle * handle;		 
	//abstraction layer
	BYTE api_id;   
	//WORD (* read)(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ ;
	//WORD (* write)(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ ;
} BSAbstractionHandle; 

typedef _ALIGN1_ struct BSAllocHandle {
	BSAbstractionHandle base ;
	//allocator parameters
	uint32 start;
	uint32 end;
} BSAllocHandle;

typedef _ALIGN1_ struct BSIterator {
 	struct BSAllocHandle base;
	BYTE tlvHdr[6];
	ptr_u current;
} BSIterator;

typedef _ALIGN1_ struct BSSystemInterface {	
	uint16 (* read)(void * handle, uint32 offset, uint8 * buffer, uint16 size) _REENTRANT_ ;
	uint16 (* write)(void * handle, uint32 offset, uint8 * buffer, uint16 size) _REENTRANT_ ;
	BSAllocHandle handle;
} BSSystemInterface;

typedef BSSystemInterface * BSSystemInterfaceP;
typedef BSAllocHandle * BSAllocHandleP;	
typedef BSAbstractionHandle * BSAbstractionHandleP;
typedef BSIterator * BSIteratorP;

extern CONST BSSystemInterface * g_aBsHandle [];		
BYTE bsErr(ptr_u result) _REENTRANT_ ;
BYTE bsNull(ptr_u result) _REENTRANT_ ;

BYTE bsInitContext(BYTE driver, BSAbstractionHandle* handle) _REENTRANT_ ;
BYTE bsInitAllocContext(BYTE driver, BSAllocHandle * handle, DWORD start, DWORD end) _REENTRANT_ ;	
BSSystemInterface * bsGetInterfaceP(void * handle) _REENTRANT_ ;
#if BS_MACRO_ENABLED
#define bsHandleWriteW(h, f, b, s) (((BSAbstractionHandle *)h)->write(((BSAbstractionHandle *)h)->handle, f, b, s))
#define bsHandleReadW(h, f, b, s) (((BSAbstractionHandle *)h)->read(((BSAbstractionHandle *)h)->handle, f, b, s))
#else
WORD bsHandleWriteW(void * handle, ptr_u offset, BYTE * buffer, WORD size) _REENTRANT_ ;
WORD bsHandleReadW(void * handle, ptr_u offset, BYTE * buffer, WORD size) _REENTRANT_ ;
#endif

BYTE bsTagLength(BYTE tag[3]) _REENTRANT_ ;
BYTE bsGetHeaderLength(BYTE * buffer, WORD * length) _REENTRANT_ ;
BYTE bsCalcHeaderLength(BYTE * buffer, WORD length) _REENTRANT_ ;
WORD bsChunkLeft(BYTE * tag, WORD actualLeft) _REENTRANT_ ;

ptr_u bsAllocObject(BSAllocHandle * handle, BYTE * tag, WORD size) _REENTRANT_ ;
ptr_u bsCommitObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size) _REENTRANT_ ;	 
ptr_u bsFindObject(BSAllocHandle * handle, BYTE mode, BYTE * tag, WORD * size) _REENTRANT_ ;
void bsRollbackObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size) _REENTRANT_ ;
void bsReleaseObjectList(BSAllocHandle * handle, BYTE * start, BYTE * end) _REENTRANT_ ;
void bsReleaseByOffset(BSAllocHandle * handle, ptr_u offset) _REENTRANT_ ;
void bsReleaseByDataOffset(BSAllocHandle * handle, ptr_u dataOffset) _REENTRANT_ ;
void bsReleaseObject(BSAllocHandle * handle, BYTE * tag) _REENTRANT_ ;

WORD bsPopByTag(BSAllocHandle * handle, BYTE * tag, WORD size, BYTE * buffer) _REENTRANT_ ;
WORD bsPopByIndex(BSAllocHandle * handle, WORD index, WORD size, BYTE * buffer) _REENTRANT_ ;
BYTE bsPopHeader(BSAllocHandle * handle, WORD offset, BYTE * tag, WORD * size) _REENTRANT_ ;

WORD bsIteratorInit(BSIterator * iterator, BSAllocHandle * handle) _REENTRANT_ ;
WORD bsIteratorNext(BSIterator * iterator) _REENTRANT_ ;
WORD bsIteratorRead(BSIterator * iterator, ptr_u offset, BYTE * buffer, WORD size) _REENTRANT_ ;

#if 0
ptr_u bsResizeObject(BSAllocHandle * handle, ptr_u offset, WORD size) _REENTRANT_ ;
#endif

#define _BSAPIS__H
#endif
