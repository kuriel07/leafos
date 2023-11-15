;******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
;* File Name          : startup_stm32f746xx.s
;* Author             : MCD Application Team
;* Description        : STM32F746xx devices vector table for MDK-ARM toolchain. 
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Set the initial PC == Reset_Handler
;*                      - Set the vector table entries with the exceptions ISR address
;*                      - Branches to __main in the C library (which eventually
;*                        calls main()).
;*                      After Reset the CortexM7 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
;* <<< Use Configuration Wizard in Context Menu >>>   
;*******************************************************************************
; 
;* Redistribution and use in source and binary forms, with or without modification,
;* are permitted provided that the following conditions are met:
;*   1. Redistributions of source code must retain the above copyright notice,
;*      this list of conditions and the following disclaimer.
;*   2. Redistributions in binary form must reproduce the above copyright notice,
;*      this list of conditions and the following disclaimer in the documentation
;*      and/or other materials provided with the distribution.
;*   3. Neither the name of STMicroelectronics nor the names of its contributors
;*      may be used to endorse or promote products derived from this software
;*      without specific prior written permission.
;*
;* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
;* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
;* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
;* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
;* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
;* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
; 
;*******************************************************************************

; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size		EQU     0x00001000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size      EQU     0x00600000			;6MB out of 8MB
			
                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
					
