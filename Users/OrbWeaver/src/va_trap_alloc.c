#include "..\..\defs.h"
#include "..\..\config.h"
#include <string.h>
#include <stdio.h>
#ifndef _BSAPIS__H	
#include "..\inc\va_trap_apis.h"
#endif
#include "..\..\interfaces\inc\if_apis.h"

/*
 * implementation of tlv allocator using tlv chaining
 * added support for primitive 0x9F and constructed 0xBF (2015.08.21)
 * 
 */ 
CONST BYTE g_endMark[] = { 0xFF, 0xFF };


/*!
******************************************************************************
\section  BYTE bsErr(ptr_u result)
\brief    check for error when allocating/accessing any bootstrapped context (such as find tag or allocating new DO)
\param 	  result from specific operation (IN) 
\return   1 on error
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

check for error when allocating/accessing any bootstrapped context (such as find tag or allocating new DO)

\endverbatim
******************************************************************************
*/	 
BYTE bsErr(ptr_u result) _REENTRANT_ {
   	if(result == (DWORD)BS_TAG_NOT_FOUND) return 1;
	return 0;
}

/*!
******************************************************************************
\section  BYTE bsNull(ptr_u result) 
\brief    check for null pointer value
\param 	  result from specific operation (IN) 
\return   1 on null
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

check for null pointer value

\endverbatim
******************************************************************************
*/	 
BYTE bsNull(ptr_u result) _REENTRANT_ {
 	if((DWORD)result == (DWORD)NULL) return 1;
	return 0;
}

static WORD bsTagValue(BYTE tag[3]) _REENTRANT_ {
	//convert to integer value for comparasion
	if((tag[0] & 0xDF) == 0x9F) {
		if(tag[1] >= 0x1F && tag[1] <= 0x7F) {
			return 0x8000 | (tag[1] + ((tag[0] & 0x20) << 2));
		}
		return ((tag[0] & 0x20) << 10) | (end_swap16(*((WORD *)(tag + 1))) & 0x7FFF);
	}
	return (WORD)tag[0];
}

WORD bsChunkLeft(BYTE * tag, WORD actualLeft) _REENTRANT_ {
	BYTE i = bsTagLength(tag);
	if((WORD)actualLeft < (WORD)(i+1)) return 0;
	if((WORD)actualLeft < (WORD)(i+128)) return actualLeft - (1 + i); //x tag, 1 len
	if((WORD)actualLeft < (WORD)(i+258)) return actualLeft - (2 + i); //x tag, 2 len
	return actualLeft - (3 + i);						//x tag, 3 len
}

BYTE bsTagLength(BYTE tag[3]) _REENTRANT_ {		/* new */
	//calculate actual tag length in bytes
	if((tag[0] & 0xDF) == 0x9F) {
		if(tag[1] >= 0x1F && tag[1] <= 0x7F) return 2;
		if(tag[1] & 0x80 && tag[1] != 0x80) {
		 	if((tag[2] & 0x80) == 0) return 3;
		}
		return 0;			//invalid tag
	}
	return 1;
}  

BYTE bsGetHeaderLength(BYTE * buffer, WORD * length) _REENTRANT_ {
	//get total header length including sizeof(tag)+sizeof(length)
	BYTE byteAdvanced = 1;
	BYTE i = bsTagLength(buffer);
	if(buffer[i] & 0x80) {
		byteAdvanced = ((buffer[i] & 0x0F) + 1);
		///printf("byte advanced %d\n", byteAdvanced);
		if(byteAdvanced == 3) {			//82 XX XX
			*length = (ptr_u)end_swap16(*((WORD *)(buffer + i + 1)));
		} else {	//if(byteAdvanced == 1) {
			*length = (ptr_u)buffer[i + 1];	
		}
	} else {
		*length = (ptr_u)buffer[i];
	}
	return (byteAdvanced + i);	
}

BYTE bsCalcHeaderLength(BYTE * buffer, WORD length) _REENTRANT_ {
	//calculate total header length including sizeof(tag)+sizeof(length)
	BYTE i = bsTagLength(buffer);
	BYTE k;
	if((WORD)length < 0x7F) { buffer[i] = length; k=1; }
	else if((WORD)length < 0x100) { buffer[i] = 0x81; buffer[i + 1] = length; k=2; }
	else {
		buffer[i] = 0x82;
		buffer[i+1] = (WORD)length >> 8;
		buffer[i+2] = (BYTE)length;
		k = 3;
	}
	return i + k;
}

