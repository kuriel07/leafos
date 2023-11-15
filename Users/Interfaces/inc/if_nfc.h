/*!\file 		if_nfc.h
 * \brief     	nfc driver
 * \details   	nfc driver
 * \author    	AGP
 * \version   	1.0
 * \date      	June 2017
 * \pre       	
 * initial release
\verbatim	
********************************************************************
1.0
 * initial release, only support DEP for both host and target comm (2017.09.18)
 * changed to I2C interface (2017.11.24)
********************************************************************
\endverbatim
 */
 
#ifndef IF_NFC__H
#include "defs.h"
#include "if_rfid.h"

//comm selection
#define PN532_COMM_I2C								0x03
#define PN532_COMM_SPI								0x02
#define PN532_COMM_AUTO								0x0F
#define PN532_COMM_INTERFACE					PN532_COMM_AUTO		//PN532_COMM_I2C

//device i2c address
#define PN532_I2C_ADDR								0x48
#define PN532_I2C_READ								0x01
// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)


#define PN532_MIFARE_ISO14443A              (0x00)		//106Kbps Mifare card
#define PN532_FELICA_1							0x01			//212Kbps felica card
#define PN532_FELICA_2							0x02			//424Kbps felica card

#define NFC_MIFARE									PN532_MIFARE_ISO14443A
#define NFC_FELICA_1								PN532_FELICA_1
#define NFC_FELICA_2								PN532_FELICA_2
#define NFC_ISO14443B							3
#define NFC_JEWEL									4
#define NFC_DEP										7

#define NFC_STATE_IDLE							0x00
#define NFC_STATE_INITIALIZED					0x80
#define NFC_STATE_ENABLED						0x40
#define NFC_STATE_CONNECTED				0x01
#define NFC_STATE_OPENED						0x02
#define NFC_STATE_BUSY							0x03

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_WRITE_ULTRALIGHT         (0xA2)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)

// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE                 (0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT          (0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT         (0x02)
#define NDEF_URIPREFIX_HTTP                 (0x03)
#define NDEF_URIPREFIX_HTTPS                (0x04)
#define NDEF_URIPREFIX_TEL                  (0x05)
#define NDEF_URIPREFIX_MAILTO               (0x06)
#define NDEF_URIPREFIX_FTP_ANONAT           (0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT           (0x08)
#define NDEF_URIPREFIX_FTPS                 (0x09)
#define NDEF_URIPREFIX_SFTP                 (0x0A)
#define NDEF_URIPREFIX_SMB                  (0x0B)
#define NDEF_URIPREFIX_NFS                  (0x0C)
#define NDEF_URIPREFIX_FTP                  (0x0D)
#define NDEF_URIPREFIX_DAV                  (0x0E)
#define NDEF_URIPREFIX_NEWS                 (0x0F)
#define NDEF_URIPREFIX_TELNET               (0x10)
#define NDEF_URIPREFIX_IMAP                 (0x11)
#define NDEF_URIPREFIX_RTSP                 (0x12)
#define NDEF_URIPREFIX_URN                  (0x13)
#define NDEF_URIPREFIX_POP                  (0x14)
#define NDEF_URIPREFIX_SIP                  (0x15)
#define NDEF_URIPREFIX_SIPS                 (0x16)
#define NDEF_URIPREFIX_TFTP                 (0x17)
#define NDEF_URIPREFIX_BTSPP                (0x18)
#define NDEF_URIPREFIX_BTL2CAP              (0x19)
#define NDEF_URIPREFIX_BTGOEP               (0x1A)
#define NDEF_URIPREFIX_TCPOBEX              (0x1B)
#define NDEF_URIPREFIX_IRDAOBEX             (0x1C)
#define NDEF_URIPREFIX_FILE                 (0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID           (0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG          (0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT          (0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW          (0x21)
#define NDEF_URIPREFIX_URN_EPC              (0x22)
#define NDEF_URIPREFIX_URN_NFC              (0x23)

#define PN532_GPIO_VALIDATIONBIT            (0x80)
#define PN532_GPIO_P30                      (0)
#define PN532_GPIO_P31                      (1)
#define PN532_GPIO_P32                      (2)
#define PN532_GPIO_P33                      (3)
#define PN532_GPIO_P34                      (4)
#define PN532_GPIO_P35                      (5)

#define PN532_RF_DISABLE					0x00
#define PN532_RF_ENABLE						0x01
#define PN532_RFCA_ENABLE					0x02

#define PN532_SAM_NORMAL					0x01

#define PN532_PARAM_NAD					0x01
#define PN532_PARAM_DID						0x02
#define PN532_PARAM_ATR_RES				0x04
#define PN532_PARAM_RATS					0x10
#define PN532_PARAM_PICC					0x20
#define PN532_PARAM_NO_PREAMBLE		0x40

#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)
#define PN532_DEFAULT_INITIATOR		(uint8 *)"\x00\xFF\xFF\x00\x00"

#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)

#define PN532_ACK_WAIT_TIME           (10)  // ms, timeout of waiting for ACK
#define PN532_DEFAULT_TIMEOUT		1000

#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

#define REVERSE_BITS_ORDER(b)         b = (b & 0xF0) >> 4 | (b & 0x0F) << 4; \
                                      b = (b & 0xCC) >> 2 | (b & 0x33) << 2; \
                                      b = (b & 0xAA) >> 1 | (b & 0x55) << 1
									  