__heap_base  	EQU		0xC0000000		; start of the heap
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               ; Top of Stack
                DCD     BReset_Handler              ; Reset Handler
                DCD     _NMI_Handler                ; NMI Handler
                DCD     _HardFault_Handler          ; Hard Fault Handler
                DCD     _MemManage_Handler          ; MPU Fault Handler
                DCD     _BusFault_Handler           ; Bus Fault Handler
                DCD     _UsageFault_Handler         ; Usage Fault Handler
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     _SVC_Handler                ; SVCall Handler
                DCD     _DebugMon_Handler           ; Debug Monitor Handler
                DCD     0                          ; Reserved
                DCD     _PendSV_Handler             ; PendSV Handler
                DCD     _SysTick_Handler            ; SysTick Handler

                ; External Interrupts
                DCD     _WWDG_IRQHandler                   ; Window WatchDog                                        
                DCD     _PVD_IRQHandler                    ; PVD through EXTI Line detection                        
                DCD     _TAMP_STAMP_IRQHandler             ; Tamper and TimeStamps through the EXTI line            
                DCD     _RTC_WKUP_IRQHandler               ; RTC Wakeup through the EXTI line                       
                DCD     _FLASH_IRQHandler                  ; FLASH                                           
                DCD     _RCC_IRQHandler                    ; RCC                                             
                DCD     _EXTI0_IRQHandler                  ; EXTI Line0                                             
                DCD     _EXTI1_IRQHandler                  ; EXTI Line1                                             
                DCD     _EXTI2_IRQHandler                  ; EXTI Line2                                             
                DCD     _EXTI3_IRQHandler                  ; EXTI Line3                                             
                DCD     _EXTI4_IRQHandler                  ; EXTI Line4                                             
                DCD     _DMA1_Stream0_IRQHandler           ; DMA1 Stream 0                                   
                DCD     _DMA1_Stream1_IRQHandler           ; DMA1 Stream 1                                   
                DCD     _DMA1_Stream2_IRQHandler           ; DMA1 Stream 2                                   
                DCD     _DMA1_Stream3_IRQHandler           ; DMA1 Stream 3                                   
                DCD     _DMA1_Stream4_IRQHandler           ; DMA1 Stream 4                                   
                DCD     _DMA1_Stream5_IRQHandler           ; DMA1 Stream 5                                   
                DCD     _DMA1_Stream6_IRQHandler           ; DMA1 Stream 6                                   
                DCD     _ADC_IRQHandler                    ; ADC1, ADC2 and ADC3s                            
                DCD     _CAN1_TX_IRQHandler                ; CAN1 TX                                                
                DCD     _CAN1_RX0_IRQHandler               ; CAN1 RX0                                               
                DCD     _CAN1_RX1_IRQHandler               ; CAN1 RX1                                               
                DCD     _CAN1_SCE_IRQHandler               ; CAN1 SCE                                               
                DCD     _EXTI9_5_IRQHandler                ; External Line[9:5]s                                    
                DCD     _TIM1_BRK_TIM9_IRQHandler          ; TIM1 Break and TIM9                   
                DCD     _TIM1_UP_TIM10_IRQHandler          ; TIM1 Update and TIM10                 
                DCD     _TIM1_TRG_COM_TIM11_IRQHandler     ; TIM1 Trigger and Commutation and TIM11
                DCD     _TIM1_CC_IRQHandler                ; TIM1 Capture Compare                                   
                DCD     _TIM2_IRQHandler                   ; TIM2                                            
                DCD     _TIM3_IRQHandler                   ; TIM3                                            
                DCD     _TIM4_IRQHandler                   ; TIM4                                            
                DCD     _I2C1_EV_IRQHandler                ; I2C1 Event                                             
                DCD     _I2C1_ER_IRQHandler                ; I2C1 Error                                             
                DCD     _I2C2_EV_IRQHandler                ; I2C2 Event                                             
                DCD     _I2C2_ER_IRQHandler                ; I2C2 Error                                               
                DCD     _SPI1_IRQHandler                   ; SPI1                                            
                DCD     _SPI2_IRQHandler                   ; SPI2                                            
                DCD     _USART1_IRQHandler                 ; USART1                                          
                DCD     _USART2_IRQHandler                 ; USART2                                          
                DCD     _USART3_IRQHandler                 ; USART3                                          
                DCD     _EXTI15_10_IRQHandler              ; External Line[15:10]s                                  
                DCD     _RTC_Alarm_IRQHandler              ; RTC Alarm (A and B) through EXTI Line                  
                DCD     _OTG_FS_WKUP_IRQHandler            ; USB OTG FS Wakeup through EXTI line                        
                DCD     _TIM8_BRK_TIM12_IRQHandler         ; TIM8 Break and TIM12                  
                DCD     _TIM8_UP_TIM13_IRQHandler          ; TIM8 Update and TIM13                 
                DCD     _TIM8_TRG_COM_TIM14_IRQHandler     ; TIM8 Trigger and Commutation and TIM14
                DCD     _TIM8_CC_IRQHandler                ; TIM8 Capture Compare                                   
                DCD     _DMA1_Stream7_IRQHandler           ; DMA1 Stream7                                           
                DCD     _FMC_IRQHandler                    ; FMC   
                DCD     _SDMMC1_IRQHandler                   ; SDIO                                            
                DCD     _TIM5_IRQHandler                   ; TIM5                                            
                DCD     _SPI3_IRQHandler                   ; SPI3                                            
                DCD     _UART4_IRQHandler                  ; UART4                                           
                DCD     _UART5_IRQHandler                  ; UART5                                           
                DCD     _TIM6_DAC_IRQHandler               ; TIM6 and DAC1&2 underrun errors                   
                DCD     _TIM7_IRQHandler                   ; TIM7                   
                DCD     _DMA2_Stream0_IRQHandler           ; DMA2 Stream 0                                   
                DCD     _DMA2_Stream1_IRQHandler           ; DMA2 Stream 1                                   
                DCD     _DMA2_Stream2_IRQHandler           ; DMA2 Stream 2                                   
                DCD     _DMA2_Stream3_IRQHandler           ; DMA2 Stream 3                                   
                DCD     _DMA2_Stream4_IRQHandler           ; DMA2 Stream 4                                   
                DCD     _ETH_IRQHandler                    ; Ethernet                                        
                DCD     _ETH_WKUP_IRQHandler               ; Ethernet Wakeup through EXTI line                      
                DCD     _CAN2_TX_IRQHandler                ; CAN2 TX                                                
                DCD     _CAN2_RX0_IRQHandler               ; CAN2 RX0                                               
                DCD     _CAN2_RX1_IRQHandler               ; CAN2 RX1                                               
                DCD     _CAN2_SCE_IRQHandler               ; CAN2 SCE                                               
                DCD     _OTG_FS_IRQHandler                 ; USB OTG FS                                      
                DCD     _DMA2_Stream5_IRQHandler           ; DMA2 Stream 5                                   
                DCD     _DMA2_Stream6_IRQHandler           ; DMA2 Stream 6                                   
                DCD     _DMA2_Stream7_IRQHandler           ; DMA2 Stream 7                                   
                DCD     _USART6_IRQHandler                 ; USART6                                           
                DCD     _I2C3_EV_IRQHandler                ; I2C3 event                                             
                DCD     _I2C3_ER_IRQHandler                ; I2C3 error                                             
                DCD     _OTG_HS_EP1_OUT_IRQHandler         ; USB OTG HS End Point 1 Out                      
                DCD     _OTG_HS_EP1_IN_IRQHandler          ; USB OTG HS End Point 1 In                       
                DCD     _OTG_HS_WKUP_IRQHandler            ; USB OTG HS Wakeup through EXTI                         
                DCD     _OTG_HS_IRQHandler                 ; USB OTG HS                                      
                DCD     _DCMI_IRQHandler                   ; DCMI  
                DCD     0                                 ; Reserved				                              
                DCD     _RNG_IRQHandler               ; Hash and Rng
                DCD     _FPU_IRQHandler                    ; FPU
                DCD     _UART7_IRQHandler                  ; UART7
                DCD     _UART8_IRQHandler                  ; UART8
                DCD     _SPI4_IRQHandler                   ; SPI4
                DCD     _SPI5_IRQHandler                   ; SPI5
                DCD     _SPI6_IRQHandler                   ; SPI6
                DCD     _SAI1_IRQHandler                   ; SAI1
                DCD     _LTDC_IRQHandler                   ; LTDC
                DCD     _LTDC_ER_IRQHandler                ; LTDC error
                DCD     _DMA2D_IRQHandler                  ; DMA2D
                DCD     _SAI2_IRQHandler                   ; SAI2
                DCD     _QUADSPI_IRQHandler                ; QUADSPI
                DCD     _LPTIM1_IRQHandler                 ; LPTIM1
                DCD     _CEC_IRQHandler                    ; HDMI_CEC
                DCD     _I2C4_EV_IRQHandler                ; I2C4 Event                                             
                DCD     _I2C4_ER_IRQHandler                ; I2C4 Error 
                DCD     _SPDIF_RX_IRQHandler               ; SPDIF_RX
                
                                         
