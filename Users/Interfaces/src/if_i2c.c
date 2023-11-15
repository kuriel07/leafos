#include "defs.h"
#include "config.h"
#include "if_apis.h"
#include <string.h>
#include "if_nfc.h"
#include "if_rfid.h"
#include "os.h"
#include "os_core.h"
//#include "stm32f4xx_hal_i2c.h"
#ifdef STM32F4

#define I2C_START(x)		((I2C_HandleTypeDef *)x)->Instance->CR1 |= I2C_CR1_START
#define I2C_START_READ(x)	((I2C_HandleTypeDef *)x)->Instance->CR1 |= I2C_CR1_START
#define I2C_STOP(x)			((I2C_HandleTypeDef *)x)->Instance->CR1 |= I2C_CR1_STOP
#define I2C_ACK(x)			((I2C_HandleTypeDef *)x)->Instance->CR1 &= ~I2C_CR1_POS
#define I2C_WRITE(x, dat)	((I2C_HandleTypeDef *)x)->Instance->DR = dat
#define I2C_READ(x)			((I2C_HandleTypeDef *)x)->Instance->DR
#define I2C_DIS_ACK(x)		((I2C_HandleTypeDef *)x)->Instance->CR1 &= ~I2C_CR1_ACK
#define I2C_EN_ACK(x)		((I2C_HandleTypeDef *)x)->Instance->CR1 |= I2C_CR1_ACK
#define I2C_WAIT_SB(x)				\
  if(I2C_WaitOnFlagUntilTimeout(x, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)	\
  {	\
    return HAL_TIMEOUT;\
  }
#define I2C_WAIT_ADDR10(x)			\
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(x, I2C_FLAG_ADD10, Timeout, Tickstart) != HAL_OK)\
    {\
      if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)\
      {\
        return HAL_ERROR;\
      }\
      else\
      {\
        return HAL_TIMEOUT;\
      }\
    }
	
#endif

#ifdef STM32F7

#define I2C_START(x)		((I2C_HandleTypeDef *)x)->Instance->CR2 |= I2C_CR2_START
#define I2C_START_READ(x)	((I2C_HandleTypeDef *)x)->Instance->CR1 |= (I2C_CR2_START | I2C_CR2_RD_WRN)
#define I2C_STOP(x)			((I2C_HandleTypeDef *)x)->Instance->CR2 |= I2C_CR2_STOP
#define I2C_ACK(x)			((I2C_HandleTypeDef *)x)->Instance->CR2 &= ~I2C_CR2_NACK
#define I2C_WRITE(x, dat)	((I2C_HandleTypeDef *)x)->Instance->TXDR = dat
#define I2C_READ(x)			((I2C_HandleTypeDef *)x)->Instance->RXDR
#define I2C_DIS_ACK(x)		
#define I2C_EN_ACK(x)		
#define I2C_WAIT_SB(x)	
#define I2C_WAIT_ADDR10(x)
  
#define I2C_FLAG_BTF		I2C_FLAG_TC
#define I2C_FLAG_SB
  
#define I2C_OAR1_ADD0                       0x00000001U		

#define I2C_7BIT_ADD_WRITE(__ADDRESS__)                    ((uint8_t)((__ADDRESS__) & (~I2C_OAR1_ADD0)))
#define I2C_7BIT_ADD_READ(__ADDRESS__)                     ((uint8_t)((__ADDRESS__) | I2C_OAR1_ADD0))

#define I2C_10BIT_ADDRESS(__ADDRESS__)                     ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)(0x00FFU))))
#define I2C_10BIT_HEADER_WRITE(__ADDRESS__)                ((uint8_t)((uint16_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)(0x0300U))) >> 7U) | (uint16_t)(0x00F0U))))
#define I2C_10BIT_HEADER_READ(__ADDRESS__)                 ((uint8_t)((uint16_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)(0x0300U))) >> 7U) | (uint16_t)(0x00F1U))))

#define I2C_MEM_ADD_MSB(__ADDRESS__)                       ((uint8_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)(0xFF00U))) >> 8U)))
#define I2C_MEM_ADD_LSB(__ADDRESS__)                       ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)(0x00FFU))))

