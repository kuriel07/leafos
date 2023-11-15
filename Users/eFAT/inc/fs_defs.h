/***************************************************************/
/*eFAT : elvish FAT Library                                    */
/*Filename : defs.h                                            */
/*Copyright (C) 2011 Agus Purwanto                             */
/*http://www.my.opera.com/kuriel                               */
/***************************************************************/
#ifndef _DEFS_DEFINED
#define _DMA_DEBUG 		0
#if _DMA_DEBUG
#define	HEAP_CALC	 	1	 /* Should be made 0 to turn OFF debugging */
#endif

#ifndef NULL	//cek apakah NULL sudah pernah didefine sebelummnya
#define NULL						0
#endif

#define TRUE 	1   
#define FALSE 	0
#define OK		1
#define FAIL	0

#define ESC_KEY	('q')	// 0x1b
//primitive data type definition

#define _LARGEFILE64_SOURCE 0
#define _FILE_OFFSET_BITS	0
#define __GNUC__			0
#define __GNUC_MINOR__		0

//Use Midgard MM for memory allocation to prevent collision
//#define malloc				m_alloc
//#define free				m_free
#define _DEFS_DEFINED
#endif