__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler    PROC
                 EXPORT  Reset_Handler             [WEAK]
        IMPORT  SystemInit
        IMPORT  tk_main
                 LDR     R0, =SystemInit
                 BLX     R0
                 LDR     R0, =tk_main
                 BX      R0
                 ENDP
					 
BReset_Handler 	PROC
		EXPORT 	BReset_Handler					[WEAK]
        IMPORT  __main
                 LDR     R0, =__main
                 BX      R0
                 ENDP
	
; Dummy Exception Handlers (infinite loops which can be modified)
	 EXPORT	 _Reset_Handler
     EXPORT    _NMI_Handler
     EXPORT    _HardFault_Handler
     EXPORT    _MemManage_Handler
     EXPORT    _BusFault_Handler
     EXPORT    _UsageFault_Handler
     EXPORT    _SVC_Handler
     EXPORT    _DebugMon_Handler
     EXPORT    _PendSV_Handler
     EXPORT    _SysTick_Handler
     EXPORT    _WWDG_IRQHandler
     EXPORT    _PVD_IRQHandler				        
     EXPORT    _TAMP_STAMP_IRQHandler				   
     EXPORT    _RTC_WKUP_IRQHandler				
     EXPORT    _FLASH_IRQHandler				    
     EXPORT    _RCC_IRQHandler				
     EXPORT    _EXTI0_IRQHandler				      
     EXPORT    _EXTI1_IRQHandler				 
     EXPORT    _EXTI2_IRQHandler				
     EXPORT    _EXTI3_IRQHandler				     
     EXPORT    _EXTI4_IRQHandler				 
     EXPORT    _DMA1_Stream0_IRQHandler				
     EXPORT    _DMA1_Stream1_IRQHandler				
     EXPORT    _DMA1_Stream2_IRQHandler				
     EXPORT    _DMA1_Stream3_IRQHandler				   
     EXPORT    _DMA1_Stream4_IRQHandler				     
     EXPORT    _DMA1_Stream5_IRQHandler				 
     EXPORT    _DMA1_Stream6_IRQHandler				
     EXPORT    _ADC_IRQHandler				
     EXPORT    _CAN1_TX_IRQHandler				    
     EXPORT    _CAN1_RX0_IRQHandler				    
     EXPORT    _CAN1_RX1_IRQHandler				 
     EXPORT    _CAN1_SCE_IRQHandler				     
     EXPORT    _EXTI9_5_IRQHandler				
     EXPORT    _TIM1_BRK_TIM9_IRQHandler				 
     EXPORT    _TIM1_UP_TIM10_IRQHandler				
     EXPORT    _TIM1_TRG_COM_TIM11_IRQHandler				
     EXPORT    _TIM1_CC_IRQHandler
     EXPORT    _TIM2_IRQHandler				      
     EXPORT    _TIM3_IRQHandler				  
     EXPORT    _TIM4_IRQHandler				
     EXPORT    _I2C1_EV_IRQHandler				  
     EXPORT    _I2C1_ER_IRQHandler				  
     EXPORT    _I2C2_EV_IRQHandler				   
     EXPORT    _I2C2_ER_IRQHandler				
     EXPORT    _SPI1_IRQHandler				
     EXPORT    _SPI2_IRQHandler				
     EXPORT    _USART1_IRQHandler				
     EXPORT    _USART2_IRQHandler				  
     EXPORT    _USART3_IRQHandler				
     EXPORT    _EXTI15_10_IRQHandler				   
     EXPORT    _RTC_Alarm_IRQHandler				
     EXPORT    _OTG_FS_WKUP_IRQHandler				 
     EXPORT    _TIM8_BRK_TIM12_IRQHandler				 
     EXPORT    _TIM8_UP_TIM13_IRQHandler				 
     EXPORT    _TIM8_TRG_COM_TIM14_IRQHandler				 
     EXPORT    _TIM8_CC_IRQHandler
     EXPORT    _DMA1_Stream7_IRQHandler				
     EXPORT    _FMC_IRQHandler	                                            
     EXPORT    _SDMMC1_IRQHandler 				
     EXPORT    _TIM5_IRQHandler				
     EXPORT    _SPI3_IRQHandler				
     EXPORT    _UART4_IRQHandler				
     EXPORT    _UART5_IRQHandler				
     EXPORT    _TIM6_DAC_IRQHandler				     
     EXPORT    _TIM7_IRQHandler				
     EXPORT    _DMA2_Stream0_IRQHandler				
     EXPORT    _DMA2_Stream1_IRQHandler				     
     EXPORT    _DMA2_Stream2_IRQHandler				 
     EXPORT    _DMA2_Stream3_IRQHandler				    
     EXPORT    _DMA2_Stream4_IRQHandler				   
     EXPORT    _ETH_IRQHandler				
     EXPORT    _ETH_WKUP_IRQHandler				       
     EXPORT    _CAN2_TX_IRQHandler				  
     EXPORT    _CAN2_RX0_IRQHandler				  
     EXPORT    _CAN2_RX1_IRQHandler				    
     EXPORT    _CAN2_SCE_IRQHandler				 
     EXPORT    _OTG_FS_IRQHandler				
     EXPORT    _DMA2_Stream5_IRQHandler				 
     EXPORT    _DMA2_Stream6_IRQHandler				
     EXPORT    _DMA2_Stream7_IRQHandler				
     EXPORT    _USART6_IRQHandler				
     EXPORT    _I2C3_EV_IRQHandler				
     EXPORT    _I2C3_ER_IRQHandler				
     EXPORT    _OTG_HS_EP1_OUT_IRQHandler				
     EXPORT    _OTG_HS_EP1_IN_IRQHandler				 
     EXPORT    _OTG_HS_WKUP_IRQHandler				 
     EXPORT    _OTG_HS_IRQHandler				    
     EXPORT    _DCMI_IRQHandler	                                           
     EXPORT    _RNG_IRQHandler					
     EXPORT    _FPU_IRQHandler
     EXPORT		_UART7_IRQHandler                  ; UART7
     EXPORT 	_UART8_IRQHandler                  ; UART8
     EXPORT 	_SPI4_IRQHandler                   ; SPI4
     EXPORT 	_SPI5_IRQHandler                   ; SPI5
     EXPORT 	_SPI6_IRQHandler                   ; SPI6
     EXPORT 	_SAI1_IRQHandler                   ; SAI1
     EXPORT 	_LTDC_IRQHandler                   ; LTDC
     EXPORT 	_LTDC_ER_IRQHandler                ; LTDC error
     EXPORT 	_DMA2D_IRQHandler                  ; DMA2D
     EXPORT 	_SAI2_IRQHandler                   ; SAI2
     EXPORT 	_QUADSPI_IRQHandler                ; QUADSPI
     EXPORT 	_LPTIM1_IRQHandler                 ; LPTIM1
     EXPORT 	_CEC_IRQHandler                    ; HDMI_CEC
     EXPORT 	_I2C4_EV_IRQHandler                ; I2C4 Event                                             
     EXPORT 	_I2C4_ER_IRQHandler                ; I2C4 Error 
     EXPORT 	_SPDIF_RX_IRQHandler               ; SPDIF_RX

				
                PRESERVE8
                AREA    RESET2, CODE, READONLY
