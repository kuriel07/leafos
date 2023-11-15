/*!\file 		MMApis.h
 * \brief     	Memory Management APIs
 * \details   	Memory Management APIs provide all necessary function to access memory such as object allocator and entry allocator
 * \author    	AGP
 * \version   	2.1
 * \date      	Created by Agus Purwanto on 3/12/14.
 * \pre       	must call mmInit() and mmMazeInit() before it can be used
 * \bug       	MMID only support up to 2GB addressing for both heap (heap0+heap1)
 * \warning   	object read/write operation should not exceed 2x logical page size
 * \copyright 	Securenetix.Pte.Ltd
\verbatim
1.0
 * initial release mmCreateObject, mmDeleteObject, mmReadObject, mmWriteObject
1.1 
 * added dual partition heap 0 (dynamic) for file system and heap 1 (static) for user application
 * asynchronous support with mmBeginTransaction, mmCommitTransaction, mmRollbackTransaction, mmEndTransaction (blocking thread, partition 0)
 * changed mmGetFreeMemDW to support dual heap
1.2
 * bug fixed: entry tag checking (mmReadEntryW)
 * bug fixed: mmGetFreeMemDW(type) for heap 0
2.0
 * modified version 1.1, merged partition 0 and 1 into single heap (heap0) provide less configuration (MMFlashAllocator, MMFlashMaze)
 * asynchronous support both partition 0 and 1
 * added: mmMazeRelocateSectorW to relocate a sector to new empty sector during entry allocation
 * bug fixed: mmAllocEntryW to heap 0 (rotation) overlapped with maze sector (see mmMazeRelocateSectorW)
 * bug fixed: asynchronous commit-begin caused by wrong pointer usage &mmAsync->table to mmAsync->table
 * bug fixed: mmEndOfEntriesDW always return 0x00000000 causing mmCreateObjectW returning NULL
2.1
 * bug fixed: mmFreeMem, shift end of heap if last chunk deleted, causing deadlock if combined with shiftchunk (added 2015.04.08)
 * bug fixed: abnormal termination and un-popped stack data memory leakage (added 2015.04.09)
 * bug fixed: mmAllocTransient changed size parameter type from BYTE to WORD when allocating data with length 0xFF (added 2015.05.04)
 * added : mmSprintf as replacement for default sprintf which buggy on keil, based on Georges Menie (www.menie.org) (added 2015.05.07)
 * bug fixed: mmDeleteObjectW, forgot to read next mmAllocChain when matched id found (2015.05.16)
2.2
 * removed: size parameter in mmReadEntry and mmWriteEntry (2015.05.20)
 * added: cache for mmGetFreeMemDW for faster operation (2015.05.30)
2.3
 * removed: mmFirstchildW, mmNextSiblingW, mmGetParentW, mmEntrySetChild, mmEntrySetSibling, mmEntrySetParent (2015.08.26)
 * added: mmEntrySetNode, mmEntryGetNode (2015.08.26)
3.0
 * removed: mmCreateObject, mmDeleteObject, mmReadObject, mmWriteObject (2015.12.08)
 * removed: mmAllocEntryW, mmEntrySetNode, mmEntryGetNode, mmEndOfEntriesDW, entries operation (2015.12.08)
 * modified: support default chain allocator, can be selected from config.h (2016.01.21)
\endverbatim
 */

#ifndef _DEFS__H
#include "..\..\defs.h"
#endif	
#ifndef _CONFIG__H
#include "..\..\config.h"
#endif
//#ifndef _RTAPIS__H
//#include "..\..\RT\include\RTApis.h"
//#endif
#ifndef _MMAPIS__H

#define MM_MAJOR_VERSION			0x03
#define MM_MINOR_VERSION			0x00
#define MM_VERSION					((MM_MAJOR_VERSION << 4) | MM_MINOR_VERSION)

/* partition management configuration */
#if MM_ENTRIES_DOWNWARD
#define MM_FLASH_HEAP_START0 		0UL											//start of heap area

