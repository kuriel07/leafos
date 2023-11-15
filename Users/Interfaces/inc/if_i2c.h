/*!\file 	if_i2c.h
 * \brief     	i2c driver
 * \details   	i2c driver
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
 
#ifndef IF_I2C__H
#include "defs.h"
#include "config.h"
//#include "stm32f4xx_hal_i2c.h"

typedef struct I2C_HandleTypeDef i2c_context;
typedef struct I2C_HandleTypeDef * i2c_context_p;

#define IF_I2C_DEV_READ		0x01
#define IF_I2C_DEV_WRITE		0x00

uint8 if_i2c_start_read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t timeout, uint32_t tickstart);
uint8 if_i2c_start_write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t timeout, uint32_t tickstart);
uint8 if_i2c_restart(I2C_HandleTypeDef *hi2c, uint8 DevAddress, uint32_t Timeout, uint32_t Tickstart);
uint8 if_i2c_start_mem_read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout, uint32_t Tickstart);
uint8 if_i2c_read(I2C_HandleTypeDef *hi2c, uint8 * result, uint32_t timeout, uint32_t tickstart);
uint8 if_i2c_read_bytes(I2C_HandleTypeDef *hi2c, uint8 * pData, uint8 Size, uint32_t timeout, uint32_t tickstart);
uint8 if_i2c_read_byte(I2C_HandleTypeDef *hi2c);
uint8 if_i2c_write(I2C_HandleTypeDef *hi2c, uint8 value, uint32_t timeout, uint32_t tickstart);
void if_i2c_stop(I2C_HandleTypeDef *hi2c);

#define IF_I2C__H
#endif