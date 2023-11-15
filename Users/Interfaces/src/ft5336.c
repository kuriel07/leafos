/**
  ******************************************************************************
  * @file    ft5336.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    25-June-2015
  * @brief   This file provides a set of functions needed to manage the FT5336
  *          touch screen devices.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "defs.h"
#include "config.h"
#include <stdint.h>
#include "..\inc\if_apis.h"
#include "..\inc\ft5336\ft5336.h"


#if SHARD_TS_TYPE == 0x5336

static ui_config * g_cur_tconfig = NULL;

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup FT5336
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/** @defgroup FT5336_Private_Types_Definitions
  * @{
  */

/* Private define ------------------------------------------------------------*/

/** @defgroup FT5336_Private_Defines
  * @{
  */

/* Private macro -------------------------------------------------------------*/

/** @defgroup FT5336_Private_Macros
  * @{
  */

/* Private variables ---------------------------------------------------------*/

/** @defgroup FT5336_Private_Variables
  * @{
  */

/* Touch screen driver structure initialization */
TS_DrvTypeDef ft5336_ts_drv =
{
  ft5336_Init,
  ft5336_ReadID,
  ft5336_Reset,

  ft5336_TS_Start,
  ft5336_TS_DetectTouch,
  ft5336_TS_GetXY,

  ft5336_TS_EnableIT,
  ft5336_TS_ClearIT,
  ft5336_TS_ITStatus,
  ft5336_TS_DisableIT

};

static I2C_HandleTypeDef hI2cAudioHandler = {0};

/******************************* I2C Routines *********************************/
/**
  * @brief  Initializes I2C MSP.
  * @param  i2c_handler : I2C handler
  * @retval None
  */
static void I2Cx_MspInit(I2C_HandleTypeDef *i2c_handler)
{
  GPIO_InitTypeDef  gpio_init_structure;
  
  if (i2c_handler == (I2C_HandleTypeDef*)(&hI2cAudioHandler))
  {
    /* AUDIO and LCD I2C MSP init */

    /*** Configure the GPIOs ***/
    /* Enable GPIO clock */
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Configure I2C Tx as alternate function */
    gpio_init_structure.Pin = GPIO_PIN_7;
    gpio_init_structure.Mode = GPIO_MODE_AF_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    gpio_init_structure.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /* Configure I2C Rx as alternate function */
    gpio_init_structure.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOH, &gpio_init_structure);
		
		
    //gpio_init_structure.Pin = GPIO_PIN_15;
		//gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;
		//gpio_init_structure.Pull = GPIO_NOPULL;
    //HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /*** Configure the I2C peripheral ***/
    /* Enable I2C clock */
    __HAL_RCC_I2C3_CLK_ENABLE();

    /* Force the I2C peripheral clock reset */
    __HAL_RCC_I2C3_FORCE_RESET();

    /* Release the I2C peripheral clock reset */
    __HAL_RCC_I2C3_RELEASE_RESET();

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(I2C3_EV_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(I2C3_ER_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
  }
  else
  {
    /* External, camera and Arduino connector I2C MSP init */

    /*** Configure the GPIOs ***/
    /* Enable GPIO clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure I2C Tx as alternate function */
    gpio_init_structure.Pin = GPIO_PIN_8;
    gpio_init_structure.Mode = GPIO_MODE_AF_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    gpio_init_structure.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

    /* Configure I2C Rx as alternate function */
    gpio_init_structure.Pin = GPIO_PIN_9;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

    /*** Configure the I2C peripheral ***/
    /* Enable I2C clock */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* Force the I2C peripheral clock reset */
    __HAL_RCC_I2C1_FORCE_RESET();

    /* Release the I2C peripheral clock reset */
    __HAL_RCC_I2C1_RELEASE_RESET();

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  }
}

/**
  * @brief  Initializes I2C HAL.
  * @param  i2c_handler : I2C handler
  * @retval None
  */
static void I2Cx_Init(I2C_HandleTypeDef *i2c_handler)
{
  if(HAL_I2C_GetState(i2c_handler) == HAL_I2C_STATE_RESET)
  {
    if (i2c_handler == (I2C_HandleTypeDef*)(&hI2cAudioHandler))
    {
      /* Audio and LCD I2C configuration */
      i2c_handler->Instance = I2C3;
    }
    else
    {
      /* External, camera and Arduino connector  I2C configuration */
      i2c_handler->Instance = I2C1;
    }
    i2c_handler->Init.Timing           = 0x40912732;
    i2c_handler->Init.OwnAddress1      = 0;
    i2c_handler->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2c_handler->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2c_handler->Init.OwnAddress2      = 0;
    i2c_handler->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2c_handler->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    /* Init the I2C */
    I2Cx_MspInit(i2c_handler);
    HAL_I2C_Init(i2c_handler);
  }
}

/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  i2c_handler : I2C handler
  * @param  Addr: I2C Address
  * @retval None
  */
static void I2Cx_Error(I2C_HandleTypeDef *i2c_handler, uint8_t Addr)
{
  /* De-initialize the I2C communication bus */
  HAL_I2C_DeInit(i2c_handler);
  
  /* Re-Initialize the I2C communication bus */
  I2Cx_Init(i2c_handler);
}

/**
  * @brief  Reads multiple data.
  * @param  i2c_handler : I2C handler
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  MemAddress: Memory address 
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
static HAL_StatusTypeDef I2Cx_ReadMultiple(I2C_HandleTypeDef *i2c_handler,
                                           uint8_t Addr,
                                           uint16_t Reg,
                                           uint16_t MemAddress,
                                           uint8_t *Buffer,
                                           uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occurred */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;    
}