#define __HAL_I2C_CLEAR_ADDRFLAG(x) 						__HAL_I2C_CLEAR_FLAG(x, I2C_FLAG_ADDR)	
#endif
  
#define I2C_TIMEOUT_FLAG          ((uint32_t)35U)         /*!< Timeout 35 ms             */
#define I2C_TIMEOUT_ADDR_SLAVE    ((uint32_t)10000U)      /*!< Timeout 10 s              */
#define I2C_TIMEOUT_BUSY_FLAG     ((uint32_t)25U)         /*!< Timeout 25 ms             */
#define I2C_NO_OPTION_FRAME       ((uint32_t)0xFFFF0000U) /*!< XferOptions default value */

/* Private define for @ref PreviousState usage */
#define I2C_STATE_MSK             ((uint32_t)((HAL_I2C_STATE_BUSY_TX | HAL_I2C_STATE_BUSY_RX) & (~(uint32_t)HAL_I2C_STATE_READY))) /*!< Mask State define, keep only RX and TX bits            */
#define I2C_STATE_NONE            ((uint32_t)(HAL_I2C_MODE_NONE))                                                        /*!< Default Value                                          */
#define I2C_STATE_MASTER_BUSY_TX  ((uint32_t)((HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | HAL_I2C_MODE_MASTER))            /*!< Master Busy TX, combinaison of State LSB and Mode enum */
#define I2C_STATE_MASTER_BUSY_RX  ((uint32_t)((HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | HAL_I2C_MODE_MASTER))            /*!< Master Busy RX, combinaison of State LSB and Mode enum */
#define I2C_STATE_SLAVE_BUSY_TX   ((uint32_t)((HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | HAL_I2C_MODE_SLAVE))             /*!< Slave Busy TX, combinaison of State LSB and Mode enum  */
#define I2C_STATE_SLAVE_BUSY_RX   ((uint32_t)((HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | HAL_I2C_MODE_SLAVE))             /*!< Slave Busy RX, combinaison of State LSB and Mode enum  */

static HAL_StatusTypeDef I2C_WaitOnMasterAddressFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, uint32_t Timeout, uint32_t Tickstart)
{
  while(__HAL_I2C_GET_FLAG(hi2c, Flag) == RESET)
  {
    if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == SET)
    {
      /* Generate Stop */
      //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	  I2C_STOP(hi2c);

      /* Clear AF Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

      hi2c->ErrorCode = HAL_I2C_ERROR_AF;
      hi2c->PreviousState = I2C_STATE_NONE;
      hi2c->State= HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U)||((HAL_GetTick() - Tickstart ) > Timeout))
      {
        hi2c->PreviousState = I2C_STATE_NONE;
        hi2c->State= HAL_I2C_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  /* Wait until flag is set */
  while((__HAL_I2C_GET_FLAG(hi2c, Flag) ? SET : RESET) == Status) 
  {
    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U)||((HAL_GetTick() - Tickstart ) > Timeout))
      {
        hi2c->PreviousState = HAL_I2C_MODE_NONE;
        hi2c->State= HAL_I2C_STATE_READY;
        
        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);
        
        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_HandleTypeDef *hi2c)
{
  if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == SET)
  {
    /* Clear NACKF Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    hi2c->ErrorCode = HAL_I2C_ERROR_AF;
    hi2c->PreviousState = I2C_STATE_NONE;
    hi2c->State= HAL_I2C_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_ERROR;
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnTXEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{    
  while(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) == RESET)
  {
    /* Check if a NACK is detected */
    if(I2C_IsAcknowledgeFailed(hi2c) != HAL_OK)
    {
      return HAL_ERROR;
    }
		
    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U) || ((HAL_GetTick()-Tickstart) > Timeout))
      {
        hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
        hi2c->PreviousState = I2C_STATE_NONE;
        hi2c->State= HAL_I2C_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;      
}