static ptr_u bsMarkObject(BSAllocHandle * handle, BYTE * tag, ptr_u offset, WORD size) _REENTRANT_ {
	BYTE advHdr, pAdvHdr;
	BYTE tlvBuffer[6];
	BYTE hdrBuffer[6];
	WORD objLen, chunkLeft; 
	ptr_u nextOffset, nOffset;
	//void * fhandle = ((BSAbstractionHandle *)handle)->handle;
	memcpy(hdrBuffer, tag, 3);
	if(bsHandleReadW(handle, offset, tlvBuffer, sizeof(tlvBuffer)) != sizeof(tlvBuffer)) return BS_TAG_NOT_FOUND;
	
	//fsFlagChunk(0x01, offset);		//flag chunk
	if(tlvBuffer[0] == 0xFF) {		//end mark offset
		tlvBuffer[0] = 0;			//set to empty object
		advHdr = bsCalcHeaderLength(hdrBuffer, size);
		//chunkLeft = bsChunkLeft(tlvBuffer, (WORD)size);
		pAdvHdr = bsCalcHeaderLength(tlvBuffer, size);	 
		//check limit  (added 2016.06.28)
		if(((DWORD)offset + advHdr + size) >= (DWORD)handle->end) return BS_TAG_NOT_FOUND;
		//start marking
		if(bsHandleWriteW(handle, offset + advHdr + (WORD)size, (BYTE *)g_endMark, 2) != 2) return BS_TAG_NOT_FOUND;
		bsHandleWriteW(handle, offset, tlvBuffer, pAdvHdr);		//mark here
	} else {
		pAdvHdr = (bsGetHeaderLength(tlvBuffer, &objLen));
		advHdr = (bsCalcHeaderLength(hdrBuffer, size));
		//calculate next chunk offset and mark next chunk
		nextOffset = (offset + pAdvHdr + (WORD)objLen);
		nOffset = offset + advHdr + (WORD)size;
		if((ptr_u)nextOffset == nOffset) {
			bsHandleWriteW(handle, offset, hdrBuffer, advHdr);		//mark here
			goto exit_mark_object;
		}
		if(nextOffset < (nOffset + 2)) return BS_TAG_NOT_FOUND;		//fail to mark
		//printf("mark tag : %02x at %08x\n", hdrBuffer[0], offset);
		bsHandleWriteW(handle, offset, hdrBuffer, advHdr);		//mark here
		chunkLeft = bsChunkLeft(hdrBuffer, (WORD)(nextOffset - nOffset));
		tlvBuffer[0] = 0;												//empty tag
		bsHandleWriteW(handle, nOffset, tlvBuffer, bsCalcHeaderLength(tlvBuffer, chunkLeft));
	}
exit_mark_object:
	return offset;
}
	 
#if 0
static void bsShiftObject(BSAllocHandle * handle, ptr_u dst, ptr_u src, WORD size) _REENTRANT_ {
	WORD ioffset;
	BYTE dbuffer[32];
	WORD tbCpy = size, tbWrt;
	//void * fhandle = ((BSAbstractionHandle *)handle)->handle;
	if(dst == src) return;
	if(dst < src) {		//forward shift
		for(ioffset = 0; ioffset < tbCpy; ioffset += sizeof(dbuffer)) {
			tbWrt = ((ioffset + sizeof(dbuffer)) > tbCpy)?(tbCpy - ioffset):sizeof(dbuffer);
			bsHandleReadW(handle, src + ioffset, dbuffer, tbWrt); //((BSAbstractionHandle *)handle)->read(fhandle, src + ioffset, dbuffer, tbWrt);
			bsHandleWriteW(handle, dst + ioffset, dbuffer, tbWrt); //((BSAbstractionHandle *)handle)->write(fhandle, dst + ioffset, dbuffer, tbWrt);
		}
	} else {			//backward shift
		tbWrt = tbCpy % sizeof(dbuffer);
		ioffset = tbCpy - tbWrt;
		for(ioffset; ioffset > 0; ioffset -= sizeof(dbuffer)) {
			bsHandleReadW(handle, src + ioffset, dbuffer, tbWrt); //((BSAbstractionHandle *)handle)->read(fhandle, src + ioffset, dbuffer, tbWrt);
			bsHandleWriteW(handle, dst + ioffset, dbuffer, tbWrt); //((BSAbstractionHandle *)handle)->write(fhandle, dst + ioffset, dbuffer, tbWrt);
			tbWrt = sizeof(dbuffer);
		}
	}
}

