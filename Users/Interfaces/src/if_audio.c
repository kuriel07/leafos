/**
  ******************************************************************************
  * @file    stm32746g_discovery_audio.c
  * @author  MCD Application Team
  * @version V1.1.1
  * @date    02-June-2016
  * @brief   This file provides the Audio driver for the STM32746G-Discovery board.
  @verbatim
    How To use this driver:
    -----------------------
       + This driver supports STM32F7xx devices on STM32746G-Discovery (MB1191) board.
       + Call the function BSP_AUDIO_OUT_Init(
                                        OutputDevice: physical output mode (OUTPUT_DEVICE_SPEAKER, 
                                                      OUTPUT_DEVICE_HEADPHONE or OUTPUT_DEVICE_BOTH)
                                        Volume      : Initial volume to be set (0 is min (mute), 100 is max (100%)
                                        AudioFreq   : Audio frequency in Hz (8000, 16000, 22500, 32000...)
                                                      this parameter is relative to the audio file/stream type.
                                       )
          This function configures all the hardware required for the audio application (codec, I2C, SAI, 
          GPIOs, DMA and interrupt if needed). This function returns AUDIO_OK if configuration is OK.
          If the returned value is different from AUDIO_OK or the function is stuck then the communication with
          the codec or the MFX has failed (try to un-plug the power or reset device in this case).
          - OUTPUT_DEVICE_SPEAKER  : only speaker will be set as output for the audio stream.
          - OUTPUT_DEVICE_HEADPHONE: only headphones will be set as output for the audio stream.
          - OUTPUT_DEVICE_BOTH     : both Speaker and Headphone are used as outputs for the audio stream
                                     at the same time.
          Note. On STM32746G-Discovery SAI_DMA is configured in CIRCULAR mode. Due to this the application
            does NOT need to call BSP_AUDIO_OUT_ChangeBuffer() to assure streaming.
       + Call the function BSP_DISCOVERY_AUDIO_OUT_Play(
                                      pBuffer: pointer to the audio data file address
                                      Size   : size of the buffer to be sent in Bytes
                                     )
          to start playing (for the first time) from the audio file/stream.
       + Call the function BSP_AUDIO_OUT_Pause() to pause playing   
       + Call the function BSP_AUDIO_OUT_Resume() to resume playing.
           Note. After calling BSP_AUDIO_OUT_Pause() function for pause, only BSP_AUDIO_OUT_Resume() should be called
              for resume (it is not allowed to call BSP_AUDIO_OUT_Play() in this case).
           Note. This function should be called only when the audio file is played or paused (not stopped).
       + For each mode, you may need to implement the relative callback functions into your code.
          The Callback functions are named AUDIO_OUT_XXX_CallBack() and only their prototypes are declared in 
          the stm32746g_discovery_audio.h file. (refer to the example for more details on the callbacks implementations)
       + To Stop playing, to modify the volume level, the frequency, the audio frame slot, 
          the device output mode the mute or the stop, use the functions: BSP_AUDIO_OUT_SetVolume(), 
          AUDIO_OUT_SetFrequency(), BSP_AUDIO_OUT_SetAudioFrameSlot(), BSP_AUDIO_OUT_SetOutputMode(),
          BSP_AUDIO_OUT_SetMute() and BSP_AUDIO_OUT_Stop().
       + The driver API and the callback functions are at the end of the stm32746g_discovery_audio.h file.
     
    Driver architecture:
    --------------------
       + This driver provides the High Audio Layer: consists of the function API exported in the stm32746g_discovery_audio.h file
         (BSP_AUDIO_OUT_Init(), BSP_AUDIO_OUT_Play() ...)
       + This driver provide also the Media Access Layer (MAL): which consists of functions allowing to access the media containing/
         providing the audio file/stream. These functions are also included as local functions into
         the stm32746g_discovery_audio_codec.c file (SAIx_Out_Init() and SAIx_Out_DeInit(), SAIx_In_Init() and SAIx_In_DeInit())
    
    Known Limitations:
    ------------------
       1- If the TDM Format used to play in parallel 2 audio Stream (the first Stream is configured in codec SLOT0 and second 
          Stream in SLOT1) the Pause/Resume, volume and mute feature will control the both streams.
       2- Parsing of audio file is not implemented (in order to determine audio file properties: Mono/Stereo, Data size, 
          File size, Audio Frequency, Audio Data header size ...). The configuration is fixed for the given audio file.
       3- Supports only Stereo audio streaming.
       4- Supports only 16-bits audio data size.
  @endverbatim  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#include <string.h>
#include "..\inc\if_apis.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif
#include "..\inc\if_audio.h"
#include "..\inc\wm8994\wm8994.h"

extern audio_driver wm8994_drv;
/* These function can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void  BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params);
void  BSP_AUDIO_OUT_MspInit(SAI_HandleTypeDef *hsai, void *Params);
void  BSP_AUDIO_OUT_MspDeInit(SAI_HandleTypeDef *hsai, void *Params);

/* These function can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void  BSP_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params);
void  BSP_AUDIO_IN_MspDeInit(SAI_HandleTypeDef *hsai, void *Params);

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32746G_DISCOVERY
  * @{
  */ 
  
/** @defgroup STM32746G_DISCOVERY_AUDIO STM32746G_DISCOVERY AUDIO
  * @brief This file includes the low layer driver for wm8994 Audio Codec
  *        available on STM32746G-Discovery board(MB1191).
  * @{
  */ 

/** @defgroup STM32746G_DISCOVERY_AUDIO_Private_Types STM32746G_DISCOVERY AUDIO Private Types
  * @{
  */ 
