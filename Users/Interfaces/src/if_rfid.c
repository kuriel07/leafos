#include "..\..\defs.h"
//#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include <string.h>
#include "..\inc\if_rfid.h"

const uint8 MFRC522_firmware_referenceV0_0[] = {
	0x00, 0x87, 0x98, 0x0f, 0x49, 0xFF, 0x07, 0x19,
	0xBF, 0x22, 0x30, 0x49, 0x59, 0x63, 0xAD, 0xCA,
	0x7F, 0xE3, 0x4E, 0x03, 0x5C, 0x4E, 0x49, 0x50,
	0x47, 0x9A, 0x37, 0x61, 0xE7, 0xE2, 0xC6, 0x2E,
	0x75, 0x5A, 0xED, 0x04, 0x3D, 0x02, 0x4B, 0x78,
	0x32, 0xFF, 0x58, 0x3B, 0x7C, 0xE9, 0x00, 0x94,
	0xB4, 0x4A, 0x59, 0x5B, 0xFD, 0xC9, 0x29, 0xDF,
	0x35, 0x96, 0x98, 0x9E, 0x4F, 0x30, 0x32, 0x8D
};
// Version 1.0 (0x91)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 self-test
const uint8 MFRC522_firmware_referenceV1_0[] = {
	0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C,
	0xC2, 0xD8, 0x7C, 0x4D, 0xD9, 0x70, 0xC7, 0x73,
	0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1, 0x3E, 0x5A,
	0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E,
	0x64, 0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC,
	0x22, 0xBC, 0xD3, 0x72, 0x35, 0xCD, 0xAA, 0x41,
	0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E, 0x02,
	0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79
};
// Version 2.0 (0x92)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 self-test
const uint8 MFRC522_firmware_referenceV2_0[] = {
	0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
	0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
	0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
	0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
	0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
	0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
	0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
	0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
};
// Clone
// Fudan Semiconductor FM17522 (0x88)
const uint8 FM17522_firmware_reference[]  = {
	0x00, 0xD6, 0x78, 0x8C, 0xE2, 0xAA, 0x0C, 0x18,
	0x2A, 0xB8, 0x7A, 0x7F, 0xD3, 0x6A, 0xCF, 0x0B,
	0xB1, 0x37, 0x63, 0x4B, 0x69, 0xAE, 0x91, 0xC7,
	0xC3, 0x97, 0xAE, 0x77, 0xF4, 0x37, 0xD7, 0x9B,
	0x7C, 0xF5, 0x3C, 0x11, 0x8F, 0x15, 0xC3, 0xD7,
	0xC1, 0x5B, 0x00, 0x2A, 0xD0, 0x75, 0xDE, 0x9E,
	0x51, 0x64, 0xAB, 0x3E, 0xE9, 0x15, 0xB5, 0xAB,
	0x56, 0x9A, 0x98, 0x82, 0x26, 0xEA, 0x2A, 0x62
};
//SPI写数据
//蟻E843写葋Ebyte数据   
void if_spi_write(spi_context_p ctx, uint8_t num)    
{  
	uint8_t count=0;   
	uint16 i;
	//TCLK=0;//上升沿有效	
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	if(((spi_context *)ctx)->t_state & SPI_STATE_LSB_FIRST) {
		for(count=0;count<8;count++)  
		{ 	      	 
			if(num&0x01) GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);		//TDIN=1;  
			else GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);				//TDIN=0;   
			num>>=1; 
			//TCLK=0;//上升沿有效
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			//TCLK=1;      
			GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			__nop();
			//TCLK=0;//上升沿有效
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
		} 	
		
	} else {
		for(count=0;count<8;count++)  
		{ 	      	 
			if(num&0x80) GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);		//TDIN=1;  
			else GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);				//TDIN=0;   
			num<<=1; 
			//TCLK=0;
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			//TCLK=1;      
			GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			__nop();
			//TCLK=0;
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
		} 		
	}	
	//TCLK=0;//下降沿有效
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
} 	

void if_rfid_write(rf_context * ctx, uint8 address, uint8 data) {
	
	//TCLK=0;//先拉低时钟 	 
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	//RCS=0; 				//rfid select
	//GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->select(ctx);
	if_spi_write((spi_context *)ctx, (address << 1) & 0x7F);
	if_spi_write((spi_context *)ctx, data);
	//RCS=1;				//rfid deselect
	//GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->deselect(ctx);
}

void if_rfid_write_bytes(rf_context * ctx, uint8 address, uint8 len, uint8 * data) {
	uint8 i;
	//TCLK=0;//先拉低时钟 	 
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	//RCS=0; 				//rfid select
	//GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->select(ctx);
	if_spi_write((spi_context *)ctx, (address << 1) & 0x7F);
	for(i=0;i<len;i++) if_spi_write((spi_context *)ctx, data[i]);
	//RCS=1;				//rfid deselect
	//GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->deselect(ctx);
	
}