#if MM_OBJECT_ALLOCATOR
#define MM_FLASH_HEAP_MAXSIZE0  	(KN_FILE_SYSTEM_SIZE)							//size of heap area
#else
//#define MM_FLASH_HEAP_MAXSIZE0  	(mmEndOfEntriesDW() - MM_FLASH_HEAP_START0)	//size of heap area
#define MM_FLASH_HEAP_MAXSIZE0  	(KN_FILE_SYSTEM_SIZE)		
#endif

#else

#define MM_FLASH_HEAP_START0 		MM_ENTRIES_TABLE_SIZE						//start of heap area

#if MM_OBJECT_ALLOCATOR
#define MM_FLASH_HEAP_MAXSIZE0  	KN_FILE_SYSTEM_SIZE							//size of heap area	
#else
#define MM_FLASH_HEAP_MAXSIZE0  	(KN_FLASH_SIZE - MM_FLASH_HEAP_START0)		//size of heap area
#endif

#endif

#if MM_OBJECT_ALLOCATOR 
#define MM_FLASH_HEAP_START1		MM_FLASH_HEAP_START0 + MM_FLASH_HEAP_MAXSIZE0
#define MM_FLASH_HEAP_MAXSIZE1		(mmEndOfEntriesDW() - MM_FLASH_HEAP_START1)
#endif

/* memory management constants */
#define MM_MEMTYPE_RAM      	0x00		   /*!< select RAM, mmGetFreeMemDW argument */
#define MM_MEMTYPE_FLASH0    	0x80		   /*!< select Flash, mmGetFreeMemDW argument */
#define MM_MEMTYPE_FLASH1    	0x81		   /*!< select Flash, mmGetFreeMemDW argument */

#ifndef MM_RAM_HEAP_SIZE
#define MM_RAM_HEAP_SIZE    	256		   	/*!< maximum RAM heap size can be used by mmAlloc */
#endif

#ifndef MM_FLASH_HEAP_START0
#define MM_FLASH_HEAP_START0 	0x200UL	  	/*!< default object heap start address (logical) */
#endif

#ifndef MM_CONTEXT_SWITCH
#define MM_CONTEXT_SWITCH		0
#endif

#ifndef OS_INIT_CONTEXT     //!<case of multithreaded
#define OS_INIT_CONTEXT()	 				/*!< default multitasking environment, init context */
#endif

#ifndef OS_SAVE_CONTEXT     //case of multithreaded	  
#define OS_SAVE_CONTEXT()		0				/*!< default multitasking environment, saving context (enter critical section) */
#endif

#ifndef OS_RESTORE_CONTEXT      //case of multithreaded
#define OS_RESTORE_CONTEXT(x)	(x)			/*!< default multitasking environment, restore context (exit critical section) */
#endif

#ifndef OS_SET_ASYNC_CONTEXT
#define OS_SET_ASYNC_CONTEXT(x) (x)	  		/*!< default multitasking environment, set current task async context */
#endif

#ifndef OS_GET_ASYNC_CONTEXT
#define OS_GET_ASYNC_CONTEXT() 	0 		/*!< default multitasking environment, get current task async context */
#endif

#ifndef MM_FLASH_PAGESIZE					
#define MM_FLASH_PAGESIZE		(KN_FLASH_PAGESIZE - sizeof(MMMazeSign))		/*!< logical page size each sector used by Maze (flash rotator) */
#endif

#define MM_MAZE_CTX_STACK_SIZE	14
#define MM_MAZE_PAGE_MAGIC      0xAA	/*!< page signature magic number */
#define MM_MAZE_TABLE_MAGIC     0x55	/*!< map signature magic number */

#define MM_ROOT_ENTRY_TAG      	'R'	   	/*!< root entry magic number */
#define MM_OBJECT_MAGIC			0xEA	/*!< object magic number */		


#ifdef _RTAPIS__H
#define mmSetLastError(x) 		rtSetLastError(x)
#else
#define mmSetLastError(x)
#endif