/**
  * @}
  */ 
  
/** @defgroup STM32746G_DISCOVERY_AUDIO_Private_Defines STM32746G_DISCOVERY AUDIO Private Defines
  * @{
  */
/**
  * @}
  */ 

/** @defgroup STM32746G_DISCOVERY_AUDIO_Private_Macros STM32746G_DISCOVERY AUDIO Private Macros
  * @{
  */
/**
  * @}
  */ 
  
/** @defgroup STM32746G_DISCOVERY_AUDIO_Private_Variables STM32746G_DISCOVERY AUDIO Private Variables
  * @{
  */
audio_driver          *audio_drv;
SAI_HandleTypeDef         haudio_out_sai={0};
SAI_HandleTypeDef         haudio_in_sai={0};
TIM_HandleTypeDef         haudio_tim;
static audio_handle_p			g_audio_ctx = NULL;

void AUDIO_IN_SAIx_DMAx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
}


uint16_t AudioInVolume = DEFAULT_AUDIO_IN_VOLUME;
    
/**
  * @}
  */ 

/** @defgroup STM32746G_DISCOVERY_AUDIO_Private_Function_Prototypes STM32746G_DISCOVERY AUDIO Private Function Prototypes
  * @{
  */
static void SAIx_Out_Init(uint32_t AudioFreq);
static void SAIx_Out_DeInit(void);
static void SAIx_In_Init(uint32_t SaiOutMode, uint32_t SlotActive, uint32_t AudioFreq);
static void SAIx_In_DeInit(void);
/**
  * @}
  */ 

/** @defgroup STM32746G_DISCOVERY_AUDIO_OUT_Exported_Functions STM32746G_DISCOVERY AUDIO Out Exported Functions
  * @{
  */ 

/**
  * @brief  Configures the audio peripherals.
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       or OUTPUT_DEVICE_BOTH.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   The I2S PLL input clock must be done in the user application.  
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{ 
  uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;

  /* Disable SAI */
  SAIx_Out_DeInit();

  /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
  BSP_AUDIO_OUT_ClockConfig(&haudio_out_sai, AudioFreq, NULL);
 
  /* SAI data transfer preparation:
  Prepare the Media to be used for the audio transfer from memory to SAI peripheral */
  haudio_out_sai.Instance = AUDIO_OUT_SAIx;
  if(HAL_SAI_GetState(&haudio_out_sai) == HAL_SAI_STATE_RESET)
  {
    /* Init the SAI MSP: this __weak function can be redefined by the application*/
    BSP_AUDIO_OUT_MspInit(&haudio_out_sai, NULL);
  }
  SAIx_Out_Init(AudioFreq);

  /* wm8994 codec initialization */
  deviceid = wm8994_drv.read_id(&wm8994_drv);
  
  if((deviceid) == WM8994_ID)
  {  
    /* Reset the Codec Registers */
    wm8994_drv.reset(&wm8994_drv);
    /* Initialize the audio driver structure */
    audio_drv = &wm8994_drv; 
    ret = AUDIO_OK;
  }
  else
  {
    ret = AUDIO_ERROR;
  }

  if(ret == AUDIO_OK)
  {
    /* Initialize the codec internal registers */
    audio_drv->init(audio_drv, OutputDevice, Volume, AudioFreq);
  }
 
  return ret;
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data in BYTES unit.
  *         In memory, first element is for left channel, second element is for right channel
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
  /* Call the audio Codec Play function */
  if(audio_drv->play(audio_drv, pBuffer, Size) != 0)
  {  
    return AUDIO_ERROR;
  }
  else
  {
    /* Update the Media layer and enable it for play */  
    HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) pBuffer, DMA_MAX(Size / AUDIODATA_SIZE));
    
    return AUDIO_OK;
  }
}

/**
  * @brief  Sends n-Bytes on the SAI interface.
  * @param  pData: pointer on data address 
  * @param  Size: number of data to be written
  * @retval None
  */
void BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size)
{
   HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) pData, Size);
}