_Reset_Handler
				B.W 			Reset_Handler
_NMI_Handler
				B.W 			NMI_Handler
_HardFault_Handler
				B.W			HardFault_Handler
_MemManage_Handler
				B.W			MemManage_Handler
_BusFault_Handler
				B.W			BusFault_Handler
_UsageFault_Handler
				B.W 			UsageFault_Handler
_SVC_Handler
				B.W			SVC_Handler
_DebugMon_Handler
				B.W			DebugMon_Handler
_PendSV_Handler
				B.W			PendSV_Handler
_SysTick_Handler
				B.W			SysTick_Handler

_WWDG_IRQHandler
				B.W	WWDG_IRQHandler
_PVD_IRQHandler				
				B.W	PVD_IRQHandler          
_TAMP_STAMP_IRQHandler				
				B.W	TAMP_STAMP_IRQHandler   
_RTC_WKUP_IRQHandler				
				B.W	RTC_WKUP_IRQHandler 
_FLASH_IRQHandler				
				B.W	FLASH_IRQHandler     
_RCC_IRQHandler				
				B.W	RCC_IRQHandler 
_EXTI0_IRQHandler				
				B.W	EXTI0_IRQHandler       
_EXTI1_IRQHandler				
				B.W	EXTI1_IRQHandler  
_EXTI2_IRQHandler				
				B.W	EXTI2_IRQHandler 
_EXTI3_IRQHandler				
				B.W	EXTI3_IRQHandler     
_EXTI4_IRQHandler				
				B.W	EXTI4_IRQHandler    
_DMA1_Stream0_IRQHandler				
				B.W	DMA1_Stream0_IRQHandler 
_DMA1_Stream1_IRQHandler				
				B.W	DMA1_Stream1_IRQHandler 
_DMA1_Stream2_IRQHandler				
				B.W	DMA1_Stream2_IRQHandler
_DMA1_Stream3_IRQHandler				
				B.W	DMA1_Stream3_IRQHandler    
_DMA1_Stream4_IRQHandler				
				B.W	DMA1_Stream4_IRQHandler     
_DMA1_Stream5_IRQHandler				
				B.W	DMA1_Stream5_IRQHandler  
_DMA1_Stream6_IRQHandler				
				B.W	DMA1_Stream6_IRQHandler 
_ADC_IRQHandler				
				B.W	ADC_IRQHandler
_CAN1_TX_IRQHandler				
				B.W	CAN1_TX_IRQHandler       
_CAN1_RX0_IRQHandler				
				B.W	CAN1_RX0_IRQHandler     
_CAN1_RX1_IRQHandler				
				B.W	CAN1_RX1_IRQHandler 
_CAN1_SCE_IRQHandler				
				B.W	CAN1_SCE_IRQHandler       
_EXTI9_5_IRQHandler				
				B.W	EXTI9_5_IRQHandler 