/**
  * @brief  Writes a value in a register of the device through BUS in using DMA mode.
  * @param  i2c_handler : I2C handler
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  MemAddress: Memory address 
  * @param  Buffer: The target register value to be written 
  * @param  Length: buffer size to be written
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_WriteMultiple(I2C_HandleTypeDef *i2c_handler,
                                            uint8_t Addr,
                                            uint16_t Reg,
                                            uint16_t MemAddress,
                                            uint8_t *Buffer,
                                            uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;
}

/**
  * @brief  Checks if target device is ready for communication. 
  * @note   This function is used with Memory devices
  * @param  i2c_handler : I2C handler
  * @param  DevAddress: Target device address
  * @param  Trials: Number of trials
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_IsDeviceReady(I2C_HandleTypeDef *i2c_handler, uint16_t DevAddress, uint32_t Trials)
{ 
  return (HAL_I2C_IsDeviceReady(i2c_handler, DevAddress, Trials, 1000));
}


/********************************* LINK TOUCHSCREEN *********************************/

/**
  * @brief  Initializes Touchscreen low level.
  * @retval None
  */
void TS_IO_Init(void)
{
  I2Cx_Init(&hI2cAudioHandler);
}

/**
  * @brief  Writes a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @param  Value: Data to be written
  * @retval None
  */
void TS_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_WriteMultiple(&hI2cAudioHandler, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT,(uint8_t*)&Value, 1);
}

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @retval Data to be read
  */
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg)
{
  uint8_t read_value = 0;

  I2Cx_ReadMultiple(&hI2cAudioHandler, Addr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&read_value, 1);

  return read_value;
}

/**
  * @brief  TS delay
  * @param  Delay: Delay in ms
  * @retval None
  */
void TS_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/* Global ft5336 handle */
static ft5336_handle_TypeDef ft5336_handle = { FT5336_I2C_NOT_INITIALIZED, 0, 0};

/**
  * @}
  */

/** @defgroup ft5336_Private_Function_Prototypes
  * @{
  */

/* Private functions prototypes-----------------------------------------------*/

/**
  * @brief  Return the status of I2C was initialized or not.
  * @param  None.
  * @retval : I2C initialization status.
  */
static uint8_t ft5336_Get_I2C_InitializedStatus(void);

/**
  * @brief  I2C initialize if needed.
  * @param  None.
  * @retval : None.
  */
static void ft5336_I2C_InitializeIfRequired(void);

/**
  * @brief  Basic static configuration of TouchScreen
  * @param  DeviceAddr: FT5336 Device address for communication on I2C Bus.
  * @retval Status FT5336_STATUS_OK or FT5336_STATUS_NOT_OK.
  */
static uint32_t ft5336_TS_Configure(uint16_t DeviceAddr);

/** @defgroup ft5336_Private_Functions
  * @{
  */