ptr_u bsResizeObject(BSAllocHandle * handle, ptr_u offset, WORD size) _REENTRANT_ {
	BYTE advHdr, pAdvHdr;
	BYTE tlvBuffer[6];
	BYTE hdrBuffer[6];
	WORD objLen, chunkLeft, maxLen; 
	ptr_u nextOffset, nOffset;
	//void * fhandle = ((BSAbstractionHandle *)handle)->handle;
	if(bsHandleReadW(handle, offset, tlvBuffer, sizeof(tlvBuffer)) != sizeof(tlvBuffer)) return BS_TAG_NOT_FOUND;
	pAdvHdr = (bsGetHeaderLength(tlvBuffer, &objLen));		//current header length
	mmMemCpy(hdrBuffer, tlvBuffer, 3);
	advHdr = bsCalcHeaderLength(hdrBuffer, size);			//new header length
	nextOffset = (offset + pAdvHdr + (WORD)objLen);				//current next offset
	nOffset = offset + advHdr + (WORD)size;						//new next offset
	if(bsHandleReadW(handle, nextOffset, tlvBuffer, sizeof(tlvBuffer)) != sizeof(tlvBuffer)) return BS_TAG_NOT_FOUND;
	//fsFlagChunk(0x01, offset);		//flag chunk
	if(tlvBuffer[0] == 0xFF) {		//end mark offset
		//TODO shift end mark
		if(bsHandleReadW(handle, nOffset, tlvBuffer, 2) != 2) return BS_TAG_NOT_FOUND;
		//shift data if (pAdvHdr != advHdr)
		bsShiftObject(handle, offset + advHdr, offset + pAdvHdr, (size > (ptr_u)objLen)?(WORD)objLen:(WORD)size);
		//write new header
		bsHandleWriteW(handle, offset, hdrBuffer, advHdr);		
	} else if(tlvBuffer[0] == 0x00) {		//free chunk
		//calculate maximum freespace available
		maxLen = (ptr_u)bsGetHeaderLength(tlvBuffer, &chunkLeft);
		nextOffset = (nextOffset + (WORD)maxLen + (WORD)chunkLeft);			//maxLen currently size of freespace header
		maxLen += ((WORD)objLen + (WORD)chunkLeft) - advHdr;				//maxLen currently merged size available
		//check if merged chunk size is not matched with new allocated size
		if(nextOffset != nOffset) {
			if((ptr_u)maxLen < (ptr_u)size) return BS_TAG_NOT_FOUND;						//if(max freespace < new size) exit (no available space)
			if(nextOffset < (nOffset + 2)) return BS_TAG_NOT_FOUND;		//no marking freespace available
			//create marking freespace
			chunkLeft = bsChunkLeft(hdrBuffer, (WORD)(nextOffset - nOffset));
			tlvBuffer[0] = 0;												//empty tag
			bsHandleWriteW(handle, nOffset, tlvBuffer, bsCalcHeaderLength(tlvBuffer, chunkLeft));
		}
		//shift data if (pAdvHdr != advHdr)
		bsShiftObject(handle, offset + advHdr, offset + pAdvHdr, (size > objLen)?(WORD)objLen:(WORD)size);
		//write new header
		bsHandleWriteW(handle, offset, hdrBuffer, advHdr);
	} else return BS_TAG_NOT_FOUND;			//cannot resize object (no freespace available
exit_mark_object:
	return offset;
}
#endif