_TIM1_BRK_TIM9_IRQHandler				
				B.W	TIM1_BRK_TIM9_IRQHandler   
_TIM1_UP_TIM10_IRQHandler				
				B.W	TIM1_UP_TIM10_IRQHandler
_TIM1_TRG_COM_TIM11_IRQHandler				
				B.W	TIM1_TRG_COM_TIM11_IRQHandler  
_TIM1_CC_IRQHandler
				B.W	TIM1_CC_IRQHandler    
_TIM2_IRQHandler				
				B.W	TIM2_IRQHandler      
_TIM3_IRQHandler				
				B.W	TIM3_IRQHandler   
_TIM4_IRQHandler				
				B.W	TIM4_IRQHandler 
_I2C1_EV_IRQHandler				
				B.W	I2C1_EV_IRQHandler    
_I2C1_ER_IRQHandler				
				B.W	I2C1_ER_IRQHandler    
_I2C2_EV_IRQHandler				
				B.W	I2C2_EV_IRQHandler    
_I2C2_ER_IRQHandler				
				B.W	I2C2_ER_IRQHandler 
_SPI1_IRQHandler				
				B.W	SPI1_IRQHandler  
_SPI2_IRQHandler				
				B.W	SPI2_IRQHandler
_USART1_IRQHandler				
				B.W	USART1_IRQHandler 
_USART2_IRQHandler				
				B.W	USART2_IRQHandler    
_USART3_IRQHandler				
				B.W	USART3_IRQHandler  
_EXTI15_10_IRQHandler				
				B.W	EXTI15_10_IRQHandler    
_RTC_Alarm_IRQHandler				
				B.W	RTC_Alarm_IRQHandler  
_OTG_FS_WKUP_IRQHandler				
				B.W	OTG_FS_WKUP_IRQHandler  
_TIM8_BRK_TIM12_IRQHandler				
				B.W	TIM8_BRK_TIM12_IRQHandler 
_TIM8_UP_TIM13_IRQHandler				
				B.W	TIM8_UP_TIM13_IRQHandler  
_TIM8_TRG_COM_TIM14_IRQHandler				
				B.W	TIM8_TRG_COM_TIM14_IRQHandler  
_TIM8_CC_IRQHandler
				B.W	TIM8_CC_IRQHandler 
_DMA1_Stream7_IRQHandler				
				B.W	DMA1_Stream7_IRQHandler
_FMC_IRQHandler				
				B.W	FMC_IRQHandler   
_SDMMC1_IRQHandler
				B.W SDMMC1_IRQHandler
_TIM5_IRQHandler				
				B.W	TIM5_IRQHandler    
_SPI3_IRQHandler				
				B.W	SPI3_IRQHandler 
_UART4_IRQHandler				
				B.W	UART4_IRQHandler  
_UART5_IRQHandler				
				B.W	UART5_IRQHandler    
_TIM6_DAC_IRQHandler				
				B.W	TIM6_DAC_IRQHandler       
_TIM7_IRQHandler				
				B.W	TIM7_IRQHandler   
_DMA2_Stream0_IRQHandler				
				B.W	DMA2_Stream0_IRQHandler
_DMA2_Stream1_IRQHandler				
				B.W	DMA2_Stream1_IRQHandler      
_DMA2_Stream2_IRQHandler				
				B.W	DMA2_Stream2_IRQHandler  
_DMA2_Stream3_IRQHandler				
				B.W	DMA2_Stream3_IRQHandler     
_DMA2_Stream4_IRQHandler				
				B.W	DMA2_Stream4_IRQHandler    
_ETH_IRQHandler				
				B.W	ETH_IRQHandler 
_ETH_WKUP_IRQHandler				
				B.W	ETH_WKUP_IRQHandler         
_CAN2_TX_IRQHandler				
				B.W	CAN2_TX_IRQHandler    
_CAN2_RX0_IRQHandler				
				B.W	CAN2_RX0_IRQHandler  
_CAN2_RX1_IRQHandler				
				B.W	CAN2_RX1_IRQHandler     
_CAN2_SCE_IRQHandler				
				B.W	CAN2_SCE_IRQHandler  
_OTG_FS_IRQHandler				
				B.W	OTG_FS_IRQHandler   
_DMA2_Stream5_IRQHandler				
				B.W	DMA2_Stream5_IRQHandler   
_DMA2_Stream6_IRQHandler				
				B.W	DMA2_Stream6_IRQHandler
_DMA2_Stream7_IRQHandler				
				B.W	DMA2_Stream7_IRQHandler  
_USART6_IRQHandler				
				B.W	USART6_IRQHandler        
_I2C3_EV_IRQHandler				
				B.W	I2C3_EV_IRQHandler 
_I2C3_ER_IRQHandler				
				B.W	I2C3_ER_IRQHandler  
_OTG_HS_EP1_OUT_IRQHandler				
				B.W	OTG_HS_EP1_OUT_IRQHandler     
_OTG_HS_EP1_IN_IRQHandler				
				B.W	OTG_HS_EP1_IN_IRQHandler       
_OTG_HS_WKUP_IRQHandler				
				B.W	OTG_HS_WKUP_IRQHandler  
_OTG_HS_IRQHandler				
				B.W	OTG_HS_IRQHandler     
