/*!\file 		defs.h
 * \brief     	global definition header for all module
 * \details   	useful when changing to different system and retaining previous system configuration
 * \author    	AGP
 * \version   	1.0
 * \date      	2014
 */

#ifndef _DEFS__H
#include <stdio.h>
//compiler warning disable
//#pragma warning(disable : 4996)
//#pragma warning(disable : 4761)
//#pragma warning(disable : 4047)
//#pragma warning(disable : 4715)

#define TRUE			1								/*!< default true constant */
#define FALSE			0								/*!< default false constant */

#ifndef NULL
#define NULL						0UL				   	/*!< default null constant */
#endif

#define _REENTRANT_					//reentrant				 	/*!< force to use reentrant function (keil C51) */
#define _FORCE_REENTRANT_			//reentrant
#define _ALIGN4_									/*!< align databytes in 4 byte boundary (compiler support) */
#define _ALIGN2_			
#define _ALIGN1_									/*!< force struct/union to 1 byte boundary */
#define __STATIC			static
		   
#define CONST				const		//code
#define DYNAMIC						//xdata
typedef unsigned long       DWORD;
typedef unsigned int         BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#define FID_MF						0x3F00	//Master File
#define FID_ADF						0x7FFF	//current ADF
#define FID_ATR						0x2F01	//EF ATR
#define FID_CARDMAN					0x8F01	//Card Manager ADF   ->directory
#define FID_LOCAL					0x7F69	//System Directory (0x69 = i)	->directory
#define FID_SMDIR					0x5F20	//OTA temporary	  ->directory
#define FID_GSM						0x7F20
#define FID_TELECOM					0x7F10

#define FID_MSG_BUF					0x6FB0	//message buffer (0348 decoded content)
#define FID_0348_KEY				0x6F48	//0348 keylist (linier fixed)	 
#define FID_AUTHKEY					0x6F88	//EF_AuthKey (opc, ki, amf) -> transparent 
#define FID_SQNLIST					0x6F40	//EF_SQNList (total 32 records, each 6 bytes) ->linier fixed  
#define FID_SMS_OUT					0x6F20	//queued SMS packets (linier fixed)
#define FID_LTAR					0x6F03	//TAR list 	(6 bytes per-record) ->linier fixed
#define FID_LCNTR					0x6F09	//CNTR list (7 bytes per-record) ->linier fixed

#define FID_WIBKEY					0x6FE8	//EF_WIBkey

/* default configuration tags */
#define DCT_KI						0x10	//KI value
#define DCT_OP						0x11	//OP value	
#define DCT_OPC						0x12	//OPC value
#define DCT_AMF						0x13	//AMF value
#define DCT_COMP128					0x21	//COMP128 mode

#define XASSERT(x)					(x)
#define TEXT(x)						#x
#define MACH_LITTLE_ENDIAN		1

#if MACH_LITTLE_ENDIAN
#define end_swap32(x)		( ((x&0xff000000) >> 24) | ((x&0x00ff0000) >> 8) | ((x&0x0000ff00) << 8) | ((x & 0xff) << 24) )		 /*!< swap endianess 32 bit (big-little) */
#define end_swap16(x)		( ((x&0xff00) >> 8) | ((x&0x00ff) << 8) )															 /*!< swap endianess 16 bit (big-little) */

#else													
#define end_swap32(x)		(x)							/*!< swap endianess 32 bit (big-little) */
#define end_swap16(x)		(x)							/*!< swap endianess 16 bit (big-little) */

#endif

//type definition
typedef signed int int32;
typedef long eint32;
typedef unsigned int uint32;
typedef unsigned long euint32;
typedef unsigned long ulong;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned short euint16;
typedef unsigned short uint;
typedef unsigned char uchar;
typedef char eint8;
typedef unsigned char euint8;
typedef signed char int8;
typedef unsigned char uint8;
typedef unsigned long ulong_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef unsigned char uchar_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
//c = constant/code
typedef CONST unsigned char uint8_c;	//taruh di code program
typedef CONST unsigned int uint16_c;	//taruh di code program
typedef CONST unsigned long uint32_c;	//taruh di code program
typedef CONST char int8_c;	//taruh di code program  
typedef CONST int int16_c;	//taruh di code program
typedef CONST long int32_c;	//taruh di code program
//typedef CONST unsigned char uint8_t;	//taruh di code program
//typedef CONST unsigned int uint16_t;	//taruh di code program
//typedef CONST unsigned long uint32_t;	//taruh di code program
//typedef CONST char int8_t;	//taruh di code program  
//typedef CONST int int16_t;	//taruh di code program
//typedef CONST long int32_t;	//taruh di code program
//v = volatile/variable
typedef DYNAMIC unsigned char uint8_v;
typedef DYNAMIC unsigned short uint16_v;
typedef DYNAMIC unsigned long uint32_v;  
typedef DYNAMIC char int8_v;
typedef DYNAMIC short int16_v;
typedef DYNAMIC long int32_v;	
//unsigned/universal pointer (either volatile/non-volatile)
typedef uint32 	ptr_u;			//typedef cast untuk pointer, diganti menyesuaikan target hardware
//volatile pointer 
typedef uint8_v * ptr_v;			//typedef cast untuk pointer, diganti menyesuaikan target hardware
//code pointer	
typedef uint8_c * ptr_c;			//typedef cast untuk pointer, diganti menyesuaikan target hardware
//@dir unsigned long * @dir zpage_ptr_to_zero_page;	
typedef signed int 		ssize_t;
typedef unsigned int	time_t;


struct datetime {
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 day;
	uint8 date;
	uint8 month;
	uint16 year;	
	uint16 days;			//days of year
	int8 tz;				//timezone
};
typedef struct datetime datetime;
typedef struct datetime * datetime_p;

//safe alloc/dealloc
extern void * os_alloc(size_t size);
extern void os_free(void * ptr);
extern void * os_realloc(void * ptr, size_t size);

#define malloc 		os_alloc
#define free		os_free

#define _DEFS__H
#endif