/*!
******************************************************************************
\section  ptr_u bsAllocObject(BSAllocHandle * handle, BYTE * tag, WORD size) 
\brief    allocate new data object from specified BSAllocHandle, bsAllocObject automatically commit operation
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (IN)
\param	  size amount of bytes to be allocated (IN)
\return   BS_TAG_NOT_FOUND on error, address of specified data object (not incuding tag and length) (IN)
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

allocate new data object from specified BSAllocHandle, bsAllocObject automatically commit operation

\endverbatim
******************************************************************************
*/
ptr_u bsAllocObject(BSAllocHandle * handle, BYTE * tag, WORD size) _REENTRANT_ {
	WORD objLen;
	BYTE endMark[] = { 0xFF, 0xFF };
	BYTE tlvBuffer[6];
	BYTE advHdr;
	ptr_u offset;
	tlvBuffer[0] = tag[0];
	//if(size == BS_TAG_NOT_FOUND) {	
	//	size = bsChunkLeft(tag, handle->end - handle->start);		//maximum size
	//	//printf("start : %ld, end : %ld, max size : %ld\n", handle->start, handle->end, size);
	//}
	advHdr = bsCalcHeaderLength(tlvBuffer, size);
	offset = bsFindObject(handle, BS_FIND_EMPTY, tag, &size);
	bsCommitObject(handle, offset, tag, size);
	return offset + advHdr;	
}  

/*!
******************************************************************************
\section  ptr_u bsCommitObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size)
\brief    commit newly allocated data object to specified BSAllocHandle
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (IN)
\param	  offset address which tag to be commited (received from bsFindObject) (IN)
\param	  size amount of bytes to be allocated (IN)
\return   address of commited data object (start from tag)
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

commit newly allocated data object to specified BSAllocHandle

\endverbatim
******************************************************************************
*/
ptr_u bsCommitObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size) _REENTRANT_ {
	BYTE tlHeader[6];
	BYTE advHdr;
	ptr_u actualOffset;
	//WORD prevLen;
	memcpy(tlHeader, tag, 3);
	//tlHeader[0] = tag;	  
	//fsFlagChunk(0x00, offset);					//unflag chunk
	advHdr = (bsCalcHeaderLength(tlHeader, size));
	actualOffset = (ptr_u)offset;
	bsHandleWriteW(handle, actualOffset, tlHeader, advHdr);
	return offset;
}

/*!
******************************************************************************
\section  void bsRollbackObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size)
\brief    rollback operation on newly allocated data object, freeing it for later operation
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (IN)
\param	  offset address which tag to be commited (received from bsFindObject) (IN)
\param	  size amount of bytes to be allocated (IN)
\return   BS_TAG_NOT_FOUND on error, address of specified memory
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

rollback operation on newly allocated data object, freeing it for later operation

\endverbatim
******************************************************************************
*/
void bsRollbackObject(BSAllocHandle * handle, ptr_u offset, BYTE * tag, WORD size) _REENTRANT_ {
	BYTE tlHeader[6];
	BYTE advHdr, pAdvHdr;
	ptr_u actualOffset;
	WORD chunkLeft;
	//tlHeader[0] = tag;	  
	//fsFlagChunk(0x00, offset);					//unflag chunk
	advHdr = (bsCalcHeaderLength(tag, size));
	actualOffset = (ptr_u)offset;
	 
	tlHeader[0] = 0;												//empty tag
	chunkLeft = bsChunkLeft(tlHeader, size + advHdr);
	bsHandleWriteW(handle, actualOffset, tlHeader, bsCalcHeaderLength(tlHeader, chunkLeft));
}

/*!
******************************************************************************
\section  bsPopByTag(BSAllocHandle * handle, BYTE * tag, WORD size, BYTE * buffer)
\brief    pop specified tag value from specified bootstrapped handle (BSAllocHandle)
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (IN)		 
\param	  size amount of bytes to be allocated (IN)
\param	  buffer buffer containing popped data (OUT)
\return   length of popped data object, -1 on error
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

pop specified tag value from specified bootstrapped handle (BSAllocHandle)
* fixed: bsPopByTag returning BS_TAG_NOT_FOUND when iterating (offset==size) finished (2016.04.11)

\endverbatim
******************************************************************************
*/
WORD bsPopByTag(BSAllocHandle * handle, BYTE * tag, WORD size, BYTE * buffer) _REENTRANT_ {
	WORD objLen;
	ptr_u offset;
	BYTE tagBuf[6];
	BYTE advHdr;
	memcpy(tagBuf, tag, 3);
	offset = bsFindObject(handle, BS_FIND_TAG, tagBuf, &objLen);
	//if(offset == BS_TAG_NOT_FOUND) return -1;			//tag unavailable
	if(bsErr(offset)) return -1;						//changed to use bsErr instead (2015.12.21)
	bsHandleReadW(handle, offset, tagBuf, sizeof(tagBuf));
	offset += bsGetHeaderLength(tagBuf, &objLen);
	if(objLen < size) size = objLen;
	bsHandleReadW(handle, offset, buffer, size);
	return (WORD)objLen;
}

