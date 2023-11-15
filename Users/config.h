/*!
 * \file 			config.h
 * \brief     	configuration for kron
 * \details   	useful when changing to different system
 * \author    	AGP
 * \version   	1.0
 * \date      	2017.03.22
 * \verbatim
 * start development STM32F1, ISO7816 driver, ESP8266 driver, LCD UI (february 2016)
 * ported to STM32F4 (july 2016)
 * added wolfSSL library (july 2016)
 * added libpng library (november 2016)
 * ported orb-weaver virtual machine+APIs (start december 2016), partially tested (march 2017)
 * added multitasking capability (january 2017)
 * switch to etheron 2.0 board (february 2017)
 * added support for MOD_RTL8710 and MOD_ESP8266
 * support for STM32F7 (2018.07.19)
 * added support for MOD_SIM868 (2018.08.05)
 */

#ifndef KRON_CONFIG_H__
#define KRON_CONFIG_H__

#include "defs.h"
#define STM32F765_SERIES		0x451
#define STM32F407_SERIES		0x413
#define STM32F746_SERIES		0x449
#define STM32F413_SERIES		0x463

#ifdef STM32F746xx
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_conf.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_i2c.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_wwdg.h"

#include "stm32f7xx_hal_rtc.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_adc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_rng.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_flash_ex.h"
#include "stm32f7xx_hal_usart.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_dma2d.h"

#define HAL_USART_READ(x)			((x).Instance->RDR)
#define HAL_USART_WRITE(x, dat)		((x).Instance->TDR = dat)
#define HARDWARE_ID		"S765g"
#define STM32F7xx		HARDWARE_ID
#ifndef STM32F7
#define STM32F7
#endif
#endif

#ifdef STM32F407xx
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_wwdg.h"

#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_usart.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_dma2d.h"
#include "stm32f4xx_hal_spi.h"

#define HAL_USART_READ(x)			(x).Instance->DR
#define HAL_USART_WRITE(x, dat)		(x).Instance->DR
#define HARDWARE_ID		"S407b"
#define STM32F4xx		HARDWARE_ID
#ifndef STM32F4
#define STM32F4
#endif
#endif

#ifdef STM32F427xx
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_wwdg.h"

#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_usart.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_dma2d.h"
#include "stm32f4xx_hal_spi.h"

#define HAL_USART_READ(x)			(x).Instance->DR
#define HAL_USART_WRITE(x, dat)		(x).Instance->DR
#define HARDWARE_ID					"S413a"
#define STM32F4xx					HARDWARE_ID
#ifndef STM32F4
#define STM32F4
#endif
#endif

#define MOD_NONE			0x0000
#define MOD_ESP8266			0x8266
#define MOD_ESP32			0x8232
#define MOD_RTL8710			0x8710
#define MOD_HM11			0x2511
#define MOD_SIM868			0x7868		//SIMCOM868
#define MAJOR_VERSION 		1
#define MINOR_VERSION 		0
#define MICRO_VERSION 		2
#define LCD_FMC_ENABLED		1		//direct LCD access through FSMC
#define LCD_LTDC_ENABLED	1
#define HWCL_ENABLED		1		//hardware accelerator enabled

////LibQRGen Configuration
#define HAVE_STRDUP 		0
#define VERSION 			""
////End of LibQRGen Configuration


#define ORB_CLA										0xA8
#define SHARD_RTOS_ENABLED				0x01
#define SHARD_ENABLE_PRERENDERED	0				//use pre-render image for rendering (need SDRAM), fast rendering
#define SHARD_OWL_SUPPORTED				0x13		//also act as terminal version
#define SHARD_TERMINAL_AUTH				0x01
#define SHARD_SECURE_SESSION			0x00
#define SHARD_SECURE_TERMINAL			0x00
#define SHARD_EXTENDED_APDU				0x00
#define SHARD_SSL_SUPPORT					0
#define SHARD_WIFI_MODULE					MOD_SIM868
#define SHARD_BLE_MODULE					MOD_NONE
#define SHARD_AUTO_POWER_DOWN			0
#define SHARD_MAX_RESOURCES				6
#define SHARD_USE_FRAMEBUFFER			1