/**
  * @brief  This function Pauses the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used.
  * @note When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play() 
  *          function for resume could lead to unexpected behaviour).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Pause(void)
{    
  /* Call the Audio Codec Pause/Resume function */
  if(audio_drv->pause(audio_drv) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Call the Media layer pause function */
    HAL_SAI_DMAPause(&haudio_out_sai);
    
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  This function  Resumes the audio file stream.  
  * @note When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play() 
  *          function for resume could lead to unexpected behaviour).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Resume(void)
{    
  /* Call the Audio Codec Pause/Resume function */
  if(audio_drv->resume(audio_drv) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Call the Media layer pause/resume function */
    HAL_SAI_DMAResume(&haudio_out_sai);
    
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Stops audio playing and Power down the Audio Codec. 
  * @param  Option: could be one of the following parameters 
  *           - CODEC_PDWN_SW: for software power off (by writing registers). 
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically). 
  *                            Then need to reconfigure the Codec after power on.  
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
  /* Call the Media layer stop function */
  HAL_SAI_DMAStop(&haudio_out_sai);
  
  /* Call Audio Codec Stop function */
  if(audio_drv->stop(audio_drv, Option) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    if(Option == CODEC_PDWN_HW)
    { 
      /* Wait at least 100us */
      HAL_Delay(1);
    }
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Controls the current audio volume level. 
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *         Mute and 100 for Max volume level).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  if(audio_drv->set_volume(audio_drv, Volume) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Enables or disables the MUTE mode by software 
  * @param  Cmd: Could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to 
  *         unmute the codec and restore previous volume level.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{ 
  /* Call the Codec Mute function */
  if(audio_drv->set_mute(audio_drv, Cmd) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @param  Output: The audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE or OUTPUT_DEVICE_BOTH
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetOutputMode(uint8_t Output)
{
  /* Call the Codec output device function */
  if(audio_drv->set_output_mode(audio_drv, Output) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Updates the audio frequency.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   This API should be called after the BSP_AUDIO_OUT_Init() to adjust the
  *         audio frequency.
  * @retval None
  */
void BSP_AUDIO_OUT_SetFrequency(uint32_t AudioFreq)
{ 
  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */ 
  BSP_AUDIO_OUT_ClockConfig(&haudio_out_sai, AudioFreq, NULL);

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);
  
  /* Update the SAI audio frequency configuration */
  haudio_out_sai.Init.AudioFrequency = AudioFreq;
  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);
}

/**
  * @brief  Updates the Audio frame slot configuration.
  * @param  AudioFrameSlot: specifies the audio Frame slot
  *         This parameter can be one of the following values
  *            @arg CODEC_AUDIOFRAME_SLOT_0123
  *            @arg CODEC_AUDIOFRAME_SLOT_02
  *            @arg CODEC_AUDIOFRAME_SLOT_13
  * @note   This API should be called after the BSP_AUDIO_OUT_Init() to adjust the
  *         audio frame slot.
  * @retval None
  */
void BSP_AUDIO_OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot)
{ 
  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);
  
  /* Update the SAI audio frame slot configuration */
  haudio_out_sai.SlotInit.SlotActive = AudioFrameSlot;
  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);
}

/**
  * @brief  Deinit the audio peripherals.
  * @retval None
  */
void BSP_AUDIO_OUT_DeInit(void)
{
  SAIx_Out_DeInit();
  /* DeInit the SAI MSP : this __weak function can be rewritten by the application */
  BSP_AUDIO_OUT_MspDeInit(&haudio_out_sai, NULL);
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
  BSP_AUDIO_OUT_TransferComplete_CallBack();
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
  BSP_AUDIO_OUT_HalfTransfer_CallBack();
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  HAL_SAI_StateTypeDef audio_out_state;
  HAL_SAI_StateTypeDef audio_in_state;

  audio_out_state = HAL_SAI_GetState(&haudio_out_sai);
  audio_in_state = HAL_SAI_GetState(&haudio_in_sai);

  /* Determines if it is an audio out or audio in error */
  if ((audio_out_state == HAL_SAI_STATE_BUSY) || (audio_out_state == HAL_SAI_STATE_BUSY_TX))
  {
    BSP_AUDIO_OUT_Error_CallBack();
  }

  if ((audio_in_state == HAL_SAI_STATE_BUSY) || (audio_in_state == HAL_SAI_STATE_BUSY_RX))
  {
    BSP_AUDIO_IN_Error_CallBack();
  }
}

/**
  * @brief  Manages the DMA full Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	uint16 i = 0;
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{ 
	uint16 i =0;
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_Error_CallBack(void)
{
}

/**
  * @brief  Initializes BSP_AUDIO_OUT MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
__weak void BSP_AUDIO_OUT_MspInit(SAI_HandleTypeDef *hsai, void *Params)
{ 
  static DMA_HandleTypeDef hdma_sai_tx;
  GPIO_InitTypeDef  gpio_init_structure;  

  /* Enable SAI clock */
  AUDIO_OUT_SAIx_CLK_ENABLE();
  
  /* Enable GPIO clock */
  AUDIO_OUT_SAIx_MCLK_ENABLE();
  AUDIO_OUT_SAIx_SCK_SD_ENABLE();
  AUDIO_OUT_SAIx_FS_ENABLE();
  /* CODEC_SAI pins configuration: FS, SCK, MCK and SD pins ------------------*/
  gpio_init_structure.Pin = AUDIO_OUT_SAIx_FS_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = AUDIO_OUT_SAIx_FS_SD_MCLK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAIx_FS_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = AUDIO_OUT_SAIx_SCK_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = AUDIO_OUT_SAIx_SCK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAIx_SCK_SD_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin =  AUDIO_OUT_SAIx_SD_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = AUDIO_OUT_SAIx_FS_SD_MCLK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAIx_SCK_SD_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = AUDIO_OUT_SAIx_MCLK_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = AUDIO_OUT_SAIx_FS_SD_MCLK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAIx_MCLK_GPIO_PORT, &gpio_init_structure);

  /* Enable the DMA clock */
  AUDIO_OUT_SAIx_DMAx_CLK_ENABLE();
    
  if(hsai->Instance == AUDIO_OUT_SAIx)
  {
    /* Configure the hdma_saiTx handle parameters */   
    hdma_sai_tx.Init.Channel             = AUDIO_OUT_SAIx_DMAx_CHANNEL;
    hdma_sai_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_sai_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_tx.Init.PeriphDataAlignment = AUDIO_OUT_SAIx_DMAx_PERIPH_DATA_SIZE;
    hdma_sai_tx.Init.MemDataAlignment    = AUDIO_OUT_SAIx_DMAx_MEM_DATA_SIZE;
    hdma_sai_tx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;         
    hdma_sai_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
    
    hdma_sai_tx.Instance = AUDIO_OUT_SAIx_DMAx_STREAM;
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmatx, hdma_sai_tx);
    
    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_tx);
    
    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_sai_tx);      
  }
  
  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_OUT_SAIx_DMAx_IRQ, AUDIO_OUT_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_OUT_SAIx_DMAx_IRQ); 
}