/** @defgroup ft5336_Public_Function_Body
  * @{
  */

/* Public functions bodies-----------------------------------------------*/


/**
  * @brief  Initialize the ft5336 communication bus
  *         from MCU to FT5336 : ie I2C channel initialization (if required).
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @retval None
  */
void ft5336_Init(uint16_t DeviceAddr)
{
  /* Wait at least 200ms after power up before accessing registers
   * Trsi timing (Time of starting to report point after resetting) from FT5336GQQ datasheet */
  TS_IO_Delay(200);

  /* Initialize I2C link if needed */
  ft5336_I2C_InitializeIfRequired();
	//NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  Software Reset the ft5336.
  *         @note : Not applicable to FT5336.
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @retval None
  */
void ft5336_Reset(uint16_t DeviceAddr)
{
  /* Do nothing */
  /* No software reset sequence available in FT5336 IC */
}

/**
  * @brief  Read the ft5336 device ID, pre initialize I2C in case of need to be
  *         able to read the FT5336 device ID, and verify this is a FT5336.
  * @param  DeviceAddr: I2C FT5336 Slave address.
  * @retval The Device ID (two bytes).
  */
uint16_t ft5336_ReadID(uint16_t DeviceAddr)
{
  volatile uint8_t ucReadId = 0;
  uint8_t nbReadAttempts = 0;
  uint8_t bFoundDevice = 0; /* Device not found by default */

  /* Initialize I2C link if needed */
  ft5336_I2C_InitializeIfRequired();

  /* At maximum 4 attempts to read ID : exit at first finding of the searched device ID */
  for(nbReadAttempts = 0; ((nbReadAttempts < 3) && !(bFoundDevice)); nbReadAttempts++)
  {
    /* Read register FT5336_CHIP_ID_REG as DeviceID detection */
    ucReadId = TS_IO_Read(DeviceAddr, FT5336_CHIP_ID_REG);

    /* Found the searched device ID ? */
    if(ucReadId == FT5336_ID_VALUE)
    {
      /* Set device as found */
      bFoundDevice = 1;
    }
  }

  /* Return the device ID value */
  return (ucReadId);
}

/**
  * @brief  Configures the touch Screen IC device to start detecting touches
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address).
  * @retval None.
  */
void ft5336_TS_Start(uint16_t DeviceAddr)
{
  /* Minimum static configuration of FT5336 */
  FT5336_ASSERT(ft5336_TS_Configure(DeviceAddr));

  /* By default set FT5336 IC in Polling mode : no INT generation on FT5336 for new touch available */
  /* Note TS_INT is active low                                                                      */
  ft5336_TS_EnableIT(DeviceAddr);
}

/**
  * @brief  Return if there is touches detected or not.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval : Number of active touches detected (can be 0, 1 or 2).
  */
uint8_t ft5336_TS_DetectTouch(uint16_t DeviceAddr)
{
  volatile uint8_t nbTouch = 0;

  /* Read register FT5336_TD_STAT_REG to check number of touches detection */
  nbTouch = TS_IO_Read(DeviceAddr, FT5336_TD_STAT_REG);
  nbTouch &= FT5336_TD_STAT_MASK;

  if(nbTouch > FT5336_MAX_DETECTABLE_TOUCH)
  {
    /* If invalid number of touch detected, set it to zero */
    nbTouch = 0;
  }

  /* Update ft5336 driver internal global : current number of active touches */
  ft5336_handle.currActiveTouchNb = nbTouch;

  /* Reset current active touch index on which to work on */
  ft5336_handle.currActiveTouchIdx = 0;

  return(nbTouch);
}

/**
  * @brief  Get the touch screen X and Y positions values
  *         Manage multi touch thanks to touch Index global
  *         variable 'ft5336_handle.currActiveTouchIdx'.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  X: Pointer to X position value
  * @param  Y: Pointer to Y position value
  * @retval None.
  */
void ft5336_TS_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y)
{
  volatile uint8_t ucReadData = 0;
  static uint16_t coord;
  uint8_t regAddressXLow = 0;
  uint8_t regAddressXHigh = 0;
  uint8_t regAddressYLow = 0;
  uint8_t regAddressYHigh = 0;
	//5336_handle.currActiveTouchIdx = 0;

  if(ft5336_handle.currActiveTouchIdx < ft5336_handle.currActiveTouchNb)
  {
    switch(ft5336_handle.currActiveTouchIdx)
    {
    case 0 :
      regAddressXLow  = FT5336_P1_XL_REG;
      regAddressXHigh = FT5336_P1_XH_REG;
      regAddressYLow  = FT5336_P1_YL_REG;
      regAddressYHigh = FT5336_P1_YH_REG;
      break;

    case 1 :
      regAddressXLow  = FT5336_P2_XL_REG;
      regAddressXHigh = FT5336_P2_XH_REG;
      regAddressYLow  = FT5336_P2_YL_REG;
      regAddressYHigh = FT5336_P2_YH_REG;
      break;

    case 2 :
      regAddressXLow  = FT5336_P3_XL_REG;
      regAddressXHigh = FT5336_P3_XH_REG;
      regAddressYLow  = FT5336_P3_YL_REG;
      regAddressYHigh = FT5336_P3_YH_REG;
      break;

    case 3 :
      regAddressXLow  = FT5336_P4_XL_REG;
      regAddressXHigh = FT5336_P4_XH_REG;
      regAddressYLow  = FT5336_P4_YL_REG;
      regAddressYHigh = FT5336_P4_YH_REG;
      break;

    case 4 :
      regAddressXLow  = FT5336_P5_XL_REG;
      regAddressXHigh = FT5336_P5_XH_REG;
      regAddressYLow  = FT5336_P5_YL_REG;
      regAddressYHigh = FT5336_P5_YH_REG;
      break;

    case 5 :
      regAddressXLow  = FT5336_P6_XL_REG;
      regAddressXHigh = FT5336_P6_XH_REG;
      regAddressYLow  = FT5336_P6_YL_REG;
      regAddressYHigh = FT5336_P6_YH_REG;
      break;

    case 6 :
      regAddressXLow  = FT5336_P7_XL_REG;
      regAddressXHigh = FT5336_P7_XH_REG;
      regAddressYLow  = FT5336_P7_YL_REG;
      regAddressYHigh = FT5336_P7_YH_REG;
      break;

    case 7 :
      regAddressXLow  = FT5336_P8_XL_REG;
      regAddressXHigh = FT5336_P8_XH_REG;
      regAddressYLow  = FT5336_P8_YL_REG;
      regAddressYHigh = FT5336_P8_YH_REG;
      break;

    case 8 :
      regAddressXLow  = FT5336_P9_XL_REG;
      regAddressXHigh = FT5336_P9_XH_REG;
      regAddressYLow  = FT5336_P9_YL_REG;
      regAddressYHigh = FT5336_P9_YH_REG;
      break;

    case 9 :
      regAddressXLow  = FT5336_P10_XL_REG;
      regAddressXHigh = FT5336_P10_XH_REG;
      regAddressYLow  = FT5336_P10_YL_REG;
      regAddressYHigh = FT5336_P10_YH_REG;
      break;

    default :
      break;

    } /* end switch(ft5336_handle.currActiveTouchIdx) */

    /* Read low part of X position */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressXLow);
    coord = (ucReadData & FT5336_TOUCH_POS_LSB_MASK) >> FT5336_TOUCH_POS_LSB_SHIFT;

    /* Read high part of X position */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressXHigh);
    coord |= ((ucReadData & FT5336_TOUCH_POS_MSB_MASK) >> FT5336_TOUCH_POS_MSB_SHIFT) << 8;

    /* Send back ready X position to caller */
    *Y = coord;

    /* Read low part of Y position */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressYLow);
    coord = (ucReadData & FT5336_TOUCH_POS_LSB_MASK) >> FT5336_TOUCH_POS_LSB_SHIFT;

    /* Read high part of Y position */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressYHigh);
    coord |= ((ucReadData & FT5336_TOUCH_POS_MSB_MASK) >> FT5336_TOUCH_POS_MSB_SHIFT) << 8;

    /* Send back ready Y position to caller */
    *X = coord;

    ft5336_handle.currActiveTouchIdx++; /* next call will work on next touch */

  } /* of if(ft5336_handle.currActiveTouchIdx < ft5336_handle.currActiveTouchNb) */
}