uint8 if_spi_read(spi_context_p ctx, uint8 add) {  
	uint16 i;
	uint8 count=0; 	
	uint8 Num=0x01; 
	uint8 b = 0;
	//TCLK=0;//下降沿有效
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	if(((spi_context *)ctx)->t_state & SPI_STATE_LSB_FIRST) {
		for(count=0;count<8;count++)  
		{ 			 
			if(add&0x01)GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);		//TDIN=1;  
			else GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);				//TDIN=0;  
			add>>=1;  	 	
			//TCLK=0;//下降沿有效  
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			//TCLK=1;  
			GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			if(( ((GPIO_TypeDef *)(((spi_context *)ctx)->handle))->IDR &  ((spi_context *)ctx)->miso) != 0) b |= Num; 		
			Num<<=1; 		 
		}  
	} else {
		Num = 0;
		for(count=0;count<8;count++)  
		{ 			 
			if(add&0x80)GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);		//TDIN=1;  
			else GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->mosi);				//TDIN=0;  
			add<<=1;  	 	
			//TCLK=0;//下降沿有效  
			GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			//TCLK=1;  
			GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
			Num<<=1; 	 
			if(( ((GPIO_TypeDef *)(((spi_context *)ctx)->handle))->IDR &  ((spi_context *)ctx)->miso) != 0) Num++; 		 
		} 
		b = Num;
	}
	//TCLK=0;//下降沿有效 
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	return b;
}

uint8 if_rfid_read(rf_context * ctx, uint8 address, uint8 len, uint8 * buffer) {
	//return if_spi_read(address | 0x80);
	uint8 i;
	uint8 temp[67];
	uint8 addr = ((address << 1) | 0x80) ;
	//TCLK=0;//先拉低时钟 	 
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->sck);
	//RCS=0; 				//rfid select
	//GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->select(ctx);
	if_spi_write((spi_context_p)ctx, addr);		//MSB high for read operation   
	for(i=0;i<(len -1);i++) buffer[i] = if_spi_read((spi_context_p)ctx, addr);
	buffer[i] =  if_spi_read((spi_context_p)ctx, 0);
	//RCS=1;				//rfid deselect
	//GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->nss);
	((spi_context *)ctx)->deselect(ctx);
}

uint8 if_rfid_read_byte(rf_context * ctx, uint8 address) {
	uint8 b;
	if_rfid_read(ctx, address, 1, &b);
	return b;
}

static void if_rc522_reset(void * ctx) {
#if 0
	GPIO_ResetBits(GPIOE, GPIO_PIN_15);
	//if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_REST);
	if_delay(20);
	GPIO_SetBits(GPIOE, GPIO_PIN_15);
	if_delay(100);
#endif
}

GPIO_TypeDef g_portConfig;
static void if_rc522_select(void * ctx) {
	//save GPIOA context
	GPIO_InitTypeDef GPIO_InitStructure;
	memcpy(&g_portConfig, ((spi_context *)ctx)->handle, sizeof(GPIO_TypeDef));
	GPIO_InitStructure.Pin = ((spi_context *)ctx)->cs;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(((spi_context *)ctx)->handle, &GPIO_InitStructure);
	
	GPIO_ResetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->cs);
}

static void if_rc522_deselect(void * ctx) {
	//PBout(0) = 1;
	GPIO_SetBits(((spi_context *)ctx)->handle, ((spi_context *)ctx)->cs);
	memcpy(((spi_context *)ctx)->handle, &g_portConfig, sizeof(GPIO_TypeDef));
}

void if_rfid_reset(rf_context * ctx) {
	((spi_context *)ctx)->reset(ctx);
}

void if_spi_init(rf_context_p ctx, uint8 state) {
	GPIO_InitTypeDef GPIO_InitStructure;
#if 0
	//PE11 = CS
	//PE12 = CLK
	//PE13 = MOSI
	//PE14 = MISO
	//PE15 = RST
	__HAL_RCC_GPIOE_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;	//CK
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = GPIO_PIN_14;	//CK
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);	
	((spi_context *)ctx)->handle = GPIOE;
	((spi_context *)ctx)->mosi = GPIO_PIN_13;
	((spi_context *)ctx)->miso = GPIO_PIN_14;
	((spi_context *)ctx)->sck = GPIO_PIN_12;	
	((spi_context *)ctx)->cs = GPIO_PIN_11;	
#else
	//PA4 = CS
	//PA5 = CLK
	//PA7 = MOSI
	//PA6 = MISO
	//NONE = RST
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;	//CS, CLK, MOSI
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = GPIO_PIN_6;	//MISO
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	((spi_context *)ctx)->handle = GPIOA;
	((spi_context *)ctx)->mosi = GPIO_PIN_7;
	((spi_context *)ctx)->miso = GPIO_PIN_6;
	((spi_context *)ctx)->sck = GPIO_PIN_5;	
	((spi_context *)ctx)->cs = GPIO_PIN_4;	
#endif				
	((spi_context *)ctx)->select = if_rc522_select;		
	((spi_context *)ctx)->deselect = if_rc522_deselect;
	((spi_context *)ctx)->reset = if_rc522_reset;
	((spi_context *)ctx)->t_state = state;
}