/**
  * @brief  Deinitializes SAI MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
__weak void BSP_AUDIO_OUT_MspDeInit(SAI_HandleTypeDef *hsai, void *Params)
{
    GPIO_InitTypeDef  gpio_init_structure;

    /* SAI DMA IRQ Channel deactivation */
    HAL_NVIC_DisableIRQ(AUDIO_OUT_SAIx_DMAx_IRQ);

    if(hsai->Instance == AUDIO_OUT_SAIx)
    {
      /* Deinitialize the DMA stream */
      HAL_DMA_DeInit(hsai->hdmatx);
    }

    /* Disable SAI peripheral */
    __HAL_SAI_DISABLE(hsai);  

    /* Deactives CODEC_SAI pins FS, SCK, MCK and SD by putting them in input mode */
    gpio_init_structure.Pin = AUDIO_OUT_SAIx_FS_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_FS_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin = AUDIO_OUT_SAIx_SCK_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_SCK_SD_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin =  AUDIO_OUT_SAIx_SD_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_SCK_SD_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin = AUDIO_OUT_SAIx_MCLK_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_MCLK_GPIO_PORT, gpio_init_structure.Pin);
  
    /* Disable SAI clock */
    AUDIO_OUT_SAIx_CLK_DISABLE();

    /* GPIO pins clock and DMA clock can be shut down in the application
       by surcharging this __weak function */
}

/**
  * @brief  Clock Config.
  * @param  hsai: might be required to set audio peripheral predivider if any.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Params  
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application     
  * @retval None
  */
__weak void BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params)
{ 
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);
  
  /* Set the PLL configuration according to the audio frequency */
  if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K))
  {
    /* Configure PLLI2S prescalers */
    /* PLLI2S_VCO: VCO_429M
    I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 429/2 = 214.5 Mhz
    I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 214.5/19 = 11.289 Mhz */
    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    rcc_ex_clk_init_struct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SN = 429;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SQ = 2;
    rcc_ex_clk_init_struct.PLLI2SDivQ = 19;
    
    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
    
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */
  {
    /* I2S clock config
    PLLI2S_VCO: VCO_344M
    I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz
    I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */
    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    rcc_ex_clk_init_struct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SN = 344;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SQ = 7;
    rcc_ex_clk_init_struct.PLLI2SDivQ = 1;
    
    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
  }
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initializes the output Audio Codec audio interface (SAI).
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @note   The default SlotActive configuration is set to CODEC_AUDIOFRAME_SLOT_0123 
  *         and user can update this configuration using 
  * @retval None
  */
static void SAIx_Out_Init(uint32_t AudioFreq)
{
  /* Initialize the haudio_out_sai Instance parameter */
  haudio_out_sai.Instance = AUDIO_OUT_SAIx;
  
  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);
  
  /* Configure SAI_Block_x 
  LSBFirst: Disabled 
  DataSize: 16 */
  haudio_out_sai.Init.AudioFrequency = AudioFreq;
  haudio_out_sai.Init.AudioMode = SAI_MODEMASTER_TX;
  haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
  haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
  haudio_out_sai.Init.DataSize = SAI_DATASIZE_16;
  haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
  haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
  haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
  haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLED;
  haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
  
  /* Configure SAI_Block_x Frame 
  Frame Length: 64
  Frame active Length: 32
  FS Definition: Start frame + Channel Side identification
  FS Polarity: FS active Low
  FS Offset: FS asserted one bit before the first bit of slot 0 */ 
  haudio_out_sai.FrameInit.FrameLength = 64; 
  haudio_out_sai.FrameInit.ActiveFrameLength = 32;
  haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
  
  /* Configure SAI Block_x Slot 
  Slot First Bit Offset: 0
  Slot Size  : 16
  Slot Number: 4
  Slot Active: All slot actives */
  haudio_out_sai.SlotInit.FirstBitOffset = 0;
  haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  haudio_out_sai.SlotInit.SlotNumber = 4; 
  haudio_out_sai.SlotInit.SlotActive = CODEC_AUDIOFRAME_SLOT_0123;

  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);
}



/**
  * @brief  Deinitializes the output Audio Codec audio interface (SAI).
  * @retval None
  */
static void SAIx_Out_DeInit(void)
{
  /* Initialize the haudio_out_sai Instance parameter */
  haudio_out_sai.Instance = AUDIO_OUT_SAIx;

  /* Disable SAI peripheral */
  __HAL_SAI_DISABLE(&haudio_out_sai);

  HAL_SAI_DeInit(&haudio_out_sai);
}

/**
  * @}
  */

/** @defgroup STM32746G_DISCOVERY_AUDIO_Out_Private_Functions STM32746G_DISCOVERY_AUDIO Out Private Functions
  * @{
  */ 
  