static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{  

  while(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == RESET)
  {
    /* Check if a STOPF is detected */
    if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == SET)
    {
      /* Clear STOP Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

      hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
      hi2c->PreviousState = I2C_STATE_NONE;
      hi2c->State= HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if((Timeout == 0U) || ((HAL_GetTick()-Tickstart) > Timeout))
    {
      hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
      hi2c->State= HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_TIMEOUT;
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnBTFFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{  
  while(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BTF) == RESET)
  {
    /* Check if a NACK is detected */
    if(I2C_IsAcknowledgeFailed(hi2c) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U) || ((HAL_GetTick()-Tickstart) > Timeout))
      {
        hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
        hi2c->PreviousState = I2C_STATE_NONE;
        hi2c->State= HAL_I2C_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}

uint8 if_i2c_start_write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, uint32_t Tickstart) {
	/* Declaration of temporary variable to prevent undefined behavior of volatile usage */
  uint32_t CurrentXferOptions; 
  if(hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;
    /* Wait until BUSY flag is reset */
    if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, Tickstart) != HAL_OK)
    {
      return HAL_BUSY;
    }
    //((I2C_HandleTypeDef *)hi2c)->Instance->CR1 &= ~I2C_CR1_POS;
	I2C_ACK(hi2c);

	((I2C_HandleTypeDef *)hi2c)->State = HAL_I2C_STATE_BUSY_RX;
    ((I2C_HandleTypeDef *)hi2c)->Mode = HAL_I2C_MODE_MASTER;
    ((I2C_HandleTypeDef *)hi2c)->ErrorCode = HAL_I2C_ERROR_NONE;
    ((I2C_HandleTypeDef *)hi2c)->XferOptions = I2C_NO_OPTION_FRAME;
	CurrentXferOptions = hi2c->XferOptions;

  /* Generate Start condition if first transfer */
  if((CurrentXferOptions == I2C_FIRST_AND_LAST_FRAME) || (CurrentXferOptions == I2C_FIRST_FRAME) || (CurrentXferOptions == I2C_NO_OPTION_FRAME))
  {
    /* Generate Start */
    //hi2c->Instance->CR1 |= I2C_CR1_START;
	I2C_START(hi2c);
  }
  else if(hi2c->PreviousState == I2C_STATE_MASTER_BUSY_RX)
  {
    /* Generate ReStart */
    //hi2c->Instance->CR1 |= I2C_CR1_START;
	I2C_START(hi2c);
  }

  /* Wait until SB flag is set */
  //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  //{
  //  return HAL_TIMEOUT;
  //}
  I2C_WAIT_SB(hi2c);

  if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
  {
    /* Send slave address */
    //hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);
	I2C_WRITE(hi2c, I2C_7BIT_ADD_WRITE(DevAddress));
  }
  else
  {
    /* Send header of slave address */
    //hi2c->Instance->DR = I2C_10BIT_HEADER_WRITE(DevAddress);
	I2C_WRITE(hi2c, I2C_10BIT_HEADER_WRITE(DevAddress));

    /* Wait until ADD10 flag is set */
	I2C_WAIT_ADDR10(hi2c);

    /* Send slave address */
    //hi2c->Instance->DR = I2C_10BIT_ADDRESS(DevAddress);
	I2C_WRITE(hi2c, I2C_10BIT_ADDRESS(DevAddress));
  }

  /* Wait until ADDR flag is set */
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
  return HAL_OK;
}

