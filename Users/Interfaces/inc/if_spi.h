/*!\file 			if_spi.h
 * \brief     	spi driver
 * \details   	spi driver
 * \author    	AGP
 * \version   	1.0
 * \date      	November 2017
 * \pre       	
 * initial release
\verbatim	
********************************************************************
1.0
 * initial release (2017.11.25)
********************************************************************
\endverbatim
 */

#ifndef IF_SPI__H
#include "..\..\defs.h"
#include "..\..\config.h"

#define SPI_STATE_LSB_FIRST	0x01
#define SPI_STATE_MSB_FIRST	0x00

typedef struct spi_context {
	void * handle;		//can be GPIO_TypeDef or I2C_HandleTypeDef
	uint16 miso;
	uint16 mosi;
	uint16 sck;
	uint16 cs;
	void (* select)(void * ctx);
	void (* deselect)(void * ctx);
	void (* reset)(void * ctx);
	uint8 t_state;
} spi_context;

typedef struct spi_context * spi_context_p;

void if_spi_write(spi_context * ctx, uint8_t num);
uint8 if_spi_read(spi_context * ctx, uint8 add);


#define IF_SPI__H
#endif