_DCMI_IRQHandler				
				B.W	DCMI_IRQHandler 
_RNG_IRQHandler
				B.W	RNG_IRQHandler
_FPU_IRQHandler
				B.W	FPU_IRQHandler  
_UART7_IRQHandler                  ; UART7
				B.W	UART7_IRQHandler  
_UART8_IRQHandler                  ; UART8
				B.W	UART8_IRQHandler  
_SPI4_IRQHandler                   ; SPI4
				B.W	SPI4_IRQHandler  
_SPI5_IRQHandler                   ; SPI5
				B.W	SPI5_IRQHandler  
_SPI6_IRQHandler                   ; SPI6
				B.W	SPI6_IRQHandler  
_SAI1_IRQHandler                   ; SAI1
				B.W	SAI1_IRQHandler  
_LTDC_IRQHandler                   ; LTDC
				B.W	LTDC_IRQHandler  
_LTDC_ER_IRQHandler                ; LTDC error
				B.W	LTDC_ER_IRQHandler  
_DMA2D_IRQHandler                  ; DMA2D
				B.W	DMA2D_IRQHandler  
_SAI2_IRQHandler                   ; SAI2
				B.W	SAI2_IRQHandler  
_QUADSPI_IRQHandler                ; QUADSPI
				B.W	QUADSPI_IRQHandler  
_LPTIM1_IRQHandler                 ; LPTIM1
				B.W	LPTIM1_IRQHandler  
_CEC_IRQHandler                    ; HDMI_CEC
				B.W	CEC_IRQHandler  
_I2C4_EV_IRQHandler                ; I2C4 Event    
				B.W	I2C4_EV_IRQHandler                                           
_I2C4_ER_IRQHandler                ; I2C4 Error 
				B.W	I2C4_ER_IRQHandler  
_SPDIF_RX_IRQHandler               ; SPDIF_RX
				B.W	SPDIF_RX_IRQHandler  
				