uint8 if_i2c_start_read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, uint32_t Tickstart)
{
  /* Declaration of temporary variable to prevent undefined behavior of volatile usage */
  uint32_t CurrentXferOptions;
  if(hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;
    /* Wait until BUSY flag is reset */
    if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, Tickstart) != HAL_OK)
    {
      return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hi2c);

    //((I2C_HandleTypeDef *)hi2c)->Instance->CR1 &= ~I2C_CR1_POS;
	I2C_ACK(hi2c);

	((I2C_HandleTypeDef *)hi2c)->State = HAL_I2C_STATE_BUSY_RX;
    ((I2C_HandleTypeDef *)hi2c)->Mode = HAL_I2C_MODE_MASTER;
    ((I2C_HandleTypeDef *)hi2c)->ErrorCode = HAL_I2C_ERROR_NONE;
    ((I2C_HandleTypeDef *)hi2c)->XferOptions = I2C_NO_OPTION_FRAME;
	CurrentXferOptions = hi2c->XferOptions;

  /* Generate Start condition if first transfer */
  if((CurrentXferOptions == I2C_FIRST_AND_LAST_FRAME) || (CurrentXferOptions == I2C_FIRST_FRAME)  || (CurrentXferOptions == I2C_NO_OPTION_FRAME))
  {
    /* Enable Acknowledge */
    //hi2c->Instance->CR1 |= I2C_CR1_ACK;
	I2C_ACK(hi2c);

    /* Generate Start */
    //hi2c->Instance->CR1 |= I2C_CR1_START;
	I2C_START(hi2c);
  }
  else if(hi2c->PreviousState == I2C_STATE_MASTER_BUSY_TX)
  {
    /* Enable Acknowledge */
    //hi2c->Instance->CR1 |= I2C_CR1_ACK;
	I2C_ACK(hi2c);

    /* Generate ReStart */
    //hi2c->Instance->CR1 |= I2C_CR1_START;
	I2C_START(hi2c);
  }

  /* Wait until SB flag is set */
  //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  //{
  //  return HAL_TIMEOUT;
  //}
  I2C_WAIT_SB(hi2c);

  if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
  {
    /* Send slave address */
    //hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);
	I2C_WRITE(hi2c, I2C_7BIT_ADD_READ(DevAddress));
  }
  else
  {
    /* Send header of slave address */
    //hi2c->Instance->DR = I2C_10BIT_HEADER_WRITE(DevAddress);
	I2C_WRITE(hi2c, I2C_10BIT_HEADER_WRITE(DevAddress));

    /* Wait until ADD10 flag is set */
	I2C_WAIT_ADDR10(hi2c);

    /* Send slave address */
    //hi2c->Instance->DR = I2C_10BIT_ADDRESS(DevAddress);
	I2C_WRITE(hi2c, I2C_10BIT_ADDRESS(DevAddress));

    /* Wait until ADDR flag is set */
    if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
    {
      if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
      {
        return HAL_ERROR;
      }
      else
      {
        return HAL_TIMEOUT;
      }
    }

    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

    /* Generate Restart */
    //hi2c->Instance->CR1 |= I2C_CR1_START;
	I2C_START(hi2c);

    /* Wait until SB flag is set */
    //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
    //{
    //  return HAL_TIMEOUT;
    //}
	
	I2C_WAIT_SB(hi2c);

    /* Send header of slave address */
    //hi2c->Instance->DR = I2C_10BIT_HEADER_READ(DevAddress);
	I2C_WRITE(hi2c, I2C_10BIT_HEADER_READ(DevAddress));
  }

  /* Wait until ADDR flag is set */
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }
    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
	return HAL_OK;
}

uint8 if_i2c_restart(I2C_HandleTypeDef *hi2c, uint8 DevAddress, uint32_t Timeout, uint32_t Tickstart) {
	
    //((I2C_HandleTypeDef *)hi2c)->Instance->CR1 &= ~I2C_CR1_POS;
	I2C_ACK(hi2c);

	((I2C_HandleTypeDef *)hi2c)->State = HAL_I2C_STATE_BUSY_RX;
    ((I2C_HandleTypeDef *)hi2c)->Mode = HAL_I2C_MODE_MASTER;
    ((I2C_HandleTypeDef *)hi2c)->ErrorCode = HAL_I2C_ERROR_NONE;
    ((I2C_HandleTypeDef *)hi2c)->XferOptions = I2C_NO_OPTION_FRAME;
  /* Generate Restart */
  //hi2c->Instance->CR1 |= I2C_CR1_START;
  I2C_START(hi2c);

  /* Wait until SB flag is set */
  //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  //{
  //  return HAL_TIMEOUT;
  //}
  I2C_WAIT_SB(hi2c);

  /* Send slave address */
  //hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);
  I2C_WRITE(hi2c, I2C_7BIT_ADD_READ(DevAddress));

  /* Wait until ADDR flag is set */
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

  /* Disable Acknowledge */
  //hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
  I2C_DIS_ACK(hi2c);
      /* Clear ADDR flag */
      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
}