/**
  * @brief  Configure the FT5336 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  DeviceAddr: Device address on communication Bus (Slave I2C address of FT5336).
  * @retval None
  */
void ft5336_TS_EnableIT(uint16_t DeviceAddr)
{
   uint8_t regValue = 0;
   regValue = (FT5336_G_MODE_INTERRUPT_TRIGGER & (FT5336_G_MODE_INTERRUPT_MASK >> FT5336_G_MODE_INTERRUPT_SHIFT)) << FT5336_G_MODE_INTERRUPT_SHIFT;

   /* Set interrupt trigger mode in FT5336_GMODE_REG */
   TS_IO_Write(DeviceAddr, FT5336_GMODE_REG, regValue);
}

/**
  * @brief  Configure the FT5336 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  DeviceAddr: Device address on communication Bus (Slave I2C address of FT5336).
  * @retval None
  */
void ft5336_TS_DisableIT(uint16_t DeviceAddr)
{
  uint8_t regValue = 0;
  regValue = (FT5336_G_MODE_INTERRUPT_POLLING & (FT5336_G_MODE_INTERRUPT_MASK >> FT5336_G_MODE_INTERRUPT_SHIFT)) << FT5336_G_MODE_INTERRUPT_SHIFT;

  /* Set interrupt polling mode in FT5336_GMODE_REG */
  TS_IO_Write(DeviceAddr, FT5336_GMODE_REG, regValue);
}

