/*!\file 		if_rfid.h
 * \brief     	module for accessing RFID based smart card
 * \details   	module for accessing RFID based smart card based on RC522
 * \author    	AGP
 * \version   	1.0
 * \date      	2017
 * \pre       	
 * derived from https://github.com/miguelbalboa/rfid
\verbatim	
********************************************************************
1.0
 * initial release (started 2017.06.12)
********************************************************************
\endverbatim
 */


#ifndef _IF_RFID__H
#include "..\..\defs.h"
#include "..\..\config.h"
#include "if_spi.h"

#define RFID_CMD_IDLE			0x00
#define RFID_CMD_MEM			0x01
#define RFID_CMD_RECV		0x08
#define RFID_CMD_XRCV		0x0C
#define RFID_CMD_XMIT			0x04
#define RFID_CMD_CCRC		0x03
#define RFID_CMD_REST		0x0F

#define RFID_REG_CMD			0x01
#define RFID_REG_CIEN			0x02
#define RFID_REG_CIRQ			0x04
#define RFID_REG_DIRQ			0x05
#define RFID_REG_ERR			0x06
#define RFID_REG_STS1			0x07
#define RFID_REG_STS2			0x08
#define RFID_REG_DATA			0x09
#define RFID_REG_DLEN			0x0A
#define RFID_REG_CTRL			0x0C
#define RFID_REG_BFRM		0x0D
#define RFID_REG_COLS		0x0E
#define RFID_REG_CRCH		0x21
#define RFID_REG_CRCL			0x22

#define	PICC_CMD_REQA				0x26	// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define	PICC_CMD_WUPA				0x52		// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define	PICC_CMD_CT					0x88		// Cascade Tag. Not really a command, but used during anti collision.
#define	PICC_CMD_SEL_CL1			0x93		// Anti collision/Select, Cascade Level 1
#define	PICC_CMD_SEL_CL2			0x95		// Anti collision/Select, Cascade Level 2
#define	PICC_CMD_SEL_CL3			0x97		// Anti collision/Select, Cascade Level 3
#define	PICC_CMD_HLTA				0x50		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
#define	PICC_CMD_RATS           	0xE0     // Request command for Answer To Reset.
	// The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
	// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
	// The read/write commands can also be used for MIFARE Ultralight.
#define	PICC_CMD_MF_AUTH_KEY_A		0x60		// Perform authentication with Key A
#define	PICC_CMD_MF_AUTH_KEY_B		0x61		// Perform authentication with Key B
#define	PICC_CMD_MF_READ					0x30		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define	PICC_CMD_MF_WRITE				0xA0		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define	PICC_CMD_MF_DECREMENT		0xC0,	// Decrements the contents of a block and stores the result in the internal data register.
#define	PICC_CMD_MF_INCREMENT			0xC1,	// Increments the contents of a block and stores the result in the internal data register.
#define	PICC_CMD_MF_RESTORE			0xC2		// Reads the contents of a block into the internal data register.
#define	PICC_CMD_MF_TRANSFER			0xB0	// Writes the contents of the internal data register to a block.
			// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
	// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
#define	PICC_CMD_UL_WRITE				0xA2		// Writes one 4 byte page to the PICC.

//type of PICC
#define PICC_TYPE_NOT_COMPLETE		0x04//;	// UID not complete
#define PICC_TYPE_MIFARE_MINI				0x09//;
#define PICC_TYPE_MIFARE_1K				0x08//;
#define PICC_TYPE_MIFARE_4K				0x18//;
#define PICC_TYPE_MIFARE_UL				0x00//;
#define PICC_TYPE_MIFARE_PLUS			0x10	//;
#define PICC_TYPE_TNP3XXX					0x01	//;
#define PICC_TYPE_ISO_14443_4				0x20		//;
#define PICC_TYPE_ISO_18092				0x40		//;

#define RF_STATE_INITIALIZED				0x8000
#define RF_STATE_CONNECTED				0x0001
#define RF_STATE_OPENED					0x0002

// A struct used for passing the UID of a PICC.
typedef struct {
	uint8		size;			// Number of bytes in the UID. 4, 7 or 10.
	uint8		uidByte[10];
	uint8		sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
} picc_uid;

typedef struct rf_context {
	spi_context base;
	uint8 command;
	uint16 state;
	picc_uid uid;
} rf_context;

typedef struct rf_context * rf_context_p;

void if_spi_init(rf_context_p ctx, uint8 state);

uint8 if_rfid_init(rf_context * ctx) ;
void if_rfid_reset(rf_context * ctx);
uint8 if_rfid_present(rf_context * ctx);					
uint8 if_rfid_wakeup(rf_context * ctx);
uint8 if_rfid_selftest(rf_context * ctx);
uint8 if_rfid_wupa_reqa(rf_context * ctx, uint8 command, uint8 *bufferATQA, uint8 *bufferSize );
uint8 if_rfid_transceive(rf_context * ctx, uint8 *sendData, uint8 sendLen, uint8 *backData, uint8 *backLen, uint8 *validBits );
uint8 if_rfid_calc_crc(rf_context *ctx,	uint8 *data,	 uint8 length, uint8 *result) ;
uint8 if_rfid_comm_picc(rf_context * ctx, uint8 command, uint8 waitIRq, uint8 *sendData, uint8 sendLen, uint8 *backData, uint8 *backLen, uint8 *validBits, uint8 rxAlign );
//picc (Mifare) APIs
uint8 if_picc_connect(rf_context * ctx, uint8 * atr);
uint8 if_picc_select(rf_context * ctx, picc_uid *uid, uint8 validBits); 
uint8 if_picc_transceive(rf_context * ctx, uint8 *sendData, uint8 sendLen, uint8 acceptTimeout );
uint8 if_picc_read(rf_context * ctx, uint8 blockAddr,  uint8 *buffer, uint8 *bufferSize ) ;
uint8 if_picc_write(rf_context * ctx, uint8 blockAddr, uint8 *buffer, uint8 bufferSize );
uint8 if_picc_authenticate(rf_context * ctx, uint8 command, uint8 blockAddr, uint8 *key); 


#define _IF_RFID__H
#endif