uint8 if_rfid_init(rf_context * ctx) {
	uint8 ver;
	uint8 ret;
	if_spi_init(ctx, SPI_STATE_MSB_FIRST);
	ctx->state = 0;
	ret = if_rfid_selftest(ctx);
	if(ret != 0) return ret;
	
	if_rfid_reset(ctx);
	// Reset baud rates
	if_rfid_write(ctx, 0x12, 0x00);
	if_rfid_write(ctx, 0x13, 0x00);
	// Reset ModWidthReg
	if_rfid_write(ctx, 0x24, 0x26);

	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	if_rfid_write(ctx, 0x2A, 0x80);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	if_rfid_write(ctx, 0x2B, 0xA9);		// TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25兪s.
	if_rfid_write(ctx, 0x2C, 0x1F);	//0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	if_rfid_write(ctx, 0x2D, 0x40);	//0xE8);
		
	if_rfid_write(ctx, 0x15, 0x40);		// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	if_rfid_write(ctx, 0x11, 0x3D);		// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	//PCD_AntennaOn();						// Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
	ret = if_rfid_read_byte(ctx, 0x14);
	if ((ret & 0x03) != 0x03) {
		if_rfid_write(ctx, 0x14, ret | 0x03);
	}
	ret = if_rfid_read_byte(ctx, 0x14);
	if_rfid_read(ctx, 0x37, 1, (uint8 *)&ver);
	//if_rfid_write(ctx, 0x16, 0x14);		//RxSelReg  (X | guard time)
	if_rfid_write(ctx, 0x17, 0x80);		//RxSelReg  (X | guard time)
	if_rfid_write(ctx, 0x18, 0x22);		//RxThresholdReg (threshold | X)
	if_rfid_write(ctx, 0x26, 0x48);		//RxThresholdReg (threshold | X)
	ctx->state = RF_STATE_INITIALIZED;
	return 0;
}

uint8 if_picc_connect(rf_context * ctx, uint8 * atr) {
	OS_DEBUG_ENTRY(if_picc_connect);
	uint8 len;
	uint8 value;
	uint8 ret = -1;
	memset(&ctx->uid, 0 ,sizeof(picc_uid));
	ctx->state &= ~RF_STATE_CONNECTED;							//clear connected state
	if((ctx->state & RF_STATE_INITIALIZED )== 0) goto exit_picc_connect;		//context not initialized
	//while(!(value = if_rfid_present(ctx)));
	if((value = if_rfid_present(ctx)) != 0) goto exit_picc_connect;
	//if_rfid_wakeup();
	value = if_picc_select(ctx, &ctx->uid, 0);
	if(value != 0) goto exit_picc_connect;
	atr[0] = ctx->uid.size;
	memcpy(atr + 1, ctx->uid.uidByte, ctx->uid.size);
	atr[ctx->uid.size + 1] = ctx->uid.sak;
	ctx->state |= RF_STATE_CONNECTED;
	ret = ctx->uid.size + 2;		//return atr len
	exit_picc_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_rfid_comm_picc(	rf_context * ctx, uint8 command,		///< The command to execute. One of the PCD_Command enums.
														uint8 waitIRq,		///< The bits in the ComIrqReg register that signals successful completion of the command.
														uint8 *sendData,		///< Pointer to the data to transfer to the FIFO.
														uint8 sendLen,		///< Number of bytes to transfer to the FIFO.
														uint8 *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
														uint8 *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
														uint8 *validBits,			///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits.
														uint8 rxAlign
)
{
	// Prepare values for BitFramingReg
	uint8 txLastBits = validBits ? *validBits : 0;
	uint8 bitFraming = (rxAlign << 4) + txLastBits;		// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
	uint8 errorRegValue , n;
	uint32 i;
	uint8 value, value2;
	uint8 _validBits = 0;
	uint8 status2, status1,status3;
	//if_rfid_write(ctx, RFID_REG_CIEN, 0x7F);    // interrupt request	
	if_rfid_write(ctx, RFID_REG_CIRQ, 0x7F);					// Clear all seven interrupt request bits		
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_IDLE);			// Stop any active command.	
	if_rfid_write(ctx, RFID_REG_DLEN, 0x80);				// FlushBuffer = 1, FIFO initialization 
	if_rfid_write_bytes(ctx, RFID_REG_DATA, sendLen, sendData);	// Write sendData to the FIFO
	if_rfid_write(ctx, RFID_REG_BFRM, bitFraming);		// Bit adjustments
	if_rfid_write(ctx, RFID_REG_CMD, command);				// Execute the command				 
	if (command == RFID_CMD_XRCV) {
		if_rfid_write(ctx, RFID_REG_BFRM, 0x80 |bitFraming);	// StartSend=1, transmission of data starts
	}
	status1 = if_rfid_read_byte(ctx, RFID_REG_STS2);
	// Wait for the command to complete.
	// In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
	// Each iteration of the do-while-loop takes 17.86  s.
	// TODO check/modify for other architectures than Arduino Uno 16bit
	if_delay(20);
	for (i = 200000; i > 0; i--) {
		status2 = if_rfid_read_byte(ctx, RFID_REG_STS2);
		n = if_rfid_read_byte(ctx, RFID_REG_CIRQ);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
		if (n & waitIRq) {					// One of the interrupts that signal success has been set.
			status3 = n;
			break;
		}
		if (n & 0x01) {						// Timer interrupt - nothing received in 25ms
			return -5;			//timeout
		}
	}
	if_rfid_write(ctx, RFID_REG_BFRM, if_rfid_read_byte(ctx, RFID_REG_BFRM) & 0x7F);	// StartSend=1, transmission of data starts
	// 35.7ms and nothing happend. Communication with the MFRC522 might be down.
	if (i == 0) {
		return -5;				//timeout
	}
	// Stop now if any errors except collisions were detected.
	errorRegValue = if_rfid_read_byte(ctx, RFID_REG_ERR); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
	if (errorRegValue & 0x13) {	 // BufferOvfl ParityErr ProtocolErr
		return -1;				//error
	}
  
	
	
	// If the caller wants data back, get it from the MFRC522.
	if (backData && backLen) {
		n = if_rfid_read_byte(ctx, RFID_REG_DLEN);	// Number of bytes in the FIFO
		if (n > *backLen) {
			return -1;			//out of capacity
		}
		*backLen = n;											// Number of bytes returned
		if_rfid_read(ctx, RFID_REG_DATA, n, backData);	// Get received data from FIFO
		_validBits = if_rfid_read_byte(ctx, RFID_REG_CTRL) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
		if (validBits) {
			*validBits = _validBits;
		}
	}
	
	// Tell about collisions
	if (errorRegValue & 0x08) {		// CollErr
		return -2;			//collision
	}
	
	// Perform CRC_A validation if requested.
#if 0
	if (backData && backLen && checkCRC) {
		// In this case a MIFARE Classic NAK is not OK.
		if (*backLen == 1 && _validBits == 4) {
			return STATUS_MIFARE_NACK;
		}
		// We need at least the CRC_A value and all 8 bits of the last byte must be received.
		if (*backLen < 2 || _validBits != 0) {
			return STATUS_CRC_WRONG;
		}
		// Verify CRC_A - do our own calculation and store the control in controlBuffer.
		byte controlBuffer[2];
		MFRC522::StatusCode status = PCD_CalculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
		if (status != STATUS_OK) {
			return status;
		}
		if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
			return STATUS_CRC_WRONG;
		}
	}