#ifdef _RTAPIS__H
#define mmExit(x) 				rtExit(x)
#else
#define mmExit(x)
#endif
/*!
 * Object ID base type
 * \brief upon each object allocated by mmCreateObject the corresponding object id is returned
 */
typedef struct MMMazeSign MMMazeSign;
typedef struct MMMazeSector MMMazeSector;
typedef struct MMMazeAsyncCtx MMMazeAsyncCtx;
typedef struct MMAllocChain MMAllocChain;

/*!
 * Maze (flash rotation) signature
 * \brief default signature at the end of each sector (physical) mark for valid logical sector
 */
_ALIGN1_ struct MMMazeSign {
	WORD id;					//sequence number	-> logical_id
	BYTE cntr;					//sequence counter	-> validation counter
	BYTE tag;					//maze tag
};

/*!
 * Maze (flash rotation) sector
 * \brief physical structure of each sector
 */
_ALIGN1_ struct MMMazeSector {
	//BYTE bytes[MM_FLASH_PAGESIZE];
	MMMazeSign mark;
};

#ifndef MM_MAZE_PAGE_RECORDS	//number of page record each table, each record coded in 2 bytes
#define MM_MAZE_PAGE_RECORDS	(MM_FLASH_PAGESIZE >> 1)
#endif

#ifndef MM_MAZE_TABLE_COUNT
#if MM_ENTRIES_DOWNWARD
#define MM_MAZE_TABLE_COUNT 	(((KN_FLASH_SIZE - MM_FLASH_HEAP_START0) / (MM_MAZE_PAGE_RECORDS * MM_FLASH_PAGESIZE)) + 1)
#else
#define MM_MAZE_TABLE_COUNT 	((KN_FLASH_SIZE / (MM_MAZE_PAGE_RECORDS * MM_FLASH_PAGESIZE)) + 1)
#endif
#endif

#ifndef MM_MAZE_TABLE_SIZE		
#define MM_MAZE_TABLE_SIZE		(MM_MAZE_TABLE_COUNT * KN_FLASH_PAGESIZE)
#endif

#ifndef MM_FLASH_HEAP_SIZE0
#if MM_USE_FLASH_ROTATION
//#define MM_FLASH_HEAP_SIZE0		((MM_FLASH_HEAP_MAXSIZE0 - MM_MAZE_TABLE_SIZE) * MM_FLASH_PAGESIZE) / KN_FLASH_PAGESIZE
#define MM_FLASH_HEAP_SIZE0		MM_FLASH_HEAP_MAXSIZE0
#else
#define MM_FLASH_HEAP_SIZE0		MM_FLASH_HEAP_MAXSIZE0
#endif
#endif

#ifndef MM_FLASH_HEAP_END0
#define MM_FLASH_HEAP_END0 		(MM_FLASH_HEAP_START0 + MM_FLASH_HEAP_SIZE0)
#endif

#ifndef MM_UNKNOWN_SECTOR
#define MM_UNKNOWN_SECTOR		0xFFFF		  	/*!< unknown sector id constant */
#endif
										
#define MM_ITOA_PAD_ZERO	0xC0
#define MM_ITOA_PAD_SPACE	0x80
#define MM_ITOA_BYTE		0x20
#define MM_ITOA_WORD		0x00
#define MM_ITOA_RADIX_16	0x10

#define MM_TAG_ALL			0xFF

#define MM_ASSERT(x)			(x)	  

/*!
 * Object allocator header
 */
_ALIGN1_ struct MMAllocChain {
	BYTE magic;
    WORD size;
    struct MMAllocChain * next;
};


/*!
 * Context to active asynchronous operation
 * \brief an instance for the specified asynchronous operation
 */
struct MMMazeAsyncCtx {	
	#if MM_CONTEXT_SWITCH
	DWORD os_ctx;
	#endif
	WORD index;	
	//WORD table[MM_MAZE_TABLE_COUNT];
	WORD stack[MM_MAZE_CTX_STACK_SIZE];
};