uint8 if_i2c_start_mem_read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout, uint32_t Tickstart)
{
  if(hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;
    /* Wait until BUSY flag is reset */
    if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, Tickstart) != HAL_OK)
    {
      return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Disable Pos */
    //hi2c->Instance->CR1 &= ~I2C_CR1_POS;
	I2C_ACK(hi2c);

    hi2c->State = HAL_I2C_STATE_BUSY_RX;
    hi2c->Mode = HAL_I2C_MODE_MEM;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
	
  /* Enable Acknowledge */
  //hi2c->Instance->CR1 |= I2C_CR1_ACK;
  I2C_EN_ACK(hi2c);

  /* Generate Start */
  //hi2c->Instance->CR1 |= I2C_CR1_START;
  I2C_START(hi2c);

  /* Wait until SB flag is set */
  //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  //{
  //  return HAL_TIMEOUT;
  //}
  
  I2C_WAIT_SB(hi2c);

  /* Send slave address */
  //hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);
  I2C_WRITE(hi2c, I2C_7BIT_ADD_WRITE(DevAddress));

  /* Wait until ADDR flag is set */
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

  /* Clear ADDR flag */
  __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

  /* Wait until TXE flag is set */
  if(I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	  I2C_STOP(hi2c);
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

  /* If Memory address size is 8Bit */
  if(MemAddSize == I2C_MEMADD_SIZE_8BIT)
  {
    /* Send Memory Address */
    //hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
	I2C_WRITE(hi2c, I2C_MEM_ADD_LSB(MemAddress));
  }
  /* If Memory address size is 16Bit */
  else
  {
    /* Send MSB of Memory Address */
    //hi2c->Instance->DR = I2C_MEM_ADD_MSB(MemAddress);
	I2C_WRITE(hi2c, I2C_MEM_ADD_MSB(MemAddress));

    /* Wait until TXE flag is set */
    if(I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
      {
        /* Generate Stop */
        //hi2c->Instance->CR1 |= I2C_CR1_STOP;
		I2C_STOP(hi2c);
        return HAL_ERROR;
      }
      else
      {
        return HAL_TIMEOUT;
      }
    }

    /* Send LSB of Memory Address */
    //hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
	I2C_WRITE(hi2c, I2C_MEM_ADD_LSB(MemAddress));
  }

  /* Wait until TXE flag is set */
  if(I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	  I2C_STOP(hi2c);
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

  /* Generate Restart */
  //hi2c->Instance->CR1 |= I2C_CR1_START;
  I2C_START(hi2c);

  /* Wait until SB flag is set */
  //if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  //{
  //  return HAL_TIMEOUT;
  //}
  
  I2C_WAIT_SB(hi2c);

  /* Send slave address */
  //hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);
  I2C_WRITE(hi2c, I2C_7BIT_ADD_READ(DevAddress));

  /* Wait until ADDR flag is set */
  if(I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }

  /* Disable Acknowledge */
  //hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
  I2C_DIS_ACK(hi2c);

  /* Clear ADDR flag */
  __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
  return HAL_OK;
}

uint8 if_i2c_read(I2C_HandleTypeDef *hi2c, uint8 * result, uint32_t timeout, uint32_t tickstart) {
	static uint8 buffer[2];
	static uint8 buflen = 0;
	static uint8 hindex = 0;
	if(buflen != hindex) {
		result[0] = buffer[hindex++ & 0x01];
		return HAL_OK;
	}
	if(I2C_WaitOnRXNEFlagUntilTimeout(hi2c, timeout, tickstart) != HAL_OK)      
	{
	  if(hi2c->ErrorCode == HAL_I2C_ERROR_TIMEOUT)
	  {
		return HAL_TIMEOUT;
	  }
	  else
	  {
		return HAL_ERROR;
	  }
    }
	//buffer[buflen++ & 0x01] = hi2c->Instance->DR;
	buffer[buflen++ & 0x01] = I2C_READ(hi2c);
	if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BTF) == SET)
	{
		//buffer[buflen++ & 0x01] = hi2c->Instance->DR;
		buffer[buflen++ & 0x01] = I2C_READ(hi2c);
	}
	result[0] = buffer[hindex++ & 0x01];
	return HAL_OK;
}