#endif
	
	return 0;
} // End PCD_CommunicateWithPICC()

uint8 if_rfid_transceive(rf_context * ctx, uint8 *sendData,		///< Pointer to the data to transfer to the FIFO.
													uint8 sendLen,		///< Number of bytes to transfer to the FIFO.
													uint8 *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
													uint8 *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
													uint8 *validBits 
) 
{
	uint8 waitIRq = 0x30;		// RxIRq and IdleIRq
	return if_rfid_comm_picc(ctx, 0x0C, waitIRq, sendData, sendLen, backData, backLen, validBits, 0);
} // End PCD_TransceiveData()

uint8 if_rfid_transceive_ext(rf_context * ctx, uint8 *sendData,		///< Pointer to the data to transfer to the FIFO.
													uint8 sendLen,		///< Number of bytes to transfer to the FIFO.
													uint8 *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
													uint8 *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
													uint8 *validBits,
													uint8 rxAlign
) 
{
	uint8 waitIRq = 0x30;		// RxIRq and IdleIRq
	return if_rfid_comm_picc(ctx, 0x0C, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign);
} // End PCD_TransceiveData()


uint8 if_rfid_wupa_reqa(rf_context * ctx, uint8 command, 		///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
												uint8 *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
												uint8 *bufferSize	///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
											) {
	uint8 validBits;
	uint8 status;
	uint8 val;
	if (bufferATQA == NULL || *bufferSize < 2) {	// The ATQA response is 2 bytes long.
		return -1;
	}
	//PCD_ClearRegisterBitMask(CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
	val = if_rfid_read_byte(ctx, RFID_REG_COLS);
	if_rfid_write(ctx, RFID_REG_COLS, val & 0x7F);
	validBits = 7;									// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
	status = if_rfid_transceive(ctx, &command, 1, bufferATQA, bufferSize, &validBits);
	if (status != 0) {
		return status;
	}
	if (*bufferSize != 2 || validBits != 0) {		// ATQA must be exactly 16 bits.
		return -1;
	}
	return 0;
} // End PICC_REQA_or_WUPA()
											