/**
  * @brief  Initializes wave recording.
  * @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2 or INPUT_DEVICE_INPUT_LINE_1
  * @param  Volume: Initial volume level (in range 0(Mute)..80(+0dB)..100(+17.625dB))
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Init(uint16_t InputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;
  uint32_t slot_active;

  if ((InputDevice != INPUT_DEVICE_INPUT_LINE_1) &&       /* Only INPUT_LINE_1 and MICROPHONE_2 inputs supported */
      (InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2))
  {
    ret = AUDIO_ERROR;
  }
  else
  {
    /* Disable SAI */
    SAIx_In_DeInit();

    /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
    BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

    /* SAI data transfer preparation:
    Prepare the Media to be used for the audio transfer from SAI peripheral to memory */
    haudio_in_sai.Instance = AUDIO_IN_SAIx;
    if(HAL_SAI_GetState(&haudio_in_sai) == HAL_SAI_STATE_RESET)
    {
      /* Init the SAI MSP: this __weak function can be redefined by the application*/
      BSP_AUDIO_OUT_MspInit(&haudio_in_sai, NULL);  /* Initialize GPIOs for SAI2 block A Master signals */
      BSP_AUDIO_IN_MspInit(&haudio_in_sai, NULL);
    }

    /* Configure SAI in master RX mode :
     *   - SAI2_block_A in master RX mode
     *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
     */
    if (InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2)
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_13;
    }
    else
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_02;
    }
    SAIx_In_Init(SAI_MODEMASTER_RX, slot_active, AudioFreq);

    /* wm8994 codec initialization */
    deviceid = wm8994_drv.read_id(&wm8994_drv);

    if((deviceid) == WM8994_ID)
    {
      /* Reset the Codec Registers */
      wm8994_drv.reset(&wm8994_drv);
      /* Initialize the audio driver structure */
      audio_drv = &wm8994_drv;
      ret = AUDIO_OK;
    }
    else
    {
      ret = AUDIO_ERROR;
    }

    if(ret == AUDIO_OK)
    {
      /* Initialize the codec internal registers */
      audio_drv->init(audio_drv, InputDevice, Volume, AudioFreq);
    }
  }
  return ret;
}

/**
  * @brief  Initializes wave recording and playback in parallel.
  * @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       or OUTPUT_DEVICE_BOTH.
  * @param  Volume: Initial volume level (in range 0(Mute)..80(+0dB)..100(+17.625dB))
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_OUT_Init(uint16_t InputDevice, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;
  uint32_t slot_active;

  if (InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2)  /* Only MICROPHONE_2 input supported */
  {
    ret = AUDIO_ERROR;
  }
  else
  {
    /* Disable SAI */
    SAIx_In_DeInit();
    SAIx_Out_DeInit();

    /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
    BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

    /* SAI data transfer preparation:
    Prepare the Media to be used for the audio transfer from SAI peripheral to memory */
    haudio_in_sai.Instance = AUDIO_IN_SAIx;
    if(HAL_SAI_GetState(&haudio_in_sai) == HAL_SAI_STATE_RESET)
    {
      /* Init the SAI MSP: this __weak function can be redefined by the application*/
      BSP_AUDIO_IN_MspInit(&haudio_in_sai, NULL);
    }

    /* SAI data transfer preparation:
    Prepare the Media to be used for the audio transfer from memory to SAI peripheral */
    haudio_out_sai.Instance = AUDIO_OUT_SAIx;
    if(HAL_SAI_GetState(&haudio_out_sai) == HAL_SAI_STATE_RESET)
    {
      /* Init the SAI MSP: this __weak function can be redefined by the application*/
      BSP_AUDIO_OUT_MspInit(&haudio_out_sai, NULL);
    }

    /* Configure SAI in master mode :
     *   - SAI2_block_A in master TX mode
     *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
     */
    if (InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2)
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_13;
    }
    else
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_02;
    }
    SAIx_In_Init(SAI_MODEMASTER_TX, slot_active, AudioFreq);

    /* wm8994 codec initialization */
    deviceid = wm8994_drv.read_id(&wm8994_drv);

    if((deviceid) == WM8994_ID)
    {
      /* Reset the Codec Registers */
      wm8994_drv.reset(&wm8994_drv);
      /* Initialize the audio driver structure */
      audio_drv = &wm8994_drv;
      ret = AUDIO_OK;
    }
    else
    {
      ret = AUDIO_ERROR;
    }

    if(ret == AUDIO_OK)
    {
      /* Initialize the codec internal registers */
      audio_drv->init(audio_drv, InputDevice | OutputDevice, Volume, AudioFreq);
    }
  }
  return ret;
}


/**
  * @brief  Starts audio recording.
  * @param  pbuf: Main buffer pointer for the recorded data storing  
  * @param  size: size of the recorded buffer in number of elements (typically number of half-words)
  *               Be careful that it is not the same unit than BSP_AUDIO_OUT_Play function
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t  BSP_AUDIO_IN_Record(uint16_t* pbuf, uint32_t size)
{
  uint32_t ret = AUDIO_ERROR;
  
  /* Start the process receive DMA */
  HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pbuf, size);
  
  /* Return AUDIO_OK when all operations are correctly done */
  ret = AUDIO_OK;
  
  return ret;
}

/**
  * @brief  Stops audio recording.
  * @param  Option: could be one of the following parameters
  *           - CODEC_PDWN_SW: for software power off (by writing registers).
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically).
  *                            Then need to reconfigure the Codec after power on.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Stop(uint32_t Option)
{
  /* Call the Media layer stop function */
  HAL_SAI_DMAStop(&haudio_in_sai);
  
  /* Call Audio Codec Stop function */
  if(audio_drv->stop(audio_drv, Option) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    if(Option == CODEC_PDWN_HW)
    {
      /* Wait at least 100us */
      HAL_Delay(1);
    }
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Pauses the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Pause(void)
{    
  /* Call the Media layer pause function */
  HAL_SAI_DMAPause(&haudio_in_sai);
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}

/**
  * @brief  Resumes the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Resume(void)
{    
  /* Call the Media layer pause/resume function */
  HAL_SAI_DMAResume(&haudio_in_sai);
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}

/**
  * @brief  Controls the audio in volume level. 
  * @param  Volume: Volume level in range 0(Mute)..80(+0dB)..100(+17.625dB)
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  if(audio_drv->set_volume(audio_drv, Volume) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Set the Global variable AudioInVolume  */
    AudioInVolume = Volume;
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Deinit the audio IN peripherals.
  * @retval None
  */