/**
  * @brief  Get IT status from FT5336 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not applicable to FT5336.
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @retval TS interrupts status : always return 0 here
  */
uint8_t ft5336_TS_ITStatus(uint16_t DeviceAddr)
{
  /* Always return 0 as feature not applicable to FT5336 */
  return 0;
}

/**
  * @brief  Clear IT status in FT5336 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not applicable to FT5336.
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @retval None
  */
void ft5336_TS_ClearIT(uint16_t DeviceAddr)
{
  /* Nothing to be done here for FT5336 */
}

/**** NEW FEATURES enabled when Multi-touch support is enabled ****/

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

/**
  * @brief  Get the last touch gesture identification (zoom, move up/down...).
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @param  pGestureId : Pointer to get last touch gesture Identification.
  * @retval None.
  */
void ft5336_TS_GetGestureID(uint16_t DeviceAddr, uint32_t * pGestureId)
{
  volatile uint8_t ucReadData = 0;

  ucReadData = TS_IO_Read(DeviceAddr, FT5336_GEST_ID_REG);

  * pGestureId = ucReadData;
}

/**
  * @brief  Get the touch detailed informations on touch number 'touchIdx' (0..1)
  *         This touch detailed information contains :
  *         - weight that was applied to this touch
  *         - sub-area of the touch in the touch panel
  *         - event of linked to the touch (press down, lift up, ...)
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @param  touchIdx : Passed index of the touch (0..1) on which we want to get the
  *                    detailed information.
  * @param  pWeight : Pointer to to get the weight information of 'touchIdx'.
  * @param  pArea   : Pointer to to get the sub-area information of 'touchIdx'.
  * @param  pEvent  : Pointer to to get the event information of 'touchIdx'.

  * @retval None.
  */