uint8 if_rfid_selftest(rf_context * ctx) {
	uint8 ZEROES[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8 n;
	uint8 i;
	uint8 result[64];
	uint8 ver;
	uint8 *ref;
	// This follows directly the steps outlined in 16.1.1
	// 1. Perform a soft reset.

	if_rfid_reset(ctx);
	
	// 2. Clear the internal buffer by writing 25 bytes of 00h
	if_rfid_write(ctx, RFID_REG_DLEN, 0x80);		// flush the FIFO buffer
	if_rfid_write_bytes(ctx, RFID_REG_DATA, 25, ZEROES);	// write 25 bytes of 00h to FIFO
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_MEM);		// transfer to internal buffer
	
	// 3. Enable self-test
	if_rfid_write(ctx, 0x36, 0x09);
	
	// 4. Write 00h to FIFO buffer
	if_rfid_write(ctx, RFID_REG_DATA, 0x00);
	
	// 5. Start self-test by issuing the CalcCRC command
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_CCRC);
	//if_rfid_write(ctx, RFID_REG_DLEN, 0x80);		// flush the FIFO buffer
	
	// 6. Wait for self-test to complete
	//n = if_rfid_read_byte(ctx, RFID_REG_DLEN);
	//if_rfid_read(RFID_REG_DATA, n, result);
	for (i = 0; i < 0xFF; i++) {
		// The datasheet does not specify exact completion condition except
		// that FIFO buffer should contain 64 bytes.
		// While selftest is initiated by CalcCRC command
		// it behaves differently from normal CRC computation,
		// so one can't reliably use DivIrqReg to check for completion.
		// It is reported that some devices does not trigger CRCIRq flag
		// during selftest.
		n = if_rfid_read_byte(ctx, RFID_REG_DLEN);
		if (n >= 64) {
			break;
		}
	}
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_IDLE);		// Stop calculating CRC for new content in the FIFO.
	
	// 7. Read out resulting 64 bytes from the FIFO buffer.
	if_rfid_read(ctx, RFID_REG_DATA, 64, result);
	
	// Auto self-test done
	// Reset AutoTestReg register to be 0 again. Required for normal operation.
	if_rfid_write(ctx, 0x36, 0x00);
	
	// Determine firmware version (see section 9.3.4.8 in spec)
	ver = if_rfid_read_byte(ctx, 0x37);
	
	// Pick the appropriate reference values
	//const byte *reference;
	switch (ver) {
		case 0x88:	// Fudan Semiconductor FM17522 clone
			ref = (uint8 *)FM17522_firmware_reference;
			break;
		case 0x90:	// Version 0.0
			ref = (uint8 *)MFRC522_firmware_referenceV0_0;
			break;
		case 0x91:	// Version 1.0
			ref = (uint8 *)MFRC522_firmware_referenceV1_0;
			break;
		case 0x92:	// Version 2.0
			ref = (uint8 *)MFRC522_firmware_referenceV2_0;
			break;
		default:	// Unknown version
			return -1; // abort test
	}
	
	// Verify that the results match up to our expectations
	for (i = 0; i < 64; i++) {
		if (result[i] != ref[i]) {
			return -1;
		}
	}
	// Test passed; all is good.
	return 0;
} // End PCD_PerformSelfTest()
											
uint8 if_rfid_present(rf_context_p ctx) {
	uint8 bufferATQA[2];
	uint8 bufferSize = sizeof(bufferATQA);
	uint8 status;
	// Reset baud rates
	if_rfid_write(ctx, 0x12, 0x00);
	if_rfid_write(ctx, 0x13, 0x00);
	// Reset ModWidthReg
	if_rfid_write(ctx, 0x24, 0x26);

	//MFRC522::StatusCode result = PICC_RequestA(bufferATQA, &bufferSize);
	status = if_rfid_wupa_reqa(ctx, PICC_CMD_REQA, bufferATQA, &bufferSize);
	//return (result == STATUS_OK || result == STATUS_COLLISION);
	return status;
} // End PICC_IsNewCardPresent()
											
uint8 if_rfid_wakeup(rf_context * ctx) {
	uint8 bufferATQA[2];
	uint8 bufferSize = sizeof(bufferATQA);
	uint8 status;
	// Reset baud rates
	if_rfid_write(ctx, 0x12, 0x00);
	if_rfid_write(ctx, 0x13, 0x00);
	// Reset ModWidthReg
	if_rfid_write(ctx, 0x24, 0x26);

	//MFRC522::StatusCode result = PICC_RequestA(bufferATQA, &bufferSize);
	status = if_rfid_wupa_reqa(ctx, PICC_CMD_WUPA, bufferATQA, &bufferSize);
	//return (result == STATUS_OK || result == STATUS_COLLISION);
	return status == 0;
} // End PICC_IsNewCardPresent()


uint8 if_rfid_calc_crc(rf_context *ctx,	uint8 *data,	 uint8 length, uint8 *result) 
{
	uint16 i;
	uint8 n;
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_IDLE);		// Stop any active command.
	if_rfid_write(ctx, RFID_REG_DIRQ, 0x04);				// Clear the CRCIRq interrupt request bit
	if_rfid_write(ctx, RFID_REG_DLEN, 0x80);			// FlushBuffer = 1, FIFO initialization
	if_rfid_write_bytes(ctx, RFID_REG_DATA, length, data);	// Write data to the FIFO
	if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_CCRC);		// Start the calculation
	
	// Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73  s.
	// TODO check/modify for other architectures than Arduino Uno 16bit

	// Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73us.
	for (i = 5000; i > 0; i--) {
		// DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
		//n = PCD_ReadRegister(DivIrqReg);
		n = if_rfid_read_byte(ctx, RFID_REG_DIRQ);
		if (n & 0x04) {									// CRCIRq bit set - calculation done
			//PCD_WriteRegister(CommandReg, PCD_Idle);	// Stop calculating CRC for new content in the FIFO.
			if_rfid_write(ctx, RFID_REG_CMD, RFID_CMD_IDLE);
			// Transfer the result from the registers to the result buffer
			result[0] = if_rfid_read_byte(ctx, RFID_REG_CRCL);
			result[1] = if_rfid_read_byte(ctx, RFID_REG_CRCH);
			return 0;
		}
	}
	// 89ms passed and nothing happend. Communication with the MFRC522 might be down.
	return -4;///STATUS_TIMEOUT;
} // End PCD_CalculateCRC()