void BSP_AUDIO_IN_DeInit(void)
{
  SAIx_In_DeInit();
  /* DeInit the SAI MSP : this __weak function can be rewritten by the application */
  BSP_AUDIO_IN_MspDeInit(&haudio_in_sai, NULL);
}

 /**
  * @brief  Rx Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
  BSP_AUDIO_IN_TransferComplete_CallBack();
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
  BSP_AUDIO_IN_HalfTransfer_CallBack();
}

/**
  * @brief  Audio IN Error callback function.
  * @retval None
  */
__weak void BSP_AUDIO_IN_Error_CallBack(void)
{   
  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
}

/**
  * @brief  Initializes BSP_AUDIO_IN MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
__weak void BSP_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params)
{
  static DMA_HandleTypeDef hdma_sai_rx;
  GPIO_InitTypeDef  gpio_init_structure;  

  /* Enable SAI clock */
  AUDIO_IN_SAIx_CLK_ENABLE();
  
  /* Enable SD GPIO clock */
  AUDIO_IN_SAIx_SD_ENABLE();
  /* CODEC_SAI pin configuration: SD pin */
  gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  gpio_init_structure.Alternate = AUDIO_IN_SAIx_SD_AF;
  HAL_GPIO_Init(AUDIO_IN_SAIx_SD_GPIO_PORT, &gpio_init_structure);

  /* Enable Audio INT GPIO clock */
  AUDIO_IN_INT_GPIO_ENABLE();
  /* Audio INT pin configuration: input */
  gpio_init_structure.Pin = AUDIO_IN_INT_GPIO_PIN;
  gpio_init_structure.Mode = GPIO_MODE_INPUT;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(AUDIO_IN_INT_GPIO_PORT, &gpio_init_structure);

  /* Enable the DMA clock */
  AUDIO_IN_SAIx_DMAx_CLK_ENABLE();
    
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* Configure the hdma_sai_rx handle parameters */
    hdma_sai_rx.Init.Channel             = AUDIO_IN_SAIx_DMAx_CHANNEL;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = AUDIO_IN_SAIx_DMAx_PERIPH_DATA_SIZE;
    hdma_sai_rx.Init.MemDataAlignment    = AUDIO_IN_SAIx_DMAx_MEM_DATA_SIZE;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    
    hdma_sai_rx.Instance = AUDIO_IN_SAIx_DMAx_STREAM;
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hdma_sai_rx);
    
    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_rx);
    
    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_sai_rx);
  }
  
  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_IN_SAIx_DMAx_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

  /* Audio INT IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_IN_INT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_IN_INT_IRQ);
}

/**
  * @brief  DeInitializes BSP_AUDIO_IN MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
__weak void BSP_AUDIO_IN_MspDeInit(SAI_HandleTypeDef *hsai, void *Params)
{
  GPIO_InitTypeDef  gpio_init_structure;

  static DMA_HandleTypeDef hdma_sai_rx;

  /* SAI IN DMA IRQ Channel deactivation */
  HAL_NVIC_DisableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_rx);
  }

 /* Disable SAI block */
  __HAL_SAI_DISABLE(hsai);

  /* Disable pin: SD pin */
  gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
  HAL_GPIO_DeInit(AUDIO_IN_SAIx_SD_GPIO_PORT, gpio_init_structure.Pin);

  /* Disable SAI clock */
  AUDIO_IN_SAIx_CLK_DISABLE();

  /* GPIO pins clock and DMA clock can be shut down in the application
     by surcharging this __weak function */
}


/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initializes the input Audio Codec audio interface (SAI).
  * @param  SaiOutMode: SAI_MODEMASTER_TX (for record and playback in parallel)
  *                     or SAI_MODEMASTER_RX (for record only).
  * @param  SlotActive: CODEC_AUDIOFRAME_SLOT_02 or CODEC_AUDIOFRAME_SLOT_13
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval None
  */