#define SHARD_TERMINAL_CONFIG		SHARD_TERMINAL_AUTH | (SHARD_SECURE_SESSION << 1) | (SHARD_SECURE_TERMINAL << 2) | (SHARD_EXTENDED_APDU << 7)
#define SHARD_TERMINAL_INPUT		0x00
#define SHARD_TERMINAL_OUTPUT		0x00

#define SHARD_AUTOREC_START		0x400		//start of autorecord offset
#define SHARD_AUTOREC_END			0x800		//end of autorecord offset
#define SHARD_SSIDREC_START			0x800		//start of SSID record offset
#define SHARD_SSIDREC_END				0xC00		//end of SSID record offset
#define SHARD_BTREC_START			0xC00		//start of bluetooth record offset
#define SHARD_BTREC_END				0xD00		//end of bluetooth record offset
#define SHARD_HISTORY_START			0x1000		//start of download history record offset
#define SHARD_HISTORY_END			0x3000		//end of download history record offset
#define SHARD_ESID_SIZE					0x10 			//esid in binary format
#define SHARD_UID_SIZE					0x20 + 1		//uid in hexstring format
#define SHARD_TOKEN_SIZE				0x20 + 1		//token in hexstring format
#define SHARD_UNAME_SIZE				0x40 + 1		//uname in string format
#define SHARD_PASS_SIZE					0x40 + 1		//pass in string format
#define SHARD_HASH_SIZE					32
#define SHARD_MAX_DESC					80
#define SHARD_MAX_NODE_NAME				16
//usb configuration
#define SHARD_DRIVER_OKEY				1
#define SHARD_DRIVER_KRON				2
#define SHARD_USB_DRIVER				SHARD_DRIVER_KRON

//kernel app Orb-Weaver configuration
#define STACK_VAR_APIS					1
#define STACK_FILE_APIS					0
#define STACK_CONVERTER_APIS		1
#define STACK_BIT_APIS					1
#define STACK_IO_APIS						1
#define STACK_GUI_APIS					1
#define STACK_CRYPTO_APIS				1
#define STACK_NETWORK_APIS			1
#define STACK_PICC_APIS					1
#define STACK_BLE_APIS					1

#define SHARD_LCD_TYPE					0x9341			//0x9325
#define DISPLAY_WIDTH					320
#define DISPLAY_HEIGHT					240
#define DISPLAY_SIZE						(DISPLAY_WIDTH * DISPLAY_HEIGHT)

//#define SHARD_NFC_TYPE					0x522			//mifare RC522
#define SHARD_NFC_TYPE					0x532			//nxp PN532
//#define SHARD_TS_TYPE					0x6236			//use 0x0000 to switch to resistive ADC based
#define SHARD_TS_TYPE					0x5336			//use 0x0000 to switch to resistive ADC based

/**** wolfSSL for KEIL-RL Configuration ****/

////WolfSSL Configuration
#define __CORTEX_M3__
#define WOLFSSL_MDK_ARM
#define WOLFSSL_MDK5
#define WOLFSSL_USER_IO

//#define WOLFSSL_STM32F2
//#define STM32F2_CRYPTO
#define SMT32F2_RNG
#define NO_SESSION_CACHE

#define NO_WRITEV
#define NO_WOLFSSL_DIR
#define NO_MAIN_DRIVER
#define NO_FILESYSTEM

#define SINGLE_THREADED

//#define WOLFSSL_DER_LOAD
#define WOLFSSL_SMALL_STACK
#define HAVE_NULL_CIPHER
#define WOLFSSL_USER_TIME
#define NO_TIME_H
#define USER_TICKS

#define NO_ECHOSERVER
#define NO_ECHOCLIENT
#define NO_SIMPLE_SERVER
#define NO_SIMPLE_CLIENT

//wolf ssl optimization
#define NO_PSK
//#define NO_WOLFSSL_CLIENT
//#define NO_WOLFSSL_SERVER
//#define NO_DSA
//#define NO_HMAC
#define NO_MD4
//#define NO_MD5
//#define NO_SHA256
#define NO_PWDBASED
#define NO_SESSION_CACHE
#define NO_RABBIT
#define NO_HC128
#define NO_DES3
//#define NO_ERROR_STRINGS
//#define MAX_DIGEST_SIZE 	32
//#define NO_TLS
//#define NO_DH
//#define NO_RSA
//#define NO_SHA
#define WOLFSSL_KEY_GEN
#define HAVE_ECC
//#define HAVE_TRUNCATED_HMAC
//#define WOLFSSL_SHA384
//#define WOLFSSL_SHA512
//#define HAVE_AESCCM
//#define HAVE_CAMELLIA
//#define HAVE_TLS_EXTENSIONS
//#define OPENSSL_EXTRA
//#define WOLFSSL_DER_LOAD


