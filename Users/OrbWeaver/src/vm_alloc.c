//
//  MMRamAllocator.c
//  
//
//  Created by Agus Purwanto on 3/12/14.
//
//

#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\MMApis.h"
//#include "..\..\KN\include\KNApis.h"
#include "..\inc\VMStackApis.h"

#define MM_RAM_HEAP_SIZE				0x1200

void vm_memcpy(void * dst, void * src, uint16 size) _REENTRANT_ {
	WORD i;
	uint8 * p_dst = dst;
	uint8 * p_src = src;
	if(p_dst == p_src) return;
	if(size == 0) return;
  	if(p_dst > p_src) {
		for(i=size;i>0;) {
			i--;
		  	p_dst[i] = p_src[i];
		}
	} else {
		for(i=0;i<size;i++) {
			p_dst[i] = p_src[i];
		}
	}
}

void vm_memset(void * buf, uint8 val, uint16 size) _REENTRANT_ {
	WORD i;
	uint8 * ptr = buf;
	for(i=0;i<size;i++) {
		ptr[i] = val;
	}
}

uint8 vm_imemcmp(void * op1, void * op2, uint16 size) _REENTRANT_ {
	uint8 * p_dst = op1;
	uint8 * p_src = op2;
	uint8 c, d;
 	while(size-- != 0) {
		c = p_dst[size];
		d = p_src[size];
		if(c >= 'a' && c <='z') c -= 0x20;
		if(d >= 'a' && d <='z') d -= 0x20;
	 	if(c != d) return -1;
	}
	return 0;
}

uint8 vm_memcmp(void * op1, void * op2, uint16 size) _REENTRANT_ {
	uint8 * p_dst = op1;
	uint8 * p_src = op2;
 	while(size-- != 0) {
	 	if(p_dst[size] != p_src[size]) return -1;
	}
	return 0;
}

//#if MM_MALLOC_API == MM_API_CHAIN
BYTE g_baAllocBuffer[MM_RAM_HEAP_SIZE + 0x10];
void mmInit() _REENTRANT_ {
	vm_memset(g_baAllocBuffer, 0, sizeof(MMAllocChain) * 2);	 
	((MMAllocChain *)g_baAllocBuffer)->next = (MMAllocChain *)((uint8 *)g_baAllocBuffer + sizeof(MMAllocChain));
}

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
    MMAllocChain * alloc_ptr;
	MMAllocChain * alloc_ptr_temp = (MMAllocChain *)NULL;
	MMAllocChain * candidate;
	DWORD cpu_sr;
    OS_INIT_CONTEXT();
    cpu_sr = OS_SAVE_CONTEXT();                 //critical section
	alloc_ptr = ((MMAllocChain *)g_baAllocBuffer);
#if 0
    while(TRUE) {
        if(alloc_ptr->next == NULL) {		//allocate new chunk at the end of the heap
            if( ((ptr_u)alloc_ptr + size + (sizeof(MMAllocChain) + sizeof(MMAllocChain))) >= ((ptr_u)g_baAllocBuffer + MM_RAM_HEAP_SIZE)) break;        //not enough memory
            alloc_ptr->size = size;
            alloc_ptr->next = (MMAllocChain *)((ptr_u)alloc_ptr + (sizeof(MMAllocChain) + size));
            alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
            alloc_ptr_temp->next = NULL;
            alloc_ptr_temp->size = 0;
            allocated = (void *)((ptr_u)alloc_ptr + sizeof(MMAllocChain));	//return pointer sekarang + ukuran header karena *[header]+[body]
            break;
        }
        alloc_ptr_temp = alloc_ptr;
        alloc_ptr = (MMAllocChain *)alloc_ptr->next;
        if((ptr_u)alloc_ptr >= ((ptr_u)alloc_ptr_temp + (alloc_ptr_temp->size + (sizeof(MMAllocChain) + sizeof(MMAllocChain)) + size))) {

            candidate = (MMAllocChain *)((ptr_u)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(MMAllocChain)));
            candidate->size = size;
            alloc_ptr_temp->next = candidate;
            candidate->next = alloc_ptr;
            //OS_RESTORE_CONTEXT
            allocated = (void *)((ptr_u)candidate + (ptr_u)sizeof(MMAllocChain));
            break;
        }
    }
