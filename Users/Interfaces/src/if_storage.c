#include "defs.h"
#include "config.h"
#include <string.h>
#include "..\inc\if_apis.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif
#include "..\inc\if_storage.h"

//BSP specific headers
#include "stm32f7xx.h"
#include "stm32f7xx_hal_sd.h"				//sd card controller
#include "stm32f7xx_hal_hcd.h"			//usb host controller
#include "efat.h"


dev_node * dev_create_node(uint32 type, char * name, void * driver, 
	void (* open)(void * driver, uchar * path, char * mode),
	void (* seek)(void * driver, unsigned offset, unsigned mode),
	void (* read)(void * driver, unsigned offset, uchar * buffer, size_t size),
	void (* write)(void * driver, unsigned offset, uchar * buffer, size_t size), 
	void (* close)(void  * driver)) {
		dev_node * node = (dev_node *)os_alloc(sizeof(dev_node));
		node->parent = NULL;
		node->child=NULL;
		node->sibling = NULL;
		node->release = os_free;
		node->type = type;
		strncpy(node->name, name, DEV_MAX_NODE_NAME);
		node->open = open;
		node->seek = seek;
		node->read = read;
		node->write = write;
		node->close = close;
		node->driver = driver;
		return node;
}
	
dev_node * dev_get_node_by_name(dev_node * parent, char * name) {
	if(parent == NULL) return NULL;
	dev_node * iterator = parent->child;
	dev_node * temp_node;
	while(iterator != NULL) {
		if(strcmp(iterator->name, name) == 0) return iterator;
		temp_node = dev_get_node_by_name(iterator, name);
		if(temp_node != NULL) return temp_node;
		iterator = iterator->sibling;
	}
	return NULL;
}

dev_node * dev_get_node_by_driver(dev_node * parent, void * driver) {
	if(parent == NULL) return NULL;
	if(driver == NULL) return NULL;
	dev_node * iterator = parent->child;
	dev_node * temp_node;
	while(iterator != NULL) {
		if(iterator->driver == driver) return iterator;
		temp_node = dev_get_node_by_driver(iterator, driver);
		if(temp_node != NULL) return temp_node;
		iterator = iterator->sibling;
	}
	return NULL;
}

void dev_add_node(dev_node * parent, dev_node * node) {
	if(node == NULL) return;
	if(parent == NULL) return;
	dev_node * iterator = parent->child;
	if(node->child == NULL) {
		parent->child = node;
		node->parent = parent;
	} else {
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = node;
		node->parent = parent;
	}
}

void dev_remove_node(dev_node * parent, dev_node * node) {
	if(node == NULL) return;
	if(parent == NULL) return;
	dev_node * iterator = parent->child;
	if(node->child == node) {
		parent->child = node->sibling;
	} else {
		//enumerate child for node
		while(iterator != NULL) {
			if(iterator->sibling == node) {
				iterator->sibling = node->sibling;
				return;
			}
			iterator = iterator->sibling;
		}
	}
}

static void dev_release_node_sibling(dev_node * node) {
	if(node == NULL) return;
	dev_node * iterator = node->child;
	dev_node * to_delete;
	while(iterator != NULL) {
		if(iterator->child != NULL) { 
			dev_release_node(iterator->child);
			iterator->child = NULL;
		}
		to_delete = iterator;
		iterator = iterator->sibling;
		to_delete->sibling = NULL;
		if(to_delete->release != NULL) to_delete->release(to_delete);
	}
}

void dev_release_node(dev_node * node){ 
	if(node == NULL) return;
	if(node->child != NULL) {
		dev_release_node_sibling(node->child);
		node->child = NULL;
	}
	if(node->release != NULL) node->release(node);
}


static SD_HandleTypeDef g_sdcard1;				//handle for sdcard 1 (board specific)
//static MMC_HandleTypeDef g_sdcard1;				//handle for sdcard 1 (board specific)
static HCD_HandleTypeDef g_hcd1;					//handle for hcd 1 (board specific)
char g_sdbuffer[2048] ;

extern void dma_memcpy(void * dst, void * src, size_t sz);

void SDMMC1_IRQHandler(void) {
	//SDMMC1->STA & SDMMC_STA_
  //__HAL_SD_CLEAR_FLAG(&g_sdcard1, SDMMC_STATIC_FLAGS);
  //__HAL_SD_CLEAR_FLAG(&g_sdcard1, SDMMC_STA_RXDAVL);
  //__HAL_SD_CLEAR_FLAG(&g_sdcard1, SDMMC_STA_RXFIFOE);
  //__HAL_SD_CLEAR_FLAG(&g_sdcard1, SDMMC_STA_RXFIFOF);
}

uint8 _sd_finished_read = FALSE;
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
	_sd_finished_read = TRUE;
}

//wrapper for sdio devnode
void if_sdio_readblock(void * interface, unsigned address, uchar * buffer, size_t size) {
	ioman_readblock(interface, buffer, address, size);
}

void if_sdio_writeblock(void * interface, unsigned address, uchar * buffer, size_t size) {
	ioman_writeblock(interface, buffer, address, size);
	
}