typedef _ALIGN1_ struct MMTransientObject {
	BYTE tag;		//4(high) magic, 4(low) refcounter
	BYTE len;
	BYTE bytes[1];
} MMTransientObject;

//object allocator APIs (persistent)
void mmInit() _REENTRANT_ ;
void mmInitObjects(void * handle) _REENTRANT_ ;
void mmFormat(void * handle) _REENTRANT_ ;  
ptr_u mmGetFreeMemDW(void * handle) _REENTRANT_ ;

//memory allocator APIs	(transient)	 
void * mmAllocMemP(WORD size) _REENTRANT_ ;
void mmFreeMem(void * ptr) _REENTRANT_ ;
void * mmAllocTransient(BYTE tag, WORD size) _REENTRANT_ ;
void mmReleaseTransient(void * ptr) _REENTRANT_ ;
void * mmFirstChunk(void) _REENTRANT_ ;
void * mmNextChunk(void * chunk) _REENTRANT_ ;
WORD mmSizeChunk(void * chunk) _REENTRANT_ ;
void * mmShiftNextChunk(void * chunk) _REENTRANT_ ;	 
void * mmGetDataOffset(void * chunk) _REENTRANT_ ;

//bootstrap APIs
extern BYTE g_baAllocBuffer[];  
WORD mmRamReadW(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ ;
WORD mmRamWriteW(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ ;

//maze operation APIs
void mmMazeInit() _REENTRANT_ ;
WORD mmMazeWriteFlash(void * handle, ptr_u address, void * bytes, WORD length) _REENTRANT_ ;
WORD mmMazeReadFlash(void * handle, ptr_u address, void * bytes, WORD length) _REENTRANT_ ;
WORD mmMazeLocateTable(BYTE id) _REENTRANT_ ;
WORD mmMazeLocateEmptyPage(void) _REENTRANT_ ;
WORD mmMazeTranslate(WORD pagenum) _REENTRANT_ ;		//translate logical address to physical address
void mmMazeUpdateTable(WORD index, WORD * newval, WORD total) _REENTRANT_ ;
WORD mmMazeRelocateSectorW(WORD origin) _REENTRANT_ ;

//maze asynchronous operation APIs
#if MM_ASYNC_SUPPORT
MMMazeAsyncCtx * mmBeginTransaction(void) _REENTRANT_ ;
MMMazeAsyncCtx * mmCommitTransaction(void) _REENTRANT_ ;
MMMazeAsyncCtx * mmRollbackTransaction(void) _REENTRANT_ ;												
void mmEndTransaction(void) _REENTRANT_ ;
#else
#define mmBeginTransaction()		(0)
#define mmCommitTransaction()
#define mmRollbackTransaction()
#define mmEndTransaction()
#endif

//memory utilities
//BYTE mmSprintf(BYTE *out, BYTE *format, ...) _REENTRANT_ ;
//#define mmSprintf sprintf
void mmMemCpy(BYTE * dst, BYTE * src, WORD size) _REENTRANT_ ;
void mmMemSet(BYTE * buf, BYTE val, WORD size) _REENTRANT_ ;
BYTE mmMemCmp(BYTE * op1, BYTE * op2, WORD size) _REENTRANT_ ;
WORD mmAtoi(BYTE* buf)_REENTRANT_;
BYTE mmItoa(BYTE mode, BYTE * buffer, WORD val) _REENTRANT_ ;
BYTE mmStrLen(BYTE * buffer) _REENTRANT_ ;
BYTE mmPushTlv(BYTE * buffer, BYTE tag, BYTE length, BYTE * value) _REENTRANT_ ;
BYTE mmPopTlv(BYTE * buffer, BYTE * tag, WORD * size, BYTE * value) _REENTRANT_ ; 
WORD mmPopTlvByTag(BYTE * buffer_in, BYTE size, BYTE tag, BYTE * buffer_out) _REENTRANT_ ;

#define _MMAPIS__H
#endif