void ft5336_TS_GetTouchInfo(uint16_t   DeviceAddr,
                            uint32_t   touchIdx,
                            uint32_t * pWeight,
                            uint32_t * pArea,
                            uint32_t * pEvent)
{
  volatile uint8_t ucReadData = 0;
  uint8_t regAddressXHigh = 0;
  uint8_t regAddressPWeight = 0;
  uint8_t regAddressPMisc = 0;

  if(touchIdx < ft5336_handle.currActiveTouchNb)
  {
    switch(touchIdx)
    {
    case 0 :
      regAddressXHigh   = FT5336_P1_XH_REG;
      regAddressPWeight = FT5336_P1_WEIGHT_REG;
      regAddressPMisc   = FT5336_P1_MISC_REG;
      break;

    case 1 :
      regAddressXHigh   = FT5336_P2_XH_REG;
      regAddressPWeight = FT5336_P2_WEIGHT_REG;
      regAddressPMisc   = FT5336_P2_MISC_REG;
      break;

    case 2 :
      regAddressXHigh   = FT5336_P3_XH_REG;
      regAddressPWeight = FT5336_P3_WEIGHT_REG;
      regAddressPMisc   = FT5336_P3_MISC_REG;
      break;

    case 3 :
      regAddressXHigh   = FT5336_P4_XH_REG;
      regAddressPWeight = FT5336_P4_WEIGHT_REG;
      regAddressPMisc   = FT5336_P4_MISC_REG;
      break;

    case 4 :
      regAddressXHigh   = FT5336_P5_XH_REG;
      regAddressPWeight = FT5336_P5_WEIGHT_REG;
      regAddressPMisc   = FT5336_P5_MISC_REG;
      break;

    case 5 :
      regAddressXHigh   = FT5336_P6_XH_REG;
      regAddressPWeight = FT5336_P6_WEIGHT_REG;
      regAddressPMisc   = FT5336_P6_MISC_REG;
      break;

    case 6 :
      regAddressXHigh   = FT5336_P7_XH_REG;
      regAddressPWeight = FT5336_P7_WEIGHT_REG;
      regAddressPMisc   = FT5336_P7_MISC_REG;
      break;

    case 7 :
      regAddressXHigh   = FT5336_P8_XH_REG;
      regAddressPWeight = FT5336_P8_WEIGHT_REG;
      regAddressPMisc   = FT5336_P8_MISC_REG;
      break;

    case 8 :
      regAddressXHigh   = FT5336_P9_XH_REG;
      regAddressPWeight = FT5336_P9_WEIGHT_REG;
      regAddressPMisc   = FT5336_P9_MISC_REG;
      break;

    case 9 :
      regAddressXHigh   = FT5336_P10_XH_REG;
      regAddressPWeight = FT5336_P10_WEIGHT_REG;
      regAddressPMisc   = FT5336_P10_MISC_REG;
      break;

    default :
      break;

    } /* end switch(touchIdx) */

    /* Read Event Id of touch index */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressXHigh);
    * pEvent = (ucReadData & FT5336_TOUCH_EVT_FLAG_MASK) >> FT5336_TOUCH_EVT_FLAG_SHIFT;

    /* Read weight of touch index */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressPWeight);
    * pWeight = (ucReadData & FT5336_TOUCH_WEIGHT_MASK) >> FT5336_TOUCH_WEIGHT_SHIFT;

    /* Read area of touch index */
    ucReadData = TS_IO_Read(DeviceAddr, regAddressPMisc);
    * pArea = (ucReadData & FT5336_TOUCH_AREA_MASK) >> FT5336_TOUCH_AREA_SHIFT;

  } /* of if(touchIdx < ft5336_handle.currActiveTouchNb) */
}

#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

/** @defgroup ft5336_Static_Function_Body
  * @{
  */

/* Static functions bodies-----------------------------------------------*/


/**
  * @brief  Return the status of I2C was initialized or not.
  * @param  None.
  * @retval : I2C initialization status.
  */
static uint8_t ft5336_Get_I2C_InitializedStatus(void)
{
  return(ft5336_handle.i2cInitialized);
}

/**
  * @brief  I2C initialize if needed.
  * @param  None.
  * @retval : None.
  */
static void ft5336_I2C_InitializeIfRequired(void)
{
  if(ft5336_Get_I2C_InitializedStatus() == FT5336_I2C_NOT_INITIALIZED)
  {
    /* Initialize TS IO BUS layer (I2C) */
    TS_IO_Init();

    /* Set state to initialized */
    ft5336_handle.i2cInitialized = FT5336_I2C_INITIALIZED;
  }
}

/**
  * @brief  Basic static configuration of TouchScreen
  * @param  DeviceAddr: FT5336 Device address for communication on I2C Bus.
  * @retval Status FT5336_STATUS_OK or FT5336_STATUS_NOT_OK.
  */
static uint32_t ft5336_TS_Configure(uint16_t DeviceAddr)
{
  uint32_t status = FT5336_STATUS_OK;

  /* Nothing special to be done for FT5336 */

  return(status);
}


static void if_touch_draw_point(gui_handle_p display, uint16 x, uint16 y)
{
	display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x - 4, y - 4, 8, 8));
}	  