/*!
******************************************************************************
\section  bsPopByIndex(BSAllocHandle * handle, BYTE * tag, WORD size, BYTE * buffer)
\brief    pop specified data object based on index of allocated data object
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (IN)		 
\param	  size amount of bytes to be allocated (IN)
\param	  buffer buffer containing popped data (OUT)
\return   length of popped data object, -1 on error
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

pop specified data object based on index of allocated data object

\endverbatim
******************************************************************************
*/
WORD bsPopByIndex(BSAllocHandle * handle, WORD index, WORD size, BYTE * buffer) _REENTRANT_ {
	WORD objLen;
	ptr_u offset;
	BYTE tagBuf[6];
	BYTE advHdr;
	//mmMemCpy(tagBuf, tag, 3);
	offset = bsFindObject(handle, BS_FIND_INDEX, (BYTE *)(uint32)index, &objLen);
	//if(offset == BS_TAG_NOT_FOUND) return -1;			//tag unavailable
	if(bsErr(offset)) return -1;						//changed to use bsErr instead (2015.12.21)
	bsHandleReadW(handle, offset, tagBuf, sizeof(tagBuf));
	offset += bsGetHeaderLength(tagBuf, &objLen);
	if(objLen < size) size = objLen;
	bsHandleReadW(handle, offset, buffer, size);
	return (WORD)objLen;
}

/*!
******************************************************************************
\section  bsPopByIndex(BSAllocHandle * handle, BYTE * tag, WORD size, BYTE * buffer)
\brief    retrieve header (Tag-Length) at specified address
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  offset address which data object reside (IN)
\param	  tag 3 bytes pointer/array containing tag to be allocated (OUT)		 
\param	  size length of data object (OUT)
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

retrieve header (Tag-Length only) at specified address

\endverbatim
******************************************************************************
*/
BYTE bsPopHeader(BSAllocHandleP handle, WORD offset, BYTE * tag, WORD * size) _REENTRANT_ {
	BYTE dbuffer[6];
	bsHandleReadW(handle, (ptr_u)offset, dbuffer, sizeof(dbuffer));
	tag[0] = dbuffer[0];
	return bsGetHeaderLength(dbuffer, size);
}