void if_storage_init(dev_node * root) {
	//SD_HandleTypeDef * sd_handle;
	uint8 error = 0;
	uint16 rca;
	uint16 i;
	uchar partition_id = 0;
	fat_FileSystem * fs;
	char buffer[20];
	DMA_HandleTypeDef dma_x;
  GPIO_InitTypeDef GPIO_InitStruct;
	HAL_SD_CardStateTypeDef sdstate = 0;
	HAL_SD_CardStatusTypeDef sdstatus ;
	HAL_StatusTypeDef status = HAL_OK;
	HAL_SD_CardCIDTypeDef cardCidInfo;
	HAL_SD_CardCSDTypedef cardinfo;
	SDMMC_CmdInitTypeDef sdCmd;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_SDMMC1_CLK_ENABLE();
	dma_memcpy(g_sdbuffer, g_sdbuffer +512, 512);		//this will initialize and enable dma automatically
	
	memset(g_sdbuffer, 0x00, sizeof(g_sdbuffer));
	NVIC_EnableIRQ(SDMMC1_IRQn);									//enable interrupt
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8;			//SDIO
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = GPIO_PIN_12;					//CLK
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = GPIO_PIN_13;					//Detect
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_2;							//CMD
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	g_sdcard1.Instance = SDMMC1;
  g_sdcard1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  g_sdcard1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  g_sdcard1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  g_sdcard1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  g_sdcard1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  g_sdcard1.Init.ClockDiv = 16;
	
  if (HAL_SD_Init(&g_sdcard1) != HAL_OK)
  {
    //Error_Handler();
		error++;
  }
	sdstate = HAL_SD_GetCardState(&g_sdcard1);
  //if (HAL_SD_ConfigWideBusOperation(&g_sdcard1, SDMMC_BUS_WIDE_4B) != HAL_OK)
  {
    //Error_Handler();
		error++;
  }
	HAL_SD_GetCardStatus(&g_sdcard1, &sdstatus); 
	
	sdstate = HAL_SD_GetCardState(&g_sdcard1);
	status = HAL_SD_GetCardCID(&g_sdcard1, &cardCidInfo);
	status = HAL_SD_GetCardCSD(&g_sdcard1, &cardinfo);
	//HAL_SD_GetCardCSD(&g_sdcard1, 
	
	//status = HAL_SD_GetCardStatus(&g_sdcard1, &sdstatus);
	sdstate = HAL_SD_GetCardState(&g_sdcard1);
	status = SDMMC_CmdSelDesel(g_sdcard1.Instance, g_sdcard1.SdCard.RelCardAdd << 16);
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET); 
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 8*512, 1, 5000);
	//while(_sd_finished_read == FALSE);
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 16*512, 1, 5000);
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 320*512, 1, 5000);
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 2048*512, 1, 5000);
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 4096*512, 1, 5000);
	status = HAL_SD_GetError(&g_sdcard1);
	HAL_Delay(1000);
	//status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 0, 512, 500);
	//status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 200, 512, 500);
	//status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 0, 4, 200);
	
	status = HAL_SD_ReadBlocks(&g_sdcard1, (uint8_t *)g_sdbuffer, 0, 4, 200);
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET); 
	status = HAL_SD_GetError(&g_sdcard1);
	status = SDMMC_CmdSelDesel(g_sdcard1.Instance, 0);
	
	//create device node for SDIO
	dev_node * sdnode = dev_create_node(DEV_TYPE_SDIO, "SDIO", &g_sdcard1, NULL, NULL, if_sdio_readblock, if_sdio_writeblock, NULL);
	dev_add_node(root, sdnode);
	//try initialize partition 0 to 3 (max 4 partition)
	for(partition_id=0;partition_id<4;partition_id++) {
		fs = fat_Init(partition_id, &g_sdcard1);
		if(fs != NULL) {
			snprintf(buffer, 20, "SDCARD%i", partition_id);
			dev_node * pnode = dev_create_node(DEV_TYPE_SDCARD , buffer, fs, NULL, NULL, NULL, NULL, NULL);
			dev_add_node(sdnode, pnode);
		}
	}
	
	//SDMMC_CmdSelDesel(g_sdcard1.Instance, 0);			//SELECT ADDRESS 0
}


uchar ioman_init(void * interface) {
	//dev_node * root = (dev_node *)interface;
	//root->driver = &g_sdcard1;
	return TRUE;
}


void ioman_readblock(void * interface, uchar * buffer, unsigned address, size_t size) {
	HAL_StatusTypeDef status = HAL_OK;
	SD_HandleTypeDef * sdcard = (SD_HandleTypeDef *)interface;
	uint32 psw;
	psw = os_enter_critical();

	status = SDMMC_CmdSelDesel(sdcard->Instance, g_sdcard1.SdCard.RelCardAdd << 16);
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET); 
	status = HAL_SD_ReadBlocks(sdcard, (uint8_t *)buffer, address, size/512, 5000);
	status = SDMMC_CmdSelDesel(g_sdcard1.Instance, 0);
	os_exit_critical(psw);
}

void ioman_writeblock(void * interface, uchar * buffer, unsigned address, size_t size) {
	HAL_StatusTypeDef status = HAL_OK;
	SD_HandleTypeDef * sdcard = (SD_HandleTypeDef *)interface;
	uint32 psw;
	psw = os_enter_critical();
	status = SDMMC_CmdSelDesel(sdcard->Instance, g_sdcard1.SdCard.RelCardAdd << 16);
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET); 
	status = HAL_SD_WriteBlocks(sdcard, (uint8_t *)buffer, address, size/512, 5000);
	status = SDMMC_CmdSelDesel(g_sdcard1.Instance, 0);
	os_exit_critical(psw);

}