NMI_Handler     PROC
                EXPORT  NMI_Handler                [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler          [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler           [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler           [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler             [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler            [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WWDG_IRQHandler                   [WEAK]                                        
                EXPORT  PVD_IRQHandler                    [WEAK]                      
                EXPORT  TAMP_STAMP_IRQHandler             [WEAK]         
                EXPORT  RTC_WKUP_IRQHandler               [WEAK]                     
                EXPORT  FLASH_IRQHandler                  [WEAK]                                         
                EXPORT  RCC_IRQHandler                    [WEAK]                                            
                EXPORT  EXTI0_IRQHandler                  [WEAK]                                            
                EXPORT  EXTI1_IRQHandler                  [WEAK]                                             
                EXPORT  EXTI2_IRQHandler                  [WEAK]                                            
                EXPORT  EXTI3_IRQHandler                  [WEAK]                                           
                EXPORT  EXTI4_IRQHandler                  [WEAK]                                            
                EXPORT  DMA1_Stream0_IRQHandler           [WEAK]                                
                EXPORT  DMA1_Stream1_IRQHandler           [WEAK]                                   
                EXPORT  DMA1_Stream2_IRQHandler           [WEAK]                                   
                EXPORT  DMA1_Stream3_IRQHandler           [WEAK]                                   
                EXPORT  DMA1_Stream4_IRQHandler           [WEAK]                                   
                EXPORT  DMA1_Stream5_IRQHandler           [WEAK]                                   
                EXPORT  DMA1_Stream6_IRQHandler           [WEAK]                                   
                EXPORT  ADC_IRQHandler                    [WEAK]                         
                EXPORT  CAN1_TX_IRQHandler                [WEAK]                                                
                EXPORT  CAN1_RX0_IRQHandler               [WEAK]                                               
                EXPORT  CAN1_RX1_IRQHandler               [WEAK]                                                
                EXPORT  CAN1_SCE_IRQHandler               [WEAK]                                                
                EXPORT  EXTI9_5_IRQHandler                [WEAK]                                    
                EXPORT  TIM1_BRK_TIM9_IRQHandler          [WEAK]                  
                EXPORT  TIM1_UP_TIM10_IRQHandler          [WEAK]                
                EXPORT  TIM1_TRG_COM_TIM11_IRQHandler     [WEAK] 
                EXPORT  TIM1_CC_IRQHandler                [WEAK]                                   
                EXPORT  TIM2_IRQHandler                   [WEAK]                                            
                EXPORT  TIM3_IRQHandler                   [WEAK]                                            
                EXPORT  TIM4_IRQHandler                   [WEAK]                                            
                EXPORT  I2C1_EV_IRQHandler                [WEAK]                                             
                EXPORT  I2C1_ER_IRQHandler                [WEAK]                                             
                EXPORT  I2C2_EV_IRQHandler                [WEAK]                                            
                EXPORT  I2C2_ER_IRQHandler                [WEAK]                                               
                EXPORT  SPI1_IRQHandler                   [WEAK]                                           
                EXPORT  SPI2_IRQHandler                   [WEAK]                                            
                EXPORT  USART1_IRQHandler                 [WEAK]                                          
                EXPORT  USART2_IRQHandler                 [WEAK]                                          
                EXPORT  USART3_IRQHandler                 [WEAK]                                         
                EXPORT  EXTI15_10_IRQHandler              [WEAK]                                  
                EXPORT  RTC_Alarm_IRQHandler              [WEAK]                  
                EXPORT  OTG_FS_WKUP_IRQHandler            [WEAK]                        
                EXPORT  TIM8_BRK_TIM12_IRQHandler         [WEAK]                 
                EXPORT  TIM8_UP_TIM13_IRQHandler          [WEAK]                 
                EXPORT  TIM8_TRG_COM_TIM14_IRQHandler     [WEAK] 
                EXPORT  TIM8_CC_IRQHandler                [WEAK]                                   
                EXPORT  DMA1_Stream7_IRQHandler           [WEAK]                                          
                EXPORT  FMC_IRQHandler                    [WEAK]
				EXPORT  SDMMC1_IRQHandler				  [WEAK]                                           
                EXPORT  TIM5_IRQHandler                   [WEAK]                                             
                EXPORT  SPI3_IRQHandler                   [WEAK]                                             
                EXPORT  UART4_IRQHandler                  [WEAK]                                            
                EXPORT  UART5_IRQHandler                  [WEAK]                                            
                EXPORT  TIM6_DAC_IRQHandler               [WEAK]                   
                EXPORT  TIM7_IRQHandler                   [WEAK]                    
                EXPORT  DMA2_Stream0_IRQHandler           [WEAK]                                  
                EXPORT  DMA2_Stream1_IRQHandler           [WEAK]                                   
                EXPORT  DMA2_Stream2_IRQHandler           [WEAK]                                    
                EXPORT  DMA2_Stream3_IRQHandler           [WEAK]                                    
                EXPORT  DMA2_Stream4_IRQHandler           [WEAK]                                 
                EXPORT  ETH_IRQHandler                    [WEAK]                                         
                EXPORT  ETH_WKUP_IRQHandler               [WEAK]                     
                EXPORT  CAN2_TX_IRQHandler                [WEAK]                                               
                EXPORT  CAN2_RX0_IRQHandler               [WEAK]                                               
                EXPORT  CAN2_RX1_IRQHandler               [WEAK]                                               
                EXPORT  CAN2_SCE_IRQHandler               [WEAK]                                               
                EXPORT  OTG_FS_IRQHandler                 [WEAK]                                       
                EXPORT  DMA2_Stream5_IRQHandler           [WEAK]                                   
                EXPORT  DMA2_Stream6_IRQHandler           [WEAK]                                   
                EXPORT  DMA2_Stream7_IRQHandler           [WEAK]                                   
                EXPORT  USART6_IRQHandler                 [WEAK]                                           
                EXPORT  I2C3_EV_IRQHandler                [WEAK]                                              
                EXPORT  I2C3_ER_IRQHandler                [WEAK]                                              
                EXPORT  OTG_HS_EP1_OUT_IRQHandler         [WEAK]                      
                EXPORT  OTG_HS_EP1_IN_IRQHandler          [WEAK]                      
                EXPORT  OTG_HS_WKUP_IRQHandler            [WEAK]                        
                EXPORT  OTG_HS_IRQHandler                 [WEAK]                                      
                EXPORT  DCMI_IRQHandler                   [WEAK]                                            
                EXPORT  RNG_IRQHandler                    [WEAK] 
                EXPORT  FPU_IRQHandler                    [WEAK]
                EXPORT  UART7_IRQHandler                  [WEAK]
                EXPORT  UART8_IRQHandler                  [WEAK]
                EXPORT  SPI4_IRQHandler                   [WEAK]
                EXPORT  SPI5_IRQHandler                   [WEAK]
                EXPORT  SPI6_IRQHandler                   [WEAK]
                EXPORT  SAI1_IRQHandler                   [WEAK]
                EXPORT  LTDC_IRQHandler                   [WEAK]
                EXPORT  LTDC_ER_IRQHandler                [WEAK]
                EXPORT  DMA2D_IRQHandler                  [WEAK]
                EXPORT  SAI2_IRQHandler                   [WEAK]   
                EXPORT  QUADSPI_IRQHandler                [WEAK]
                EXPORT  LPTIM1_IRQHandler                 [WEAK]
                EXPORT  CEC_IRQHandler                    [WEAK]   
                EXPORT  I2C4_EV_IRQHandler                [WEAK]
                EXPORT  I2C4_ER_IRQHandler                [WEAK] 
                EXPORT  SPDIF_RX_IRQHandler               [WEAK]
                
WWDG_IRQHandler                                                       
PVD_IRQHandler                                      
TAMP_STAMP_IRQHandler                  
RTC_WKUP_IRQHandler                                
FLASH_IRQHandler                                                       
RCC_IRQHandler     
				B				.                                                       
EXTI0_IRQHandler   
				B				.                                                       
EXTI1_IRQHandler   
				B				.                                                        
EXTI2_IRQHandler   
				B				.                                                       
EXTI3_IRQHandler    
				B				.                                                     
EXTI4_IRQHandler     
				B				.                                                     
DMA1_Stream0_IRQHandler  
				B				.                                     
DMA1_Stream1_IRQHandler   
				B				.                                       
DMA1_Stream2_IRQHandler    
				B				.                                      
DMA1_Stream3_IRQHandler   
				B				.                                      
DMA1_Stream4_IRQHandler    
				B				.                                      
DMA1_Stream5_IRQHandler    
				B				.                                      
DMA1_Stream6_IRQHandler  
				B				.                                        
ADC_IRQHandler    
				B				.                                     
CAN1_TX_IRQHandler  
				B				.                                                          
CAN1_RX0_IRQHandler  
				B				.                                                        
CAN1_RX1_IRQHandler    
				B				.                                                       
CAN1_SCE_IRQHandler     
				B				.                                                      
EXTI9_5_IRQHandler     
				B				.                                           
TIM1_BRK_TIM9_IRQHandler   
				B				.                     
TIM1_UP_TIM10_IRQHandler    
				B				.                  
TIM1_TRG_COM_TIM11_IRQHandler  
				B				.
TIM1_CC_IRQHandler    
				B				.                                           
TIM2_IRQHandler     
				B				.                                                      
TIM3_IRQHandler     
				B				.                                                      
TIM4_IRQHandler   
				B				.                                                        
I2C1_EV_IRQHandler  
				B				.                                                       
I2C1_ER_IRQHandler  
				B				.                                                       
I2C2_EV_IRQHandler   
				B				.                                                     
I2C2_ER_IRQHandler    
				B				.                                                       
SPI1_IRQHandler      
				B				.                                                    
SPI2_IRQHandler    
				B				.                                                       
USART1_IRQHandler     
				B				.                                                  
USART2_IRQHandler      
				B				.                                                 
USART3_IRQHandler       
				B				.                                               
EXTI15_10_IRQHandler   
				B				.                                         
RTC_Alarm_IRQHandler      
				B				.                      
OTG_FS_WKUP_IRQHandler      
				B		.
TIM8_BRK_TIM12_IRQHandler   
				B		.
TIM8_UP_TIM13_IRQHandler 
				B		.
TIM8_TRG_COM_TIM14_IRQHandler  
				B				.
TIM8_CC_IRQHandler                
				B				.                               
DMA1_Stream7_IRQHandler        
				B				.                                         
FMC_IRQHandler    
				B				.
SDMMC1_IRQHandler   
				B				.
TIM5_IRQHandler       
				B				.                                                     
SPI3_IRQHandler       
				B				.                                                     
UART4_IRQHandler     
				B				.                                                     
UART5_IRQHandler         
				B				.                                                 
TIM6_DAC_IRQHandler       
				B				.                     
TIM7_IRQHandler           
				B				.                   
DMA2_Stream0_IRQHandler          
                B             .                   
DMA2_Stream1_IRQHandler         
                B               .                                  
DMA2_Stream2_IRQHandler         
                B               .                                   
DMA2_Stream3_IRQHandler         
                B              .                                    
DMA2_Stream4_IRQHandler        
                B               .                                 
ETH_IRQHandler             
                B              .                                              
ETH_WKUP_IRQHandler      
                B              .                            
CAN2_TX_IRQHandler      
                B              .                                                       
CAN2_RX0_IRQHandler      
                B              .                                                      
CAN2_RX1_IRQHandler      
                B              .                                                      
CAN2_SCE_IRQHandler      
                B              .                                                      
OTG_FS_IRQHandler     
                B             .                                                  
DMA2_Stream5_IRQHandler    
                B              .                                        
DMA2_Stream6_IRQHandler    
                B              .                                        
DMA2_Stream7_IRQHandler    
                B              .                                        
USART6_IRQHandler         
                B             .                                                  
I2C3_EV_IRQHandler       
                B             .                                                      
I2C3_ER_IRQHandler      
                B             .                                                       
OTG_HS_EP1_OUT_IRQHandler   
                B              .                          
OTG_HS_EP1_IN_IRQHandler  
                B              .                            
OTG_HS_WKUP_IRQHandler   
                B             .                                
OTG_HS_IRQHandler      
                B             .                                                
DCMI_IRQHandler      
                B             .                                                         
RNG_IRQHandler     
                B             .    
FPU_IRQHandler   
                B              .  
UART7_IRQHandler    
                B             .                 
UART8_IRQHandler     
                B              .               
SPI4_IRQHandler     
                B             .                 
SPI5_IRQHandler     
                B             .                 
SPI6_IRQHandler     
                B             .                 
SAI1_IRQHandler     
                B             .                 
LTDC_IRQHandler     
                B              .                
LTDC_ER_IRQHandler    
                B              .               
DMA2D_IRQHandler   
                B              .  
SAI2_IRQHandler  
                B             .         
QUADSPI_IRQHandler
                B             .   
LPTIM1_IRQHandler
                B             .   
CEC_IRQHandler
                B             .   
I2C4_EV_IRQHandler
                B              .  
I2C4_ER_IRQHandler
                B              .  
SPDIF_RX_IRQHandler
                B              .

                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB
                
                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit
                
                 ELSE
                
                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap
				
                 
__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF
					 
			     EXPORT  __user_heap_size

__user_heap_size
                LDR     R0, =  Heap_Size
				BX	LR

                 END

;************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE*****