/*!
******************************************************************************
\section  ptr_u bsFindObject(BSAllocHandle * handle, BYTE mode, BYTE * tag, WORD * size)
\brief    find specified data object for specified purpose
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  mode mode which operation takes place BS_FIND_EMPTY, BS_FIND_TAG, BS_FIND_INDEX, BS_FIND_OFFSET, BS_FIND_ALLOCATED (IN)
\param	  tag 3 bytes pointer/array containing tag to be find/allocated (IN/OUT)		 
\param	  size length of data object, BS_FIND_ALLOCATED total size of allocated data object (IN/OUT)
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

find specified data object for specified purpose

\endverbatim
******************************************************************************
*/
ptr_u bsFindObject(BSAllocHandle * handle, BYTE mode, BYTE * tag, WORD * size) _REENTRANT_ {
	ptr_u offset = handle->start;
	WORD objLen;
	BYTE advHdr;
	BYTE tlvBuffer[6];
	BYTE tbWrt;
	WORD tbCpy;
	//BYTE dbuffer[32];
	union szDumpInfo {
		ptr_u hoffset; 			//fixed size to use ptr_u instead of WORD (2015.12.21)
		struct dByteInfo {
			BYTE tlen;
			WORD len;
		} bInfo;
	} szDump;
	WORD ioffset;
	//void * fhandle = ((BSAbstractionHandle *)handle)->handle;
	//BYTE tlen, len = 0;
	//WORD hoffset = 0;
	szDump.hoffset = 0;
	while((DWORD)offset < (DWORD)handle->end) {
		if(bsHandleReadW(handle, offset, tlvBuffer, sizeof(tlvBuffer)) < 2) return BS_TAG_NOT_FOUND;
		if(memcmp(tlvBuffer, g_endMark, 2) == 0) {
			switch(mode) {
				case BS_FIND_INDEX:
				case BS_FIND_OFFSET:
				case BS_FIND_TAG:
					return BS_TAG_NOT_FOUND;		//object not found
				case BS_FIND_EMPTY:	return bsMarkObject(handle, tag, offset, size[0]);
				case BS_FIND_ALL: return (ptr_u)szDump.bInfo.len;
#if 0
				case BS_FIND_DEFRAG:
					bsHandleWriteW(handle, szDump.hoffset, (BYTE *)g_endMark, 2);
#endif
				case BS_FIND_ALLOCATED: return szDump.hoffset;
			}
		}
		advHdr = bsGetHeaderLength(tlvBuffer, &objLen);
		tbCpy = (advHdr + (WORD)objLen);
		switch(mode) {		
			case BS_FIND_TAG:
				if(bsTagValue(tlvBuffer) == bsTagValue(tag)) {		//matched tag
					size[0] = objLen;
					return offset;
				}
				break;
			case BS_FIND_OFFSET:
				if((offset + advHdr) == (ptr_u)tag) {
					size[0] = objLen;
					return offset;	
				}
				break;
			case BS_FIND_EMPTY:	 
				if(tlvBuffer[0] == 0) {		//empty tag	
					if(objLen >= size[0]) {
						mark_empty_object:
						if((DWORD)(szDump.hoffset = bsMarkObject(handle, tag, offset, size[0])) != ((DWORD)-1)) return (ptr_u)szDump.hoffset; 
					}
					//check if (next_chunk == end_chunk), added 2016.07.30
					if(bsHandleReadW(handle, offset + tbCpy, tlvBuffer, sizeof(tlvBuffer)) < 2) break;
					if(memcpy(tlvBuffer, g_endMark, 2) == 0) {
						bsHandleWriteW(handle, offset, (uint8 *)g_endMark, 2);		//shift end mark first
						offset = handle->start;
						continue;											//fixed 2017.05.04, re-iterate chunk
						//goto mark_empty_object;
					}
				}
				break;
			case BS_FIND_ALL:
				if(tlvBuffer[0] != 0) {
					szDump.bInfo.tlen = bsTagLength(tlvBuffer);
					memcpy(tag + szDump.bInfo.len, tlvBuffer, szDump.bInfo.tlen);
					szDump.bInfo.len += szDump.bInfo.tlen;
				}
				break;
			case BS_FIND_ALLOCATED:
				if(tlvBuffer[0] != 0) {
					szDump.hoffset += tbCpy;
				}
				break;
			case BS_FIND_INDEX:
				if(szDump.hoffset == (WORD)tag) {
					size[0] = objLen;
					return offset;
				}
				szDump.hoffset++;
				break;
#if 0
			case BS_FIND_DEFRAG:
				if(tlvBuffer[0] != 0) {		//free tag
					for(ioffset = 0; ioffset < tbCpy; ioffset += sizeof(dbuffer)) {
						tbWrt = ((ioffset + sizeof(dbuffer)) > tbCpy)?(tbCpy - ioffset):sizeof(dbuffer);
						bsHandleReadW(handle, offset, dbuffer, tbWrt);
						bsHandleWriteW(handle, szDump.hoffset + ioffset, dbuffer, tbWrt); //((BSAbstractionHandle *)handle)->write(fhandle, szDump.hoffset + ioffset, dbuffer, tbWrt);
					}
					szDump.hoffset += tbCpy;
				}
				break;
#endif
			default: break;
		}
		offset += tbCpy;
	}
	return BS_TAG_NOT_FOUND;
}