uint8 if_picc_select(rf_context * ctx,	picc_uid *uid,			///< Pointer to Uid struct. Normally output, but can also be used to supply a known UID.
											uint8 validBits		///< The number of known UID bits supplied in *uid. Normally 0. If set you must also supply uid->size.
) 
{
	uint8 uidComplete;
	uint8 selectDone;
	uint8 useCascadeTag;
	uint8 cascadeLevel = 1;
	uint8 result;
	uint8 count;
	uint8 index;
	uint8 uidIndex;					// The first index in uid->uidByte[] that is used in the current Cascade Level.
	int8_t currentLevelKnownBits;		// The number of known UID bits in the current Cascade Level.
	uint8 buffer[9];					// The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A
	uint8 bufferUsed;				// The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
	uint8 rxAlign;					// Used in BitFramingReg. Defines the bit position for the first bit received.
	uint8 txLastBits;				// Used in BitFramingReg. The number of valid bits in the last transmitted byte. 
	uint8 *responseBuffer;
	uint8 responseLength;
	uint8 bytesToCopy, maxBytes;
	uint8 valueOfCollReg;
	uint8 collisionPos;
	
	// Description of buffer structure:
	//		Byte 0: SEL 				Indicates the Cascade Level: PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2 or PICC_CMD_SEL_CL3
	//		Byte 1: NVB					Number of Valid Bits (in complete command, not just the UID): High nibble: complete bytes, Low nibble: Extra bits. 
	//		Byte 2: UID-data or CT		See explanation below. CT means Cascade Tag.
	//		Byte 3: UID-data
	//		Byte 4: UID-data
	//		Byte 5: UID-data
	//		Byte 6: BCC					Block Check Character - XOR of bytes 2-5
	//		Byte 7: CRC_A
	//		Byte 8: CRC_A
	// The BCC and CRC_A are only transmitted if we know all the UID bits of the current Cascade Level.
	//
	// Description of bytes 2-5: (Section 6.5.4 of the ISO/IEC 14443-3 draft: UID contents and cascade levels)
	//		UID size	Cascade level	Byte2	Byte3	Byte4	Byte5
	//		========	=============	=====	=====	=====	=====
	//		 4 bytes		1			uid0	uid1	uid2	uid3
	//		 7 bytes		1			CT		uid0	uid1	uid2
	//						2			uid3	uid4	uid5	uid6
	//		10 bytes		1			CT		uid0	uid1	uid2
	//						2			CT		uid3	uid4	uid5
	//						3			uid6	uid7	uid8	uid9
	
	// Sanity checks
	if (validBits > 80) {
		return -1;
	}
	
	// Prepare MFRC522
	//PCD_ClearRegisterBitMask(CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
	if_rfid_write(ctx, RFID_REG_COLS, if_rfid_read_byte(ctx, RFID_REG_COLS) & 0x7F); 
	
	// Repeat Cascade Level loop until we have a complete UID.
	uidComplete = 0;
	while (!uidComplete) {
		// Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
		switch (cascadeLevel) {
			case 1:
				buffer[0] = PICC_CMD_SEL_CL1;
				uidIndex = 0;
				useCascadeTag = validBits && uid->size > 4;	// When we know that the UID has more than 4 bytes
				break;
			
			case 2:
				buffer[0] = PICC_CMD_SEL_CL2;
				uidIndex = 3;
				useCascadeTag = validBits && uid->size > 7;	// When we know that the UID has more than 7 bytes
				break;
			
			case 3:
				buffer[0] = PICC_CMD_SEL_CL3;
				uidIndex = 6;
				useCascadeTag = 0;						// Never used in CL3.
				break;
			
			default:
				return -1;
				break;
		}
		
		// How many UID bits are known in this Cascade Level?
		currentLevelKnownBits = validBits - (8 * uidIndex);
		if (currentLevelKnownBits < 0) {
			currentLevelKnownBits = 0;
		}
		// Copy the known bits from uid->uidByte[] to buffer[]
		index = 2; // destination index in buffer[]
		if (useCascadeTag) {
			buffer[index++] = PICC_CMD_CT;
		}
		bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
		if (bytesToCopy) {
			maxBytes = useCascadeTag ? 3 : 4; // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
			if (bytesToCopy > maxBytes) {
				bytesToCopy = maxBytes;
			}
			for (count = 0; count < bytesToCopy; count++) {
				buffer[index++] = uid->uidByte[uidIndex + count];
			}
		}
		// Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
		if (useCascadeTag) {
			currentLevelKnownBits += 8;
		}
		
		// Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
		selectDone = 0;
		while (!selectDone) {
			// Find out how many bits and bytes to send and receive.
			if (currentLevelKnownBits >= 32) { // All UID bits in this Cascade Level are known. This is a SELECT.
				//Serial.print(F("SELECT: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes
				// Calculate BCC - Block Check Character
				buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
				// Calculate CRC_A
				result = if_rfid_calc_crc(ctx, buffer, 7, &buffer[7]);
				if (result != 0) {
					return result;
				}
				txLastBits		= 0; // 0 => All 8 bits are valid.
				bufferUsed		= 9;
				// Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
				responseBuffer	= &buffer[6];
				responseLength	= 3;
			}
			else { // This is an ANTICOLLISION.
				//Serial.print(F("ANTICOLLISION: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				txLastBits		= currentLevelKnownBits % 8;
				count			= currentLevelKnownBits / 8;	// Number of whole bytes in the UID part.
				index			= 2 + count;					// Number of whole bytes: SEL + NVB + UIDs
				buffer[1]		= (index << 4) + txLastBits;	// NVB - Number of Valid Bits
				bufferUsed		= index + (txLastBits ? 1 : 0);
				// Store response in the unused part of buffer
				responseBuffer	= &buffer[index];
				responseLength	= sizeof(buffer) - index;
			}
			
			// Set bit adjustments
			rxAlign = txLastBits;											// Having a separate variable is overkill. But it makes the next line easier to read.
			//PCD_WriteRegister(BitFramingReg, (rxAlign << 4) + txLastBits);	// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
			if_rfid_write(ctx, RFID_REG_BFRM, (rxAlign << 4) + txLastBits);	
			
			// Transmit the buffer and receive the response.
			//result = PCD_TransceiveData(buffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign);
			result = if_rfid_transceive_ext(ctx, buffer, bufferUsed, responseBuffer, & responseLength, &txLastBits, rxAlign);
			if (result == (uint8)-2) { // More than one PICC in the field => collision.
				//valueOfCollReg = PCD_ReadRegister(CollReg); // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
				valueOfCollReg = if_rfid_read_byte(ctx, RFID_REG_COLS);
				if (valueOfCollReg & 0x20) { // CollPosNotValid
					return -2; // Without a valid collision position we cannot continue
				}
				collisionPos = valueOfCollReg & 0x1F; // Values 0-31, 0 means bit 32.
				if (collisionPos == 0) {
					collisionPos = 32;
				}
				if (collisionPos <= currentLevelKnownBits) { // No progress - should not happen 
					return -1;
				}
				// Choose the PICC with the bit set.
				currentLevelKnownBits = collisionPos;
				count			= (currentLevelKnownBits - 1) % 8; // The bit to modify
				index			= 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
				buffer[index]	|= (1 << count);
			}
			else if (result != 0) {
				return result;
			}
			else { // STATUS_OK
				if (currentLevelKnownBits >= 32) { // This was a SELECT.
					selectDone = 1; // No more anticollision 
					// We continue below outside the while.
				}
				else { // This was an ANTICOLLISION.
					// We now have all 32 bits of the UID in this Cascade Level
					currentLevelKnownBits = 32;
					// Run loop again to do the SELECT.
				}
			}
		} // End of while (!selectDone)
		
		// We do not check the CBB - it was constructed by us above.
		
		// Copy the found UID bytes from buffer[] to uid->uidByte[]
		index			= (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
		bytesToCopy		= (buffer[2] == PICC_CMD_CT) ? 3 : 4;
		for (count = 0; count < bytesToCopy; count++) {
			uid->uidByte[uidIndex + count] = buffer[index++];
		}
		
		// Check response SAK (Select Acknowledge)
		if (responseLength != 3 || txLastBits != 0) { // SAK must be exactly 24 bits (1 byte + CRC_A).
			return -1;
		}
		// Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
		result = if_rfid_calc_crc(ctx, responseBuffer, 1, &buffer[2]);
		if (result != 0) {
			return result;
		}
		if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2])) {
			return -3;		//invalid crc
		}
		if (responseBuffer[0] & 0x04) { // Cascade bit set - UID not complete yes
			cascadeLevel++;
		}
		else {
			uidComplete = 1;
			uid->sak = responseBuffer[0];
		}
	} // End of while (!uidComplete)
	// Set correct uid->size
	uid->size = 3 * cascadeLevel + 1;
	return 0;
} // End PICC_Select()