uint8 if_i2c_read_bytes(I2C_HandleTypeDef *hi2c, uint8 * pData, uint8 Size, uint32_t timeout, uint32_t tickstart) {
	while(Size > 0U)
    {
      if(Size <= 3U)
      {
        /* One byte */
        if(Size== 1U)
        {
          /* Wait until RXNE flag is set */
          if(I2C_WaitOnRXNEFlagUntilTimeout(hi2c, timeout, tickstart) != HAL_OK)      
          {
            if(hi2c->ErrorCode == HAL_I2C_ERROR_TIMEOUT)
            {
              return HAL_TIMEOUT;
            }
            else
            {
              return HAL_ERROR;
            }
          }

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;
        }
        /* Two bytes */
        else if(Size == 2U)
        {
          /* Wait until BTF flag is set */
          if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, timeout, tickstart) != HAL_OK)
          {
            return HAL_TIMEOUT;
          }

          /* Generate Stop */
          //hi2c->Instance->CR1 |= I2C_CR1_STOP;
		  I2C_STOP(hi2c);

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;
        }
        /* 3 Last bytes */
        else
        {
          /* Wait until BTF flag is set */
          if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, timeout, tickstart) != HAL_OK)
          {
            return HAL_TIMEOUT;
          }

          /* Disable Acknowledge */
          //hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
		  I2C_DIS_ACK(hi2c);

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;

          /* Wait until BTF flag is set */
          if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, timeout, tickstart) != HAL_OK)
          {
            return HAL_TIMEOUT;
          }

          /* Generate Stop */
          //hi2c->Instance->CR1 |= I2C_CR1_STOP;
		  I2C_STOP(hi2c);

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;

          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
		  (*pData++) = I2C_READ(hi2c);
          Size--;
        }
      }
      else
      {
        /* Wait until RXNE flag is set */
        if(I2C_WaitOnRXNEFlagUntilTimeout(hi2c, timeout, tickstart) != HAL_OK)      
        {
          if(hi2c->ErrorCode == HAL_I2C_ERROR_TIMEOUT)
          {
            return HAL_TIMEOUT;
          }
          else
          {
            return HAL_ERROR;
          }
        }

        /* Read data from DR */
        //(*pData++) = hi2c->Instance->DR;
		(*pData++) = I2C_READ(hi2c);
        Size--;

        if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BTF) == SET)
        {
          /* Read data from DR */
          //(*pData++) = hi2c->Instance->DR;
          (*pData++) = I2C_READ(hi2c);
          Size--;
        }
      }
    }
	return HAL_OK;
}


uint8 if_i2c_read_byte(I2C_HandleTypeDef *hi2c) {
	uint8 c;
	if_i2c_read(hi2c, &c, 1, HAL_GetTick());
	return c;
}

uint8 if_i2c_write(I2C_HandleTypeDef *hi2c, uint8 value, uint32_t timeout, uint32_t tickstart) {
	
  /* Wait until TXE flag is set */
  if(I2C_WaitOnTXEFlagUntilTimeout(hi2c, timeout, tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	  I2C_STOP(hi2c);
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }
	/* Write data to DR */
	//hi2c->Instance->DR = value;
    I2C_WRITE(hi2c, value);
	
	/* Wait until TXE flag is set */
	if(I2C_WaitOnTXEFlagUntilTimeout(hi2c, timeout, tickstart) != HAL_OK)
  {
    if(hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	  I2C_STOP(hi2c);
      return HAL_ERROR;
    }
    else
    {
      return HAL_TIMEOUT;
    }
  }
	return HAL_OK;
}

void if_i2c_stop(I2C_HandleTypeDef *hi2c) {
	
	/* Disable Acknowledge */
	//hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
	I2C_DIS_ACK(hi2c);
    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
    /* Generate Stop */
    //hi2c->Instance->CR1 |= I2C_CR1_STOP;
	I2C_STOP(hi2c);

    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
	__HAL_UNLOCK((I2C_HandleTypeDef *)hi2c);
}