/*!
******************************************************************************
\section  void bsReleaseObjectList(BSAllocHandle * handle, BYTE * start, BYTE * end)
\brief    release list of data object from specified tag value to specified tag value
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  start pointer to start tag value
\param	  end pointer to end tag value
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

release list of data object from specified tag value to specified tag value

\endverbatim
******************************************************************************
*/
void bsReleaseObjectList(BSAllocHandle * handle, BYTE * start, BYTE * end) _REENTRANT_ {
	ptr_u offset = handle->start;
	ptr_u prevOffset = BS_TAG_NOT_FOUND, dOffset;
	WORD objLen;
	BYTE advHdr, pAdvHdr;
	BYTE tlvBuffer[6];
	BYTE prevTag[3];
	WORD prevLen, chunkLeft;
	WORD tgVal;
	while(1) {
		if(bsHandleReadW(handle, offset, tlvBuffer, sizeof(tlvBuffer)) != sizeof(tlvBuffer)) return;
		if(memcmp(tlvBuffer, g_endMark, 2) == 0) {
			if(prevOffset != BS_TAG_NOT_FOUND && prevTag[0] == 0) {
				bsHandleWriteW(handle, prevOffset, (BYTE *)g_endMark, 2);	
			}
			return;	
		}
		advHdr = (bsGetHeaderLength(tlvBuffer, &objLen));
		dOffset = offset;
		tgVal = bsTagValue(tlvBuffer);
		if(end == NULL) {		   //check for offset
			if(tlvBuffer[0] == 0 || (offset + advHdr) == (ptr_u)offset) goto start_compacting;
		} else {				   //check for tag range
			if(tlvBuffer[0] == 0 || (tgVal >= bsTagValue(start) && tgVal <= bsTagValue(end))) {		//matched tag
			start_compacting:
				if(prevOffset != BS_TAG_NOT_FOUND && prevTag[0] == 0) {
					bsHandleReadW(handle, prevOffset, tlvBuffer, sizeof(tlvBuffer));
					pAdvHdr = (bsGetHeaderLength(tlvBuffer, &prevLen));
					
					tlvBuffer[0] = 0;		//empty tag
					chunkLeft = bsChunkLeft(tlvBuffer, (WORD)((pAdvHdr + (WORD)prevLen) + (advHdr + (WORD)objLen)));
					bsHandleWriteW(handle, prevOffset, tlvBuffer, bsCalcHeaderLength(tlvBuffer, chunkLeft));
					dOffset = prevOffset;	
				} else {
					if(tlvBuffer[0] != 0) {
						tlvBuffer[0] = 0;		//replace tag
						chunkLeft = bsChunkLeft(tlvBuffer, (WORD)(advHdr + (WORD)objLen));
						bsHandleWriteW(handle, offset, tlvBuffer, bsCalcHeaderLength(tlvBuffer, chunkLeft));
					}
				}
			}
		}
		//prevTag = tlvBuffer[0];
		memcpy(prevTag, tlvBuffer, 3);
		prevOffset = dOffset;
		offset += (advHdr + (WORD)objLen);
	}
	return;
}

/*!
******************************************************************************
\section  void bsReleaseByOffset(BSAllocHandle * handle, ptr_u offset)
\brief    release data object by it's TL-DO address (including header)
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  offset address which TL-DO reside (IN)
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

release data object by it's TLV address (including header)

\endverbatim
******************************************************************************
*/
void bsReleaseByOffset(BSAllocHandle * handle, ptr_u offset) _REENTRANT_ {
	BYTE tlvBuffer[6];
	BYTE advHdr;
	WORD objLen;
	static ptr_u poffset;
	if(bsErr(offset)) return;
	poffset = offset;
	if(bsHandleReadW(handle, poffset, tlvBuffer, sizeof(tlvBuffer)) != sizeof(tlvBuffer)) return;	
	advHdr = (bsGetHeaderLength(tlvBuffer, &objLen));
	bsRollbackObject(handle, offset, tlvBuffer, objLen);
}

/*!
******************************************************************************
\section  void bsReleaseByDataOffset(BSAllocHandle * handle, ptr_u dataOffset)
\brief    release data object by it's DO address 
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  offset address which DO (not including header) reside (IN)
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

release data object by it's TLV address (including header)

\endverbatim
******************************************************************************
*/
void bsReleaseByDataOffset(BSAllocHandle * handle, ptr_u dataOffset) _REENTRANT_ {
	bsReleaseObjectList(handle, (BYTE *)dataOffset, NULL);			
}
 