uint8_t if_touch_read_once(uint16 nbtouch)
{
	uint8 ret = 1;
	uint8_t t=0;	 
		uint16 i=0;
	//ft5336_TS_DisableIT(TS_I2C_ADDRESS);		//disable interrupt tp
	
	//NVIC_DisableIRQ(EXTI15_10_IRQn);
	g_cur_tconfig->t_state = UI_KEY_UP;
	//while( ft5336_TS_DetectTouch(TS_I2C_ADDRESS) == 0 ) { }
	for (i=0;i<nbtouch;i++) {
		ft5336_TS_GetXY(TS_I2C_ADDRESS, &g_cur_tconfig->x, &g_cur_tconfig->y);
		ret = 1;
	}
	//ft5336_TS_EnableIT(TS_I2C_ADDRESS);		//enable interrupt tp
	
	//NVIC_EnableIRQ(EXTI15_10_IRQn);
	return ret;
}

void if_touch_adjust(void * handle)
{								 
	uint16_t pos_temp[4][2];//◊¯±ÅE∫¥Ê÷µ
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint16 nbtouch = 0;
	uint32_t tem1,tem2;
	ui_config * conf = ((gui_handle_p)handle)->touch_config;
	float fac; 	   
	gui_handle_p display = (gui_handle_p)handle;
	cnt=0;				
	display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
	display->present(display);
	if_touch_draw_point(display, 20, 20);
	conf->t_state = UI_KEY_UP;//œ˚≥˝¥•∑¢–≈∫≈ 
	conf->xfac = 0;//xfac”√¿¥±ÅE« «∑Ò–£◊ºπ˝,À˘“‘–£◊º÷Æ«∞±ÿ–ÅEÂµÅE“‘√‚¥˙ÍÅE 
	while(1)
	{
		//if(conf->t_state == UI_KEY_DOWN)//∞¥ºÅE¥œ¬¡À
		if((nbtouch = ft5336_TS_DetectTouch(TS_I2C_ADDRESS)) != 0)
		{
			os_wait(50);
			if(if_touch_read_once(nbtouch))//µ√µΩµ•¥Œ∞¥ºÅEµ
			{  								   
				pos_temp[cnt][0]=conf->x;
				pos_temp[cnt][1]=conf->y;
				cnt++;
			}			 
			switch(cnt)
			{			   
				case 1:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), 20);
					display->present(display);
					break;
				case 2:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, 20, (display->height - 20));
					display->present(display);
					break;
				case 3:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), (display->height - 20));
					display->present(display);
					break;
				case 4:	 //»´≤øÀƒ∏ˆµ„“—æ≠µ√µΩ
	    		    	//∂‘±ﬂœ‡µ»
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					tem2=0;
					d1=sqrt(tem1+tem2);//µ√µΩ1,2µƒæ‡¿ÅE
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					tem2=0;
					d2=sqrt(tem1+tem2);//µ√µΩ3,4µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.6||fac>2||d1==0||d2==0)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem1 = 0;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//µ√µΩ1,3µƒæ‡¿ÅE
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem1 = 0;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//µ√µΩ2,4µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.6||fac>2)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}//’˝»∑¡À
								   
					//∂‘Ω«œﬂœ‡µ»
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x2-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y2-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//µ√µΩ1,4µƒæ‡¿ÅE
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x1-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y1-y4 
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//µ√µΩ2,3µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.6||fac>2)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}
					conf->yfac=(float)(display->height - 40)/(pos_temp[0][1]-pos_temp[3][1]);//µ√µΩxfac		 
					conf->yoff=(display->height - conf->yfac*(pos_temp[0][1]+pos_temp[3][1]))/2;//µ√µΩxoff
						  
					conf->xfac=(float)(display->width - 40)/(pos_temp[3][0]-pos_temp[2][0]);//µ√µΩyfac
					conf->xoff=(display->width - conf->xfac*(pos_temp[3][0]+pos_temp[2][0]))/2;//µ√µΩyoff  
					return;//–£’˝ÕÅE…				 
			}
		}
	} 
}

static uint8 if_touch_read(gui_handle * display, uint16 * x, uint16 * y) {
	uint8 ret = 0;
	uint csr;
	ui_config * conf = display->touch_config;
	if(conf->t_state != UI_KEY_DOWN) return -1;
	conf->t_state = UI_KEY_UP;
	//x[0] = conf->x;
	//y[0] = conf->y;
	//return ret;
	
#if 0
	NVIC_DisableIRQ(EXTI15_10_IRQn);
	csr = os_enter_critical();
	if_touch_wake(display);
	//start read here
	if(if_touch_get_xy(&conf->x,&conf->y))
	{
		if_touch_get(display, x, y);
	} else 
		ret = -1;
	
	if_touch_sleep(display);
    os_exit_critical(csr);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
#else
	ft5336_TS_GetXY(TS_I2C_ADDRESS, &conf->x, &conf->y);
#endif
	if_touch_get(display, x, y);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
	return ret;
}


uint8 if_touch_get(void * display, uint16 * x, uint16 * y) {
	uint16 xx, yy, t;
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	xx = conf->xfac * conf->x + conf->xoff;
	yy = (conf->yfac * conf->y + conf->yoff);
	switch(((gui_handle_p)display)->orientation) {
		case UI_DISP_ORIENTATION_0:
			xx = xx;
			yy = ((gui_handle_p)display)->height - yy;
			break;
		case UI_DISP_ORIENTATION_1:
			t = xx;
			xx = ((gui_handle_p)display)->width - yy;
			yy = ((gui_handle_p)display)->height - t;
			break;
		case UI_DISP_ORIENTATION_2:		//default orientation
			xx = ((gui_handle_p)display)->width - xx;
			yy = yy;
			break;
		case UI_DISP_ORIENTATION_3:
			t = xx;
			xx = yy;
			yy = t;
			break;
	}
	*x = xx;
	*y = yy;
	return 0;
}

static uint8 if_touch_state(gui_handle * display) {
	ui_config * conf = display->touch_config;
	return conf->t_state;
}

//touch apis definition
void if_touch_wake(void * display) {
	
}

void if_touch_sleep(void * display) {
	
}

void EXTI15_10_IRQHandler()
{
	uint16_t i;
	void * task;
  	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET)
	{
    	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_15);
		task = os_find_task_by_name("tc");
		if(task != NULL) os_resume(task);		
		if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_TOUCH);
	}
}

void if_task_touch() {
	//very high priority task
	uint8 reg;
	uint8 buffer[6];
	gui_handle_p display = (gui_handle_p)os_get_context();
	while(1) {
		//os_suspend();
		while(ft5336_TS_DetectTouch(TS_I2C_ADDRESS) == 0) { 
			__nop(); 
			os_wait(50);		//MUST ADD delay before reading any data from FT6x36
		}
		if(g_cur_tconfig != NULL) {
			//ft5336_TS_GetXY(TS_I2C_ADDRESS, &g_cur_tconfig->x, &g_cur_tconfig->y);
			display->touch_read(display, &g_cur_tconfig->x, &g_cur_tconfig->y);
				g_cur_tconfig->t_state=UI_KEY_DOWN;
		}
	}
}

int8 if_touch_init(void * display)
{
	uint8 reg;
	//os_task * task = NULL;
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	g_cur_tconfig = conf;
	GPIO_InitTypeDef GPIO_InitStructure;
	ft5336_Init(TS_I2C_ADDRESS);
	ft5336_TS_Start(TS_I2C_ADDRESS);
	ft5336_TS_EnableIT(TS_I2C_ADDRESS);
	
	//for(addr =0;addr < 0xFF;addr++) {
	//if(HAL_I2C_IsDeviceReady(&dev_ft6236, FT6X36_ADDR, 10, 2000) == HAL_OK) {
	//	reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0x00);
	//	reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0xBC);
	//	reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0x00);
	//	TS_I2C_ReadMulti(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_THOLD, ((gui_handle_p)display)->touch_treshold, 6);
	//} else 
	//	return -1;
	
	((gui_handle_p)display)->touch_state = if_touch_state;
	((gui_handle_p)display)->touch_read = if_touch_read;
	if(conf->magic != 0xEC) {
		memset((uint8 *)conf, 0, sizeof(ui_config));
		//if_gui_switch_orientation(display, );
		//task = os_find_task_by_name("tc");
		//if(task != NULL) os_resume(
		
		if_touch_adjust(display);
		conf->magic = 0xEC;
		if_config_write(conf, sizeof(ui_config));
	}
	os_create_task(display, if_task_touch, "tc", 11, 4096);
	return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
	
	#endif 		//end 5336
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