#define TFM_TIMING_RESISTANT
#define USE_FAST_MATH

#define PNG_ABORT()

static  int ValidateDate(const unsigned char* date, unsigned char format, int dateType){ return 1; }

#if    defined(MDK_CONF_RTX_TCP_FS)
#include "WolfSSL/inc/config-RTX-TCP-FS.h"
#elif  defined(MDK_CONF_TCP_FS)
#include "WolfSSL/inc/config-TCP-FS.h"
#elif  defined(MDK_CONF_FS)
#include "WolfSSL/inc/config-FS.h"
#elif  defined(MDK_CONF_BARE_METAL)
#include "WolfSSL/inc/config-BARE-METAL.h"
#elif  defined(MDK_WOLFLIB)
#include "WolfSSL/inc/config-WOLFLIB.h"
#endif
////End of WolfSSL Configuration

#define GPIO_SetBits(p, x) 		HAL_GPIO_WritePin(p, x, GPIO_PIN_SET)
#define GPIO_ResetBits(p, x) 	HAL_GPIO_WritePin(p, x, GPIO_PIN_RESET)

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40010C0C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40011E08 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //Êä³E
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //ÊäÈE

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //Êä³E
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //ÊäÈE

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //Êä³E
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //ÊäÈE

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //Êä³E
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //ÊäÈE

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //Êä³E
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //ÊäÈE

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //Êä³E
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //ÊäÈE

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //Êä³E
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //ÊäÈE

/* LCD ¿ØÖÆÒý½ÅÅäÖÃ*/
#define LCD_CS  PCout(8)
#define LCD_RS	PCout(9)
#define LCD_WR  PCout(10)
#define LCD_RD	PCout(11)

//¶¨ÒåLCDµÄ³ß´E
#define LCD_W 320
#define LCD_H 240

/***********************************************************************************
ÉèÖÃÑÕÉ«ºE¨ÒE
***********************************************************************************/
#define   BLACK        0x0000                    /* ºÚÉ«£º 0, 0, 0               */
#define   NAVY         0x000F                    /* ÉûÜ¶É«£º 0, 0, 128           */
#define   DGREEN       0x03E0                    /* ÉûÞÌÉ«£º 0, 128, 0           */
#define   DCYAN        0x03EF                    /* ÉûãàÉ«£º 0, 128, 128         */
#define   MAROON       0x7800                    /* ÉûÖE«£º128, 0, 0            */
#define   PURPLE       0x780F                    /* ×ÏÉ«£º 128, 0, 128           */
#define   OLIVE        0x7BE0                    /* éÏé­ÂÌ£º128, 128, 0          */
#define   LGRAY        0xC618                    /* »Ò°×É«£º192, 192, 192        */
#define   DGRAY        0x7BEF                    /* Éû×ÒÉ«£º128, 128, 128        */
#define   BLUE         0x001F                    /* À¶É«£º 0, 0, 255             */
#define   GREEN        0x07E0                 	 /* ÂÌÉ«£º 0, 255, 0             */
#define   CYAN         0x07FF                    /* ÇàÉ«£º 0, 255, 255           */
#define   RED          0xF800                    /* ºE«£º 255, 0, 0             */
#define   MAGENTA      0xF81F                    /* Æ·ºEº 255, 0, 255           */
#define   YELLOW       0xFFE0                    /* »ÆÉ«£º 255, 255, 0           */
#define   WHITE        0xFFFF                    /* °×É«£º 255, 255, 255         */
#define   IDMCOLOR(color) (((color & 0x001F) << 11) | ((color & 0xF800) >> 11) | (color & 0x07E0))
#define   RGBCOLOR(red, green, blue) (((red >> 3) << 11) |  ((green >> 2) << 5) | (blue & 0x1F))
#endif