/*!
******************************************************************************
\section  void bsReleaseObject(BSAllocHandle * handle, BYTE * tag)
\brief    release data object by it's tag value
\param 	  handle handle to specified BSAllocHandle (IN)
\param	  tag 3 bytes tag value
\return   header size
\author   AGP
\version  1.0
\date     2016.03.20

\verbatim

release data object by it's tag value

\endverbatim
******************************************************************************
*/
void bsReleaseObject(BSAllocHandle * handle, BYTE * tag) _REENTRANT_ {
 	bsReleaseObjectList(handle, tag, tag);	
}

/*!
******************************************************************************
\section  WORD bsIteratorInit(BSIterator * iterator, BSAllocHandle * handle)
\brief    initialize bootstrap iterator for iterating object through any bootstrapped driver
\param 	  iterator pointer to uninitialized iterator handle	(IN/OUT)
\param	  handle handle to specified bootstrapped driver (IN)
\return   object size (not including tag and length size), zero if not object allocated
\author   AGP
\version  1.0
\date     2016.04.14

\verbatim

initialize bootstrap iterator for iterating object through any bootstrapped driver

\endverbatim
******************************************************************************
*/
WORD bsIteratorInit(BSIterator * iterator, BSAllocHandle * handle) _REENTRANT_ {
	WORD objLen;
	BYTE advHdr;				   
	memcpy(iterator, handle, sizeof(BSAllocHandle));
	if(bsHandleReadW((BSAllocHandleP)iterator, ((BSAllocHandle *)iterator)->start, iterator->tlvHdr, 6) != 6) return 0;
	if(memcmp(iterator->tlvHdr, g_endMark, 2) == 0) return 0;	 
	iterator->current = ((BSAllocHandle *)iterator)->start;
	if(iterator->tlvHdr[0] == 0) {		//check for empty object, then advance iterator;; (fixed 2016.07.30)
		return bsIteratorNext(iterator);
	} else {
		advHdr = bsGetHeaderLength(iterator->tlvHdr, &objLen);
	}
	return objLen;
}
 
/*!
******************************************************************************
\section  WORD bsIteratorNext(BSIterator * iterator)
\brief    iterate next object on collection
\param 	  iterator pointer to initialized iterator handle (IN/OUT)
\return   object size (not including tag and length size), zero if no object left
\author   AGP
\version  1.0
\date     2016.04.14

\verbatim

iterate next object on collection

\endverbatim
******************************************************************************
*/
WORD bsIteratorNext(BSIterator * iterator) _REENTRANT_ {
	WORD objLen;
	BYTE advHdr;
	ptr_u nextPtr;
	do {				 
		//if(bsHandleReadW((BSAllocHandleP)iterator, iterator->current, iterator->tlvHdr, 6) != 6) return 0;
		//if(mmMemCmp(iterator->tlvHdr, g_endMark, 2) == 0) return 0;		//end mark
		advHdr = bsGetHeaderLength(iterator->tlvHdr, &objLen);
		iterator->current += (advHdr + (WORD)objLen);	  
		if(bsHandleReadW((BSAllocHandleP)iterator, iterator->current, iterator->tlvHdr, 6) != 6) return 0;	
		if(memcmp(iterator->tlvHdr, g_endMark, 2) == 0) return 0;		//end mark
	} while(iterator->tlvHdr[0] == 0);
	//get current object length 
	bsGetHeaderLength(iterator->tlvHdr, &objLen);
	return objLen;
}
 
/*!
******************************************************************************
\section  WORD bsIteratorRead(BSIterator * iterator, ptr_u offset, BYTE * buffer, WORD size) 
\brief    read data object from iterator
\param 	  iterator pointer to initialized iterator handle (IN)
\param    offset start address of data to be read (IN)
\param	  buffer pointer to buffer containing the data to be read (OUT)
\param	  size amount of bytes to be read (IN)
\return   amount of bytes readed
\author   AGP
\version  1.0
\date     2016.04.14

\verbatim

read data object from iterator

\endverbatim
******************************************************************************
*/
WORD bsIteratorRead(BSIterator * iterator, ptr_u offset, BYTE * buffer, WORD size) _REENTRANT_ { 
	WORD objLen;
	BYTE advHdr;
   	if(iterator->tlvHdr[0] == 0) return 0;	 
	advHdr = bsGetHeaderLength(iterator->tlvHdr, &objLen);
	return bsHandleReadW((BSAllocHandleP)iterator, iterator->current + advHdr, buffer, size);
}