static void SAIx_In_Init(uint32_t SaiOutMode, uint32_t SlotActive, uint32_t AudioFreq)
{
  /* Initialize SAI2 block A in MASTER RX */
  /* Initialize the haudio_out_sai Instance parameter */
  haudio_out_sai.Instance = AUDIO_OUT_SAIx;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);

  /* Configure SAI_Block_x
  LSBFirst: Disabled
  DataSize: 16 */
  haudio_out_sai.Init.AudioFrequency = AudioFreq;
  haudio_out_sai.Init.AudioMode = SaiOutMode;
  haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
  haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
  haudio_out_sai.Init.DataSize = SAI_DATASIZE_16;
  haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
  haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
  haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
  haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLED;
  haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;

  /* Configure SAI_Block_x Frame
  Frame Length: 64
  Frame active Length: 32
  FS Definition: Start frame + Channel Side identification
  FS Polarity: FS active Low
  FS Offset: FS asserted one bit before the first bit of slot 0 */
  haudio_out_sai.FrameInit.FrameLength = 64;
  haudio_out_sai.FrameInit.ActiveFrameLength = 32;
  haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

  /* Configure SAI Block_x Slot
  Slot First Bit Offset: 0
  Slot Size  : 16
  Slot Number: 4
  Slot Active: All slot actives */
  haudio_out_sai.SlotInit.FirstBitOffset = 0;
  haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  haudio_out_sai.SlotInit.SlotNumber = 4;
  haudio_out_sai.SlotInit.SlotActive = SlotActive;

  HAL_SAI_Init(&haudio_out_sai);

  /* Initialize SAI2 block B in SLAVE RX synchronous from SAI2 block A */
  /* Initialize the haudio_in_sai Instance parameter */
  haudio_in_sai.Instance = AUDIO_IN_SAIx;
  
  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_in_sai);
  
  /* Configure SAI_Block_x
  LSBFirst: Disabled
  DataSize: 16 */
  haudio_in_sai.Init.AudioFrequency = AudioFreq;
  haudio_in_sai.Init.AudioMode = SAI_MODESLAVE_RX;
  haudio_in_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
  haudio_in_sai.Init.Protocol = SAI_FREE_PROTOCOL;
  haudio_in_sai.Init.DataSize = SAI_DATASIZE_16;
  haudio_in_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
  haudio_in_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
  haudio_in_sai.Init.Synchro = SAI_SYNCHRONOUS;
  haudio_in_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLED;
  haudio_in_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
  
  /* Configure SAI_Block_x Frame
  Frame Length: 64
  Frame active Length: 32
  FS Definition: Start frame + Channel Side identification
  FS Polarity: FS active Low
  FS Offset: FS asserted one bit before the first bit of slot 0 */
  haudio_in_sai.FrameInit.FrameLength = 64;
  haudio_in_sai.FrameInit.ActiveFrameLength = 32;
  haudio_in_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  haudio_in_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  haudio_in_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
  
  /* Configure SAI Block_x Slot
  Slot First Bit Offset: 0
  Slot Size  : 16
  Slot Number: 4
  Slot Active: All slot active */
  haudio_in_sai.SlotInit.FirstBitOffset = 0;
  haudio_in_sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  haudio_in_sai.SlotInit.SlotNumber = 4;
  haudio_in_sai.SlotInit.SlotActive = SlotActive;

  HAL_SAI_Init(&haudio_in_sai);

  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);

  /* Enable SAI peripheral */
  __HAL_SAI_ENABLE(&haudio_in_sai);
}



/**
  * @brief  Deinitializes the output Audio Codec audio interface (SAI).
  * @retval None
  */
static void SAIx_In_DeInit(void)
{
  /* Initialize the haudio_in_sai Instance parameter */
  haudio_in_sai.Instance = AUDIO_IN_SAIx;

  /* Disable SAI peripheral */
  __HAL_SAI_DISABLE(&haudio_in_sai);

  HAL_SAI_DeInit(&haudio_in_sai);
}

#define AUDIO_CMD_READ			0x01
#define AUDIO_CMD_WRITE			0x02

typedef struct audio_command audio_command;
struct audio_command {
	uint8 cmd;
	uint16 * buffer;
	uint32 index;
	uint32 size;
} ;
uint8 g_audio_in_state = 0;


void audio_dispatch_in_request(os_task * task, uint8 state) {
	os_message * msg;
	audio_command * cmd = NULL;
	os_message * to_enqueue = NULL;
	os_message * iterator;
	uint32 num_of_msg = 0;
	uint8 num_of_channel = 1;
	uint32 reslen = 0;
	audio_handle * handle = (audio_handle *)g_audio_ctx;
	num_of_channel = 0;
	
}

/**
  * @brief  User callback when record buffer is filled.
  * @retval None
  */
__weak void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
	uint16 i = 0;
	//BSP_AUDIO_OUT_ChangeBuffer
	g_audio_in_state = 1;
	os_task * audio_task = os_find_task_by_name("audio_task");
	if(audio_task != NULL) {
		//os_resume(audio_task);
		//audio_dispatch_in_request(audio_task, 2);
	}
	//HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*) g_audio_ctx->buffer_in , AUDIO_BUFFER_SIZE);
  //BSP_AUDIO_IN_Stop(CODEC_PDWN_SW); 
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{ 
  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
	uint16 i = 0;
	g_audio_in_state = 1;
	os_task * audio_task = os_find_task_by_name("audio_task");
	if(audio_task != NULL) {
		//os_resume(audio_task);
		//audio_dispatch_in_request(audio_task, 1);
	}
	//BSP_AUDIO_IN_TransferComplete_CallBack
	//HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*) g_audio_ctx->buffer_in + (AUDIO_BUFFER_SIZE / 2), AUDIO_BUFFER_SIZE / 2);
  BSP_AUDIO_IN_Stop(CODEC_PDWN_SW); 
}