#else
    while(TRUE) {
        if(alloc_ptr->next == NULL) {		//allocate new chunk at the end of the heap
            if( ((uint8 *)alloc_ptr + size + (sizeof(MMAllocChain) + sizeof(MMAllocChain))) >= ((uint8 *)g_baAllocBuffer + MM_RAM_HEAP_SIZE)) break;        //not enough memory
            alloc_ptr->size = size;
            alloc_ptr->next = (MMAllocChain DYNAMIC *)((uint8 *)alloc_ptr + (sizeof(MMAllocChain) + size));
            alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
            alloc_ptr_temp->next = NULL;
            alloc_ptr_temp->size = 0;
            allocated = (MMAllocChain DYNAMIC *)((uint8 *)alloc_ptr + sizeof(MMAllocChain));	//return pointer sekarang + ukuran header karena *[header]+[body]
            break;
        }
        alloc_ptr_temp = alloc_ptr;
        alloc_ptr = (MMAllocChain *)alloc_ptr->next;
        if((uint8 *)alloc_ptr >= ((uint8 *)alloc_ptr_temp + (alloc_ptr_temp->size + (sizeof(MMAllocChain) + sizeof(MMAllocChain)) + size))) {

            candidate = (MMAllocChain DYNAMIC *)((uint8 *)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(MMAllocChain)));
            candidate->size = size;
            alloc_ptr_temp->next = candidate;
            candidate->next = alloc_ptr;
            //OS_RESTORE_CONTEXT
            allocated = (MMAllocChain DYNAMIC *)((uint8 *)candidate + sizeof(MMAllocChain));
            break;
        }
    }
#endif
    OS_RESTORE_CONTEXT(cpu_sr);
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
	MMAllocChain * alloc_ptr = ((MMAllocChain *)g_baAllocBuffer);
	MMAllocChain * temp = NULL;
	DWORD cpu_sr;
    OS_INIT_CONTEXT();
    
    cpu_sr = OS_SAVE_CONTEXT();                 //critical section
	while((ptr_u)alloc_ptr != NULL) {
		if( ((ptr_u)alloc_ptr->next) == (ptr_u)((uint8 *)ptr - sizeof(MMAllocChain)) ) {		//found
			temp = alloc_ptr->next;
			alloc_ptr->next = temp->next;
			if(alloc_ptr->next == NULL) { 		//shift end of heap added 2015.04.09 v2.1, modified 2015.04.14
				alloc_ptr->next = temp;
			}
			temp->size = 0;
			temp->next = NULL;
            goto exit_free;
		}
		alloc_ptr = alloc_ptr->next;		//next iterator
	}
	temp = NULL;
	exit_free:
    OS_RESTORE_CONTEXT(cpu_sr);
    return;
}

void * mmFirstChunk(void) _REENTRANT_ {
	return (void *)((ptr_u)g_baAllocBuffer + sizeof(MMAllocChain));
}

void * mmNextChunk(void * chunk) _REENTRANT_ {
	MMAllocChain * alloc_ptr = (MMAllocChain *)((ptr_u)chunk - sizeof(MMAllocChain));
	alloc_ptr = alloc_ptr->next;
	if(alloc_ptr == NULL) return NULL;
	return (void *)((ptr_u)alloc_ptr + sizeof(MMAllocChain));
}

WORD mmSizeChunk(void * chunk) _REENTRANT_ {
	MMAllocChain * alloc_ptr = (MMAllocChain *)((ptr_u)chunk - sizeof(MMAllocChain));
	return alloc_ptr->size;
}

void * mmShiftNextChunk(void * chunk) _REENTRANT_ {
	MMAllocChain * alloc_ptr = NULL;
	MMAllocChain * next_ptr = NULL;
	WORD gap = 0;
	if(chunk == NULL) return NULL;
	alloc_ptr = (MMAllocChain *)((ptr_u)chunk - sizeof(MMAllocChain));
	next_ptr = alloc_ptr->next;
	if(next_ptr == NULL) return NULL;
	gap = (ptr_u)next_ptr - ((ptr_u)alloc_ptr + sizeof(MMAllocChain) + alloc_ptr->size);
	if(gap > 0) {
		alloc_ptr->next = (MMAllocChain *)((ptr_u)alloc_ptr + sizeof(MMAllocChain) + alloc_ptr->size);	  
		alloc_ptr->next->size = next_ptr->size;
		alloc_ptr->next->next = next_ptr->next;
		vm_memcpy((BYTE *)alloc_ptr->next + sizeof(MMAllocChain), (BYTE *)next_ptr + sizeof(MMAllocChain), alloc_ptr->next->size);
		next_ptr = alloc_ptr->next;
	}
	//check if next->next == end of heap
	if(next_ptr->size == NULL && next_ptr->next == NULL) {
		return NULL;
	}
	return ((BYTE *)next_ptr + sizeof(MMAllocChain));
}

void * mmAllocTransient(BYTE tag, WORD size) _REENTRANT_ {
	uint8 * allocated = (uint8 *)mmAllocMemP(3 + size);
	if(allocated == NULL) return NULL;
	allocated[0] = tag;
	allocated[1] = size >> 8;
	allocated[2] = size;
	return (allocated + 3);
}

void mmReleaseTransient(void * ptr) _REENTRANT_ {
	if(ptr == NULL) return;
	mmFreeMem((BYTE *)((ptr_u)ptr - 3));
}
//#endif