uint8 if_picc_get_ats(rf_context * ctx, uint8 * ats) {
	
}

/**
 * Wrapper for MIFARE protocol communication.
 * Adds CRC_A, executes the Transceive command and checks that the response is MF_ACK or a timeout.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
uint8 if_picc_transceive(rf_context * ctx, uint8 *sendData,		///< Pointer to the data to transfer to the FIFO. Do NOT include the CRC_A.
													uint8 sendLen,		///< Number of bytes in sendData.
													uint8 acceptTimeout	///< True => A timeout is also success
												) 
{
	uint8 result;
	uint8 cmdBuffer[18]; // We need room for 16 bytes data and 2 bytes CRC_A.
	uint8 waitIRq = 0x30;		// RxIRq and IdleIRq
	uint8 cmdBufferSize = sizeof(cmdBuffer);
	uint8 validBits = 0;
	
	// Sanity check
	if (sendData == NULL || sendLen > 16) {
		return -1;
	}
	
	// Copy sendData[] to cmdBuffer[] and add CRC_A
	memcpy(cmdBuffer, sendData, sendLen);
	result = if_rfid_calc_crc(ctx, cmdBuffer, sendLen, &cmdBuffer[sendLen]);
	if (result != 0) { 
		return result;
	}
	sendLen += 2;
	
	// Transceive the data, store the reply in cmdBuffer[]
	result = if_rfid_comm_picc(ctx, 0x0C, waitIRq, cmdBuffer, sendLen, cmdBuffer, &cmdBufferSize, &validBits, 0);
	if (acceptTimeout && result == (uint8)-5) {
		return 0;
	}
	if (result != 0) {
		return result;
	}
	// The PICC must reply with a 4 bit ACK
	if (cmdBufferSize != 1 || validBits != 4) {
		return -1;
	}
	if (cmdBuffer[0] != 0xA) {			//mifare ACK
		return -1;
	}
	return 0;
} // End PCD_MIFARE_Transceive()

/**
 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
 * The MF0ICU1 returns a NAK for higher addresses.
 * The MF0ICU1 responds to the READ command by sending 16 bytes starting from the page address defined by the command argument.
 * For example; if blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned.
 * A roll-back is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh, 00h and 01h are returned.
 * 
 * The buffer must be at least 18 bytes because a CRC_A is also returned.
 * Checks the CRC_A before returning STATUS_OK.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
uint8 if_picc_read(rf_context * ctx, uint8 blockAddr, 	///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The first page to return data from.
											uint8 *buffer,		///< The buffer to store the data in
											uint8 *bufferSize	///< Buffer size, at least 18 bytes. Also number of bytes returned if STATUS_OK.
										) 
{
	uint8 result;
	
	// Sanity check
	if (buffer == NULL || *bufferSize < 18) {
		return -1;
	}
	
	// Build command buffer
	buffer[0] = PICC_CMD_MF_READ;
	buffer[1] = blockAddr;
	// Calculate CRC_A
	result = if_rfid_calc_crc(ctx, buffer, 2, &buffer[2]);
	if (result != 0) {
		return result;
	}
	
	// Transmit the buffer and receive the response, validate CRC_A.
	return if_rfid_transceive(ctx, buffer, 4, buffer, bufferSize, NULL);
} // End MIFARE_Read()

/**
 * Writes 16 bytes to the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight the operation is called "COMPATIBILITY WRITE".
 * Even though 16 bytes are transferred to the Ultralight PICC, only the least significant 4 bytes (bytes 0 to 3)
 * are written to the specified address. It is recommended to set the remaining bytes 04h to 0Fh to all logic 0.
 * * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
uint8 if_picc_write(rf_context * ctx,	uint8 blockAddr, ///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The page (2-15) to write to.
											uint8 *buffer,	///< The 16 bytes to write to the PICC
											uint8 bufferSize	///< Buffer size, must be at least 16 bytes. Exactly 16 bytes are written.
										)
{
	uint8 result;
	uint8 cmdBuffer[2];
	
	// Sanity check
	if (buffer == NULL || bufferSize < 16) {
		return -1;
	}
	
	// Mifare Classic protocol requires two communications to perform a write.
	// Step 1: Tell the PICC we want to write to block blockAddr.
	cmdBuffer[0] = PICC_CMD_MF_WRITE;
	cmdBuffer[1] = blockAddr;
	result = if_picc_transceive(ctx, cmdBuffer, 2, 0); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != 0) {
		return result;
	}
	
	// Step 2: Transfer the data
	result = if_picc_transceive(ctx, buffer, bufferSize, 0); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != 0) {
		return result;
	}
	
	return 0;
} // End MIFARE_Write()

uint8 if_picc_authenticate(rf_context * ctx, uint8 command,		///< PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
											uint8 blockAddr, 	///< The block number. See numbering in the comments in the .h file.
											uint8 *key	///< Pointer to the Crypteo1 key to use (6 bytes)
	) 
{
	uint8 waitIRq = 0x10;		// IdleIRq
	uint8 i;
	uint8 txBits =0;
	// Build command buffer
	uint8 sendData[12];
	sendData[0] = command;
	sendData[1] = blockAddr;
	for (i = 0; i < 6; i++) {	// 6 key bytes
		sendData[2+i] = key[i];
	}
	// Use the last uid bytes as specified in http://cache.nxp.com/documents/application_note/AN10927.pdf
	// section 3.2.5 "MIFARE Classic Authentication".
	// The only missed case is the MF1Sxxxx shortcut activation,
	// but it requires cascade tag (CT) byte, that is not part of uid.
	if(ctx->uid.size < 4) return -1;			//invalid uid
	for (i = 0; i < 4; i++) {				// The last 4 bytes of the UID
		sendData[8+i] = ctx->uid.uidByte[i+ctx->uid.size-4];
	}
	// Start the authentication.
	//return PCD_CommunicateWithPICC(PCD_MFAuthent, waitIRq, &sendData[0], sizeof(sendData));
	return if_rfid_comm_picc(ctx, 0x0E, waitIRq, &sendData[0], sizeof(sendData), NULL, 0, &txBits, 0 );
} // End PCD_Authenticate()