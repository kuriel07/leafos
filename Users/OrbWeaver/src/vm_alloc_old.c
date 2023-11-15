//
//  MMRamAllocator.c
//  
//
//  Created by Agus Purwanto on 3/12/14.
//
//

#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\include\MMApis.h"
#include "..\..\KN\include\KNApis.h"
#include "..\..\BS\include\BSApis.h"
						  
WORD mmRamReadW(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ {
	mmMemCpy(buffer, (BYTE *)handle + (WORD)offset, size);
	return size;
}

WORD mmRamWriteW(void * handle, ptr_u offset, void * buffer, WORD size) _REENTRANT_ {
 	mmMemCpy((BYTE *)handle + (WORD)offset, buffer, size);
	return size;
}


#if MM_MALLOC_API == MM_API_AKASHIC		
BYTE g_baAllocBuffer[MM_RAM_HEAP_SIZE + 0x10];
/*!
******************************************************************************
\fn  		void * mmAllocMemP(WORD size)
\brief    	allocate chunk of memory (RAM) and return the corresponding pointer	
\param		size size of heap to allocate (IN)	   
\return   	pointer to newly allocated memory
\author   	AGP
\version  	1.0
\date     	2014.03.19

\verbatim

allocate a new heap to be used by later application

\endverbatim
******************************************************************************
*/
void * mmAllocMemP(WORD size)_REENTRANT_{
    void * allocated = NULL;  
	BYTE tag = MM_OBJECT_MAGIC;
#if MM_CONTEXT_SWITCH
	DWORD cpu_sr;
    OS_INIT_CONTEXT();
    cpu_sr = OS_SAVE_CONTEXT();                 //critical section
#endif
	allocated = mmAllocTransient(tag, size);
#if MM_CONTEXT_SWITCH
    OS_RESTORE_CONTEXT(cpu_sr);
#endif
    return allocated;
}

/*!
******************************************************************************
\fn  		void mmFreeMem(void * ptr)
\brief    	release an allocated memory
\param 		ptr	pointer to corresponding heap to be freed (IN)	   
\return   	void
\author   	AGP
\version  	1.1
\date     	2014.03.19

\verbatim

release allocated heap by scanning all allocated heap, if the chunk didn't exist
it will automatically quit
* modified 2015.04.09 shifted end of heap on last chunk release

\endverbatim
******************************************************************************
*/
void mmFreeMem(void * ptr) _REENTRANT_ { 	 
	BSAllocHandle bHandle;
#if MM_CONTEXT_SWITCH
	DWORD cpu_sr;	   
    OS_INIT_CONTEXT();
    cpu_sr = OS_SAVE_CONTEXT();                 //critical section
#endif
	//mmReleaseTransient(ptr);
	if(ptr == NULL) return;
	//mmFreeMem((BYTE *)((ptr_u)ptr - 2));
	ptr = (ptr_u)((WORD)ptr - (WORD)g_baAllocBuffer);
	bsInitAllocContext(BS_RAM_DRIVER, (BSAbstractionHandle *)&bHandle, 0, sizeof(g_baAllocBuffer));
	bsReleaseByDataOffset(&bHandle, ptr);
#if MM_CONTEXT_SWITCH
	OS_RESTORE_CONTEXT(cpu_sr);
#endif
    return;
}

void * mmFirstChunk(void) _REENTRANT_ {
	WORD objLen;
	BYTE advHdr = bsGetHeaderLength(g_baAllocBuffer, &objLen);
	return (void *)((ptr_u)g_baAllocBuffer);
}

void * mmNextChunk(void * chunk) _REENTRANT_ {
	WORD objLen;
	BYTE advHdr;
	extern CONST BYTE g_endMark[];
	//MMAllocChain * alloc_ptr = (MMAllocChain *)((ptr_u)chunk - sizeof(MMAllocChain));
	//alloc_ptr = alloc_ptr->next;
	if(mmMemCmp(chunk, g_endMark, 2) == 0) return NULL;		//end mark
	//if(alloc_ptr == NULL) return NULL;
	//if(alloc_ptr->size == 0) return NULL;
	do {	
		advHdr = bsGetHeaderLength(chunk, &objLen);
		chunk = (void *)((ptr_u)chunk + advHdr + (WORD)objLen);	  
		//if(mmMemCmp(chunk, g_endMark, 2) == 0) return NULL;		//end mark
	} while(((BYTE *)chunk)[0] == 0);
	return chunk;
}

WORD mmSizeChunk(void * chunk) _REENTRANT_ {
	WORD objLen;
	extern CONST BYTE g_endMark[];
	//MMAllocChain * alloc_ptr = (MMAllocChain *)((ptr_u)chunk - sizeof(MMAllocChain));
	if(mmMemCmp(chunk, g_endMark, 2) == 0) return NULL;		//end mark
	bsGetHeaderLength(chunk, &objLen);
	return (WORD)objLen;
}

void * mmShiftNextChunk(void * chunk) _REENTRANT_ {
	ptr_u alloc_ptr = NULL;
	ptr_u next_ptr = NULL;
	WORD gap = 0;
	WORD objLen;
	BYTE advHdr;
	BYTE tlvBuffer[6];
	extern CONST BYTE g_endMark[];
	if(chunk == NULL) return NULL;
	tlvBuffer[0] = 0;
	//alloc_ptr = (ptr_u)chunk;
	if(mmMemCmp(chunk, g_endMark, 2) == 0) return NULL;
	advHdr = bsGetHeaderLength(chunk, &objLen);
	alloc_ptr = ((ptr_u)chunk + advHdr + (WORD)objLen);
	read_next_chunk:
	next_ptr = mmNextChunk(chunk); 
	if(next_ptr == NULL) return NULL;
	advHdr = bsGetHeaderLength(next_ptr, &objLen);
	//if(next_ptr[0] == 0) {
	//	gap += advHdr + (WORD)objLen;
	//	goto read_next_chunk;
	//}
	gap = (WORD)next_ptr - (WORD)alloc_ptr;
	if(gap > 0) {
		//check for end mark
		if(mmMemCmp(next_ptr, g_endMark, 2) == 0) {
			//shift end mark
			shift_end_mark:
			mmMemCpy((BYTE *)alloc_ptr, (BYTE *)g_endMark, 2);
			next_ptr = alloc_ptr;
		} else {
			//shift next chunk
			mmMemCpy((BYTE *)alloc_ptr, (BYTE *)next_ptr, advHdr + (WORD)objLen);
			//relocate freespace
			next_ptr = alloc_ptr;		//back to first iterator
			mmMemCpy((BYTE *)alloc_ptr + advHdr + (WORD)objLen, tlvBuffer, 
				bsCalcHeaderLength(tlvBuffer, bsChunkLeft(tlvBuffer, (WORD)gap)));
		}
	}
	return next_ptr;
	//check if next->next == end of heap
}

void * mmGetDataOffset(void * chunk) _REENTRANT_ {
	BYTE advHdr;
	WORD objLen;
 	advHdr = bsGetHeaderLength(chunk, &objLen);
	return (ptr_u)chunk + advHdr;
}

void * mmAllocTransient(BYTE tag, WORD size) _REENTRANT_ {
	BYTE tagBuf[3];
	BSAllocHandle bHandle;
	tagBuf[0] = tag;
	tagBuf[1] = 0x1F;
	bsInitAllocContext(BS_RAM_DRIVER, (BSAbstractionHandle *)&bHandle, 0, sizeof(g_baAllocBuffer));
	return g_baAllocBuffer + (WORD)bsAllocObject(&bHandle, tagBuf, size);
}

void mmReleaseTransient(void * ptr) _REENTRANT_ {
	BSAllocHandle bHandle;
	if(ptr == NULL) return;
	//mmFreeMem((BYTE *)((ptr_u)ptr - 2));
	ptr = (ptr_u)((WORD)ptr - (WORD)g_baAllocBuffer);
	bsInitAllocContext(BS_RAM_DRIVER, (BSAbstractionHandle *)&bHandle, 0, sizeof(g_baAllocBuffer));
	//bsDeallocObject(&bHandle, ptr);
    bsReleaseByOffset(&bHandle, ptr);
}
#endif