#define NFC_BUSY(ctx)						(ctx->nfc_state & NFC_STATE_OPENED)
#define NFC_LOCK(ctx)						((ctx->nfc_state |= NFC_STATE_OPENED))
#define NFC_UNLOCK(ctx)					((ctx->nfc_state &= ~NFC_STATE_OPENED))

typedef struct nfc_context
{
	union {
	I2C_HandleTypeDef i2c_base;
	rf_context spi_base;
	} base;
	uint8 comm_interface;
    void (* begin)(struct nfc_context * ctx);
    void (* wakeup)(struct nfc_context * ctx);
    int8 (* writeCommand)(struct nfc_context * ctx, uint8 *header, uint8 hlen, uint8 *body, uint8 blen);
    int16 (* readResponse)(struct nfc_context * ctx, uint8 buf[], uint16 len, uint16 timeout);
	uint8 command;
	uint8 nfc_state;
	uint8 type;		//type of card
	uint8 uid[7];  // ISO14443A uid
    uint8 uidLen;  // uid len
    uint8 key[6];  // Mifare Classic key
    uint8 inListedTag; // Tg number of inlisted tag.
} nfc_context;

typedef struct nfc_context * nfc_context_p;


uint8 if_nfc_init(nfc_context_p ctx) ;
uint8 if_nfc_sleep(nfc_context_p ctx);
void if_nfc_begin(nfc_context_p ctx);
uint32_t if_nfc_get_firmware_version(nfc_context_p ctx);
uint8 if_nfc_write_gpio(nfc_context_p ctx, uint8_t pinstate);
uint8_t if_nfc_read_gpio(nfc_context_p ctx);
uint8 if_nfc_sam_config(nfc_context_p ctx, uint8 mode);
uint8 if_nfc_set_rf_field(nfc_context_p ctx, uint8 mode);
uint8 if_nfc_set_timing(nfc_context_p ctx, uint8_t atr_timeout, uint8 retry_timeout);
uint8 if_nfc_set_max_retry_com(nfc_context_p ctx, uint8 value);
uint8 if_nfc_set_passive_activation_retries(nfc_context_p ctx, uint8_t maxRetries);
void if_nfc_set_parameters(nfc_context_p ctx, uint8 value);

//host APIs
uint8 if_nfc_read_passive_target_id(nfc_context_p ctx, uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout, uint8 inlist);
uint8 if_nfc_mfc_is_first_block (nfc_context_p ctx, uint32_t uiBlock);
uint8 if_nfc_mfc_is_trailer_block (nfc_context_p ctx, uint32_t uiBlock);
uint8_t if_nfc_mfc_authenticate (nfc_context_p ctx, uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData);
uint8_t if_nfc_mfc_read_data_block (nfc_context_p ctx, uint8_t blockNumber, uint8_t *data);
uint8_t if_nfc_mfc_write_data_block (nfc_context_p ctx, uint8_t blockNumber, uint8_t *data);
uint8_t if_nfc_mfc_format_ndef (nfc_context_p ctx);
uint8_t if_nfc_mfc_write_ndefuri (nfc_context_p ctx, uint8_t sectorNumber, uint8_t uriIdentifier, const char *url);
uint8_t if_nfc_mful_read_page (nfc_context_p ctx, uint8_t page, uint8_t *buffer);
uint8_t if_nfc_mful_write_page (nfc_context_p ctx, uint8_t page, uint8_t *buffer);
uint8 if_nfc_data_exchange(nfc_context_p ctx, uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength);
uint8 if_nfc_list_passive_target(nfc_context_p ctx, uint8 maxtg, uint8 type, uint8 * initiatorData, uint8 len);
uint8 if_nfc_get_atr(nfc_context_p ctx, uint8 * atr);
uint8 if_nfc_select(nfc_context_p ctx, uint8 logical_id);
int8_t if_nfc_init_picc(nfc_context_p ctx);
int8_t if_nfc_init_dep(nfc_context_p ctx);
int8_t if_nfc_start_dep(nfc_context_p ctx, uint8 active, uint8 baudrate, uint8 * initiator);
int8_t if_nfc_start_psl(nfc_context_p ctx, uint8 active, uint8 baudrate, uint8 * initiator, uint8 ilen) ;
uint8 if_nfc_open(nfc_context_p ctx);
void if_nfc_close(nfc_context_p ctx);

//target APIs
int8_t if_tag_init_as(nfc_context_p ctx, uint8_t* command, const uint8_t len, const uint16_t timeout);
int8_t if_tag_init_dep(nfc_context_p ctx);
int8_t if_tag_connect_dep(nfc_context_p ctx);
int8_t if_tag_init_picc(nfc_context_p ctx);
int8_t if_tag_connect_picc(nfc_context_p ctx);
uint8 if_tag_get_state(nfc_context_p ctx);
int16_t if_tag_get_data(nfc_context_p ctx, uint8_t *buf, uint16_t len);
uint8 if_tag_set_data(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen);
int16_t if_tag_release(nfc_context_p ctx, const uint8_t relevantTarget);
int16_t if_tag_get_command(nfc_context_p ctx, uint8_t *buf, uint8_t len);
uint8 if_tag_set_response(nfc_context_p ctx, uint8_t *buf, uint8_t len);

#define IF_NFC__H
#endif
