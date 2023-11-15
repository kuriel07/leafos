#ifndef IF_DEVICE__H
#define IF_DEVICE__H

#include "../../defs.h"

#define DEV_MAX_NODE_NAME			16
typedef struct dev_node dev_node;
typedef struct dev_node * dev_node_p;

#define DEV_TYPE_MASK						0xF0000000
#define DEV_SUBTYPE_MASK				0x0FF00000
#define DEV_TYPE_NONE						0x00000000
#define DEV_TYPE_STORAGE				0x40000000	
#define DEV_TYPE_IO							0x10000000
#define DEV_TYPE_HDD						(DEV_TYPE_STORAGE | 0x0100000)
#define DEV_TYPE_SDCARD					(DEV_TYPE_STORAGE | 0x0200000)
#define DEV_TYPE_FDD						(DEV_TYPE_STORAGE | 0x0300000)
#define DEV_TYPE_CDROM					(DEV_TYPE_STORAGE | 0x0400000)
#define DEV_TYPE_DVDROM					(DEV_TYPE_STORAGE | 0x0500000)
#define DEV_TYPE_AUDIO_IN				(DEV_TYPE_IO | 0x0100000)
#define DEV_TYPE_AUDIO_OUT			(DEV_TYPE_IO | 0x0200000)
#define DEV_TYPE_HID						(DEV_TYPE_IO | 0x0300000)
#define DEV_TYPE_GPIO						(DEV_TYPE_IO | 0x0400000)
#define DEV_TYPE_USBD						(DEV_TYPE_IO | 0x0500000)
#define DEV_TYPE_ETHERNET				(DEV_TYPE_IO | 0x0600000)
#define DEV_TYPE_SDIO						(DEV_TYPE_IO | 0x0700000)
#define DEV_TYPE_SATA						(DEV_TYPE_IO | 0x0800000)
#define DEV_TYPE_USBH						(DEV_TYPE_IO | 0x0900000)


#define CMD_DATBYTE		0
#define CMD_DATHW		1
#define CMD_DATWORD		2
#define CMD_LONGRES		4		//long response
#define CMD_NORES		8		//no response
#define CMD_DATA		16		//with data
#define CMD_DATBLOCK	32		//transmit block
#define CMD_DATWIDE		64		//wide bus
#define CMD_DATXMIT		128		//transmit
#define CMD_SHORTRES	0

#define GO_IDLE_STATE		0	//CMD0
#define SEND_OP_COND		1	//CMD1
#define ALL_SEND_CID		2	//CMD2
#define SEND_RELATIVE_ADDR	3	//CMD3
#define SET_DSR				4	//CMD4
#define IO_SEND_OP_COND		5	//CMD5
#define SELECT_CARD			7	//CMD7
#define SD_READ_CSD			9	//CMD9
#define SD_READ_CID			10	//CMD10
#define STOP_TRANSMISSION	12	//CMD12
#define SD_READ_STATUS		13	//CMD13
#define GO_INACTIVE_STATE	15	//CMD15
#define SET_BLOCKLEN		16	//CMD16
#define READ_SINGLE_BLOCK	17	//CMD17
#define WRITE_SINGLE_BLOCK	24	//CMD24
#define APP_CMD				55	//CMD55
#define SD_STATUS			13	//ACMD13
#define SD_APP_OP_COND		41	//ACMD41
#define SD_READ_OCR			58	//OCR opearating condition register


//use device tree to represent each media storage, each device node contain specific driver and operation
struct dev_node {
	struct dev_node * parent;
	struct dev_node * sibling;
	struct dev_node * child;
	uint32 type;
	char name[DEV_MAX_NODE_NAME];
	void (* release)(void *);
	void * driver;							//driver handle
	void (* open)(void * driver, uchar * path, char * mode);
	void (* seek)(void * driver, unsigned offset, unsigned mode);
	void (* read)(void * driver, unsigned offset, uchar * buffer, size_t size);
	void (* write)(void * driver, unsigned offset, uchar * buffer, size_t size);
	void (* close)(void  * driver);
};


dev_node * dev_create_node(uint32 type, char * name, void * driver, 
	void (* open)(void * driver, uchar * path, char * mode),
	void (* seek)(void * driver, unsigned offset, unsigned mode),
	void (* read)(void * driver, unsigned offset, uchar * buffer, size_t size),
	void (* write)(void * driver, unsigned offset, uchar * buffer, size_t size), 
	void (* close)(void  * driver));
dev_node * dev_get_node_by_name(dev_node * parent, char * name);
dev_node * dev_get_node_by_driver(dev_node * parent, void * driver);
void dev_add_node(dev_node * parent, dev_node * node);
void dev_remove_node(dev_node * parent, dev_node * node);
void dev_release_node(dev_node * node);	


void if_storage_init(dev_node * root);


#endif