static void audio_task()
{
  uint32_t numOfReadBytes, numOfWrittenBytes;    
	os_message * msg;
	uint8 i;
	audio_command * cmd = NULL;
	uint8 num_of_channel = 1;
	uint32 reslen = 0;
	audio_handle * handle = (audio_handle *)os_get_context();
	os_message * channel_msgs[8];
	uint8 audio_out_state = 0;
	os_message * to_enqueue = NULL;
	os_message * iterator;
  //osEvent event;  
	//g_audio_in_state = 0;
	HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t *)handle->buffer_in, DMA_MAX(AUDIO_BUFFER_SIZE));
  for(;;)
  {
		//read from device (audio in)
		//BSP_AUDIO_IN_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, handle->volume_in, handle->freq_in);		//initialize input with specified volume and sampling frequency
		//BSP_AUDIO_IN_Resume();
		//os_suspend();
		//HAL_SAI_Receive_IT(
		//BSP_AUDIO_IN_Pause();
		
		//g_audio_in_state = 0;
		//to_enqueue = NULL;
		//while(g_audio_in_state == 0) { os_suspend(); }
		num_of_channel = 0;
		while(1) {
			msg = os_dequeue_message();
			if(msg == NULL) break;
			if(msg->reqlen != sizeof(audio_command)) continue;
			cmd = (audio_command * )msg->request;
			
			//event->
			//if( event->request[0] == osEventMessage )
			if(cmd->cmd == AUDIO_CMD_WRITE)
			{
				if(num_of_channel < 8) {
					//channel_msgs[num_of_channel] = msg;
					num_of_channel++;
				} else {
					msg->flag |= OS_MESSAGE_SERVICED;
					os_dispatch_reply(msg);
				}
			}
			/* Recording .... */
			if(cmd->cmd == AUDIO_CMD_READ)
			{
				//os_wait(25);
				uint32 buf_left = cmd->size - cmd->index;
				//g_audio_in_state = 0;
				//os_wait(25);
				reslen = (cmd->size > AUDIO_BUFFER_SIZE)? AUDIO_BUFFER_SIZE: cmd->size;
				dma_memcpy(msg->response, handle->buffer_in, reslen);
				cmd->index += reslen;
				msg->reslen = cmd->size;
				msg->flag |= OS_MESSAGE_SERVICED;
				os_dispatch_reply(msg);
			}
		}
		if(g_audio_in_state == 1) {
			g_audio_in_state = 0;
		HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t *)handle->buffer_in, DMA_MAX(AUDIO_BUFFER_SIZE));
		}
		/*
	//requeue messages
	iterator = to_enqueue;
	while(iterator != NULL) {
		msg = iterator;
		iterator = iterator->next;
		msg->next = NULL;
		//os_queue_message(task, msg);
	}*/
		/*
		//}
		//interlave all channel into single buffer before sending to device
		for(i=0;i<num_of_channel;i++) {
			msg = channel_msgs[i];
			cmd = (audio_command * )msg->request;
			
			os_dispatch_reply(msg);
		}
		
		if(num_of_channel > 0) {
			//send to device (audio out), interleaved
			if(audio_out_state == 0) {
				BSP_AUDIO_OUT_Play(handle->buffer_out, DMA_MAX(AUDIO_SAMPLING_FREQ));
				audio_out_state = 1;
			}
			HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) handle->buffer_out, DMA_MAX(AUDIO_SAMPLING_FREQ));
		} else {
			if(audio_out_state == 1) {
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);			//stop output (set to standby mode)
				audio_out_state = 0;
			}
			//BSP_AUDIO_OUT_Pause();
		}
		*/
		//wait 
		
		os_wait(16);

  }
}

static void if_audio_init_device(audio_handle * handle) {
	g_audio_ctx = handle;
	//BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, OUTPUT_DEVICE_AUTO, handle->volume_in, handle->freq_in);
	//BSP_AUDIO_IN_Init(INPUT_DEVICE_INPUT_LINE_1, handle->volume_in, handle->freq_in);		//initialize input with specified volume and sampling frequency
	BSP_AUDIO_IN_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, handle->volume_in * 2, handle->freq_in);		//initialize input with specified volume and sampling frequency
  BSP_AUDIO_IN_Record((uint16_t*)handle->buffer_in, AUDIO_BUFFER_SIZE/2);
  //BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, handle->volume_out, handle->freq_out);			//initialize output with specified volume and sampling frequency
  //BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
}

static uint32 if_audio_read(audio_handle * handle, int16 * buffer, uint32 bytes) {
	uint32 reslen = 0;
	audio_command * cmd = os_alloc(sizeof(audio_command));
	if(cmd != NULL) {
		cmd->cmd = AUDIO_CMD_READ;
		cmd->buffer = (void *)buffer;
		cmd->index = 0;
		cmd->size = bytes;
		os_message * msg = os_send_message(handle->audio_task, os_create_message(handle, cmd, sizeof(audio_command), buffer));
		while((msg->flag & OS_MESSAGE_SERVICED) == 0) { 
			os_suspend();
			__nop();
			//os_wait(100); 
		}			//wait for line in
		reslen = msg->reslen;
		//memcpy(buffer, msg->response, msg->reslen);
		os_delete_message(msg);
		//os_free(cmd);
	}
	return reslen;
}

static void if_audio_write(audio_handle * handle, int16 * buffer, uint32 bytes) {
	os_message * msg;
	audio_command * cmd = os_alloc(sizeof(audio_command));
	if(cmd != NULL) {
		cmd->cmd = AUDIO_CMD_WRITE;
		cmd->buffer = (void *)buffer;
		cmd->index = 0;
		cmd->size = bytes;
		msg = os_send_message(handle->audio_task, os_create_message(handle, cmd, sizeof(audio_command), NULL));
		while((msg->flag & OS_MESSAGE_SERVICED) == 0) { os_wait(100); }			//wait for line out
		os_delete_message(msg);
		os_free(cmd);
	}
}

void if_audio_init(audio_handle * handle) {
	os_task * task = NULL;
	handle->driver = &wm8994_drv;
	handle->volume_in = DEFAULT_AUDIO_IN_VOLUME;
	handle->volume_out = DEFAULT_AUDIO_IN_VOLUME;
	handle->freq_in = AUDIO_SAMPLING_FREQ;
	handle->freq_out = AUDIO_SAMPLING_FREQ;
	handle->buffer_in_head = 0;
	handle->buffer_in_tail = 0;
	handle->buffer_out_head = 0;
	handle->buffer_out_tail = 0;
	__HAL_RCC_DMA2_CLK_ENABLE();
	task = os_find_task_by_name("audio_task");
	if(task == NULL) {
		task = os_create_task(handle, audio_task, "audio_task", 16, 2048);
	}
	if(task != NULL) {
		handle->audio_task = task;
		handle->init = if_audio_init_device;
		handle->read = if_audio_read;
		handle->write = if_audio_write;
		
		handle->init(handle);			//start initializing device
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
