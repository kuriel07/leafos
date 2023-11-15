#include "defs.h"
#include "config.h"
#include "if_apis.h"
#include <string.h>
#include "if_nfc.h"
#include "if_rfid.h"
#include "os.h"
#include "os_core.h"
#include "if_i2c.h"

uint8_t pn532_packetbuffer[64];
#define HAL(func)   (ctx->func)

#define STATUS_READ     	2
#define DATA_WRITE      	1
#define DATA_READ       	3	

#define DMSG(x)
#define DMSG_HEX(x)

#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
void if_nfc_i2c_begin(nfc_context_p ctx);
uint8 if_nfc_i2c_is_ready(nfc_context_p ctx);
void if_nfc_i2c_write_frame(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen);
int8_t if_nfc_i2c_read_ack_frame(nfc_context_p ctx);
int16_t if_nfc_i2c_read_response(nfc_context_p ctx, uint8_t buf[], uint16_t len, uint16_t timeout);
void if_nfc_spi_begin(nfc_context_p ctx);
uint8 if_nfc_spi_is_ready(nfc_context_p ctx);
void if_nfc_spi_write_frame(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen);
int8_t if_nfc_spi_read_ack_frame(nfc_context_p ctx);
int16_t if_nfc_spi_read_response(nfc_context_p ctx, uint8_t buf[], uint16_t len, uint16_t timeout);
#endif

void if_nfc_begin(nfc_context_p ctx)
{
	uint8 i;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return;		//NFC not initialized
#if PN532_COMM_INTERFACE == PN532_COMM_I2C
	//use I2C address select
	if(HAL_I2C_IsDeviceReady((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, 10, 2000) == HAL_OK) {
		i = 0;
	}
	//enable RF field
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_SPI
	//use CS pin to wakeup device
	((spi_context *)ctx)->select(ctx);
    if_delay(2);
	((spi_context *)ctx)->deselect(ctx);
	//enable RF field
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
	switch(ctx->comm_interface) {
		case PN532_COMM_I2C:
			if_nfc_i2c_begin(ctx);
			break;
		case PN532_COMM_SPI:
			if_nfc_spi_begin(ctx);
			break;
	}
#endif
}


uint8 if_nfc_is_ready(nfc_context_p ctx)
{
	uint8 status = STATUS_READ;
#if PN532_COMM_INTERFACE == PN532_COMM_I2C
	//HAL_I2C_Mem_Read((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, status, 1, &status, 1, 200);
	//HAL_I2C_Master_Receive((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, &status, 1, 200);
	uint32 tickstart;
	uint8 c;
	uint8 i;
	tickstart = HAL_GetTick();
	//read status byte
	status = 0x01;
	//if(if_i2c_read((I2C_HandleTypeDef *)ctx, &c, 200, tickstart) == 0) {
	//	if((c & 0x01) == 0x01) status = (c & 0x01);
	//}
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_SPI
    //digitalWrite(_ss, LOW);
	((spi_context_p)ctx)->select(ctx);
    if_spi_write((spi_context_p)ctx, STATUS_READ);
    status = if_spi_read((spi_context_p)ctx, 0) & 1;
	((spi_context_p)ctx)->deselect(ctx);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
	switch(ctx->comm_interface) {
		case PN532_COMM_I2C:
			status = if_nfc_i2c_is_ready(ctx);
			break;
		case PN532_COMM_SPI:
			status = if_nfc_spi_is_ready(ctx);
			break;
	}
#endif
    return status;
}

void if_nfc_write_frame(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen)
{
	uint8 sum;
	uint8 length;
	uint8 i;
	uint8 checksum;
#if PN532_COMM_INTERFACE == PN532_COMM_I2C
	uint32 tickstart;
	uint8 cmdBuffer[300];
	uint16 cmdLen = 0;
	//cmdBuffer[cmdLen++]  = DATA_WRITE;
	cmdBuffer[cmdLen++]  = PN532_PREAMBLE;
	cmdBuffer[cmdLen++]  = PN532_STARTCODE1;
	cmdBuffer[cmdLen++]  = PN532_STARTCODE2;
    length = hlen + blen + 1;   // length of data field: TFI + DATA
	cmdBuffer[cmdLen++] = length;
	cmdBuffer[cmdLen++] = ~length + 1;
	cmdBuffer[cmdLen++] = PN532_HOSTTOPN532;
    sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
    for (i = 0; i < hlen; i++) {
		cmdBuffer[cmdLen++]  = header[i];
        sum += header[i];
	}
	//cmdLen = 7 + hlen;
    for (i = 0; i < blen; i++) {
		cmdBuffer[cmdLen++]  = body[i];
        sum += body[i];
    }
	//cmdLen += blen;
    checksum = ~sum + 1;        // checksum of TFI + DATA
	cmdBuffer[cmdLen++] = checksum;
	cmdBuffer[cmdLen++] = PN532_POSTAMBLE;
	
	tickstart = HAL_GetTick();
	HAL_I2C_Master_Transmit((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, cmdBuffer, cmdLen, 500);
	//if_i2c_stop(
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_SPI	
	((spi_context_p)ctx)->select(ctx);
    if_delay(2);               // wake up PN532

    if_spi_write((spi_context_p)ctx, DATA_WRITE);
    if_spi_write((spi_context_p)ctx, PN532_PREAMBLE);
    if_spi_write((spi_context_p)ctx, PN532_STARTCODE1);
    if_spi_write((spi_context_p)ctx, PN532_STARTCODE2);

    length = hlen + blen + 1;   // length of data field: TFI + DATA
	if_spi_write((spi_context_p)ctx, length);
    if_spi_write((spi_context_p)ctx, ~length + 1);         // checksum of length

    if_spi_write((spi_context_p)ctx, PN532_HOSTTOPN532);
    sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    //DMSG("write: ");

    for (i = 0; i < hlen; i++) {
        if_spi_write((spi_context_p)ctx, header[i]);
        sum += header[i];

        //DMSG_HEX(header[i]);
    }
    for (i = 0; i < blen; i++) {
        if_spi_write((spi_context_p)ctx, body[i]);
        sum += body[i];

        //DMSG_HEX(body[i]);
    }

    checksum = ~sum + 1;        // checksum of TFI + DATA
    if_spi_write((spi_context_p)ctx, checksum);
    if_spi_write((spi_context_p)ctx, PN532_POSTAMBLE);

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
	switch(ctx->comm_interface) {
		case PN532_COMM_I2C:
			if_nfc_i2c_write_frame(ctx, header, hlen, body, blen);
			break;
		case PN532_COMM_SPI:
			if_nfc_spi_write_frame(ctx, header, hlen, body, blen);
			break;
	}
#endif
}

int8_t if_nfc_read_ack_frame(nfc_context_p ctx)
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
	uint8 i;
	int8_t status = 0;
    uint16_t time = 0;
	uint16_t timeout = 20;
    uint8_t ackBuf[sizeof(PN532_ACK) + 1];
#if PN532_COMM_INTERFACE == PN532_COMM_I2C
	uint32 tickstart;
    uint8 c;
    /* Disable Pos */
	//restart start bit to change to read mode
	//generate start bit
	for(i=0;i<10;i++) {
		if(if_i2c_start_read((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, 5, HAL_GetTick()) == 0) {
			if(if_i2c_read((I2C_HandleTypeDef *)ctx, &c, 5, HAL_GetTick()) == 0) {
				if((c & 0x01) == 0x01) break;
			}
		}
		if_i2c_stop((I2C_HandleTypeDef *)ctx);
		if_delay(1);
	}
	if(i == 10) {
		return PN532_TIMEOUT;
	}
    for ( i = 0; i < sizeof(PN532_ACK); i++) {
		if_i2c_read((I2C_HandleTypeDef *)ctx, &ackBuf[i], 5, HAL_GetTick());
    }
	if_i2c_stop((I2C_HandleTypeDef *)ctx);
    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
#endif

#if PN532_COMM_INTERFACE == PN532_COMM_SPI	
	((spi_context_p)ctx)->select(ctx);
    if_delay(1);
    if_spi_write((spi_context_p)ctx, DATA_READ);

    for ( i = 0; i < sizeof(PN532_ACK); i++) {
        ackBuf[i] = if_spi_read((spi_context_p)ctx, 0);
    }

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
	switch(ctx->comm_interface) {
		case PN532_COMM_I2C:
			status = if_nfc_i2c_read_ack_frame(ctx);
			break;
		case PN532_COMM_SPI:
			status = if_nfc_spi_read_ack_frame(ctx);
			break;
	}
#endif
	return status;
}

int16_t if_nfc_read_response(nfc_context_p ctx, uint8_t buf[], uint16_t len, uint16_t timeout)
{
    int16_t result;
	uint8_t cmd;
	uint8_t length ;
	uint8 checksum;
    uint16_t time = 0;
	uint16 j;
	uint8 i, sum;
#if PN532_COMM_INTERFACE == PN532_COMM_I2C
	uint32 tickstart;
    uint8 c;
	for(i=0;i<10;i++) {
		if(if_i2c_start_read((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, 5, HAL_GetTick()) == 0) {
			if(if_i2c_read((I2C_HandleTypeDef *)ctx, &c, 5, HAL_GetTick()) == 0) {
				if((c & 0x01) == 0x01) break;
			}
		}
		if_i2c_stop((I2C_HandleTypeDef *)ctx);
		if_delay(1);
	}
	if(i == 10) {
		return PN532_TIMEOUT;
	}
	do {
		if (0x00 != if_i2c_read_byte((I2C_HandleTypeDef *)ctx)     ||       // PREAMBLE
                0x00 != if_i2c_read_byte((I2C_HandleTypeDef *)ctx)  ||       // STARTCODE1
                0xFF != if_i2c_read_byte((I2C_HandleTypeDef *)ctx)           // STARTCODE2
           ) {

            result = PN532_INVALID_FRAME;
            break;
        }
        length = if_i2c_read_byte((I2C_HandleTypeDef *)ctx) ;
        if (0 != (uint8_t)(length + if_i2c_read_byte((I2C_HandleTypeDef *)ctx) )) {   // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }
        cmd = ctx->command + 1;               // response command
        if (PN532_PN532TOHOST != if_i2c_read_byte((I2C_HandleTypeDef *)ctx)  || (cmd) != if_i2c_read_byte((I2C_HandleTypeDef *)ctx) ) {
            result = PN532_INVALID_FRAME;
            break;
        }
        length -= 2;
        if (length > len) {
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        sum = PN532_PN532TOHOST + cmd;
        for ( i = 0; i < length; i++) {
            buf[i] = if_i2c_read_byte((I2C_HandleTypeDef *)ctx) ;
            sum += buf[i];
        }
		
        checksum = if_i2c_read_byte((I2C_HandleTypeDef *)ctx) ;
        if (0 != (uint8_t)(sum + checksum)) {
            result = PN532_INVALID_FRAME;
            break;
        }
        result = length;
	} while(0);
	//stop transfer
	if_i2c_stop((I2C_HandleTypeDef *)ctx);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_SPI	
    while (!if_nfc_is_ready(ctx)) {
        if_delay(10);
        time++;
        if (timeout > 0 && time > timeout) {
            return PN532_TIMEOUT;
        }
    }
    ((spi_context_p)ctx)->select(ctx);
	if_delay(1);
    do {
        if_spi_write((spi_context_p)ctx, DATA_READ);

        if (0x00 != if_spi_read((spi_context_p)ctx, 0)      ||       // PREAMBLE
                0x00 != if_spi_read((spi_context_p)ctx, 0)  ||       // STARTCODE1
                0xFF != if_spi_read((spi_context_p)ctx, 0)           // STARTCODE2
           ) {

            result = PN532_INVALID_FRAME;
            break;
        }

        length = if_spi_read((spi_context_p)ctx, 0);
        if (0 != (uint8_t)(length + if_spi_read((spi_context_p)ctx, 0))) {   // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }

        cmd = ctx->command + 1;               // response command
        if (PN532_PN532TOHOST != if_spi_read((spi_context_p)ctx, 0) || (cmd) != if_spi_read((spi_context_p)ctx, 0)) {
            result = PN532_INVALID_FRAME;
            break;
        }

        DMSG("read:  ");
        DMSG_HEX(cmd);

        length -= 2;
        if (length > len) {
            for (i = 0; i < length; i++) {
                DMSG_HEX(if_spi_read(ctx, 0));                 // dump message
            }
            DMSG("\nNot enough space\n");
            if_spi_read((spi_context_p)ctx, 0);
            if_spi_read((spi_context_p)ctx, 0);
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        sum = PN532_PN532TOHOST + cmd;
        for ( i = 0; i < length; i++) {
            buf[i] = if_spi_read((spi_context_p)ctx, 0);
            sum += buf[i];

            DMSG_HEX(buf[i]);
        }
        DMSG('\n');

        checksum = if_spi_read((spi_context_p)ctx, 0);
        if (0 != (uint8_t)(sum + checksum)) {
            DMSG("checksum is not ok\n");
            result = PN532_INVALID_FRAME;
            break;
        }
        if_spi_read((spi_context_p)ctx, 0);         // POSTAMBLE

        result = length;
    } while (0);

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
#endif
#if PN532_COMM_INTERFACE == PN532_COMM_AUTO
	switch(ctx->comm_interface) {
		case PN532_COMM_I2C:
			result = if_nfc_i2c_read_response(ctx, buf, len, timeout);
			break;
		case PN532_COMM_SPI:
			result = if_nfc_spi_read_response(ctx, buf, len, timeout);
			break;
	}
#endif
    return result;
}

int8 if_nfc_write_command(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen)
{
	uint8 timeout;
    ctx->command = header[0];
    if_nfc_write_frame(ctx, header, hlen, body, blen);
	timeout = PN532_ACK_WAIT_TIME;
    while (!if_nfc_is_ready(ctx)) {
        if_delay(10);
        timeout--;
        if (0 == timeout) {
            return -2;
        }
    }
    if (if_nfc_read_ack_frame(ctx)) {
        return PN532_INVALID_ACK;
    }
    return 0;
}

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
void if_nfc_wakeup(nfc_context_p ctx)
{
    //HAL(wakeup)((nfc_context_p)ctx);
}


uint8 if_nfc_init(nfc_context_p ctx) {
	uint32 version;
	uint8 ret = -1;
	uint8 esid[SHARD_ESID_SIZE];
	uint16 dev_id;
	GPIO_InitTypeDef GPIO_InitStructure;
	if_flash_get_esid(esid, SHARD_ESID_SIZE);
	dev_id = ((esid[2] << 8) | esid[3]) & 0xFFF;
	switch(dev_id) {
		case 0x463:				//STM32F413 (ARGOS)
#if (PN532_COMM_INTERFACE & PN532_COMM_I2C) != 0
			//enable I2C port
			__HAL_RCC_GPIOB_CLK_ENABLE();		//i2c1
			GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
			GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;  //Í¨ÓÃÍÆÍEä³E
			GPIO_InitStructure.Pull = GPIO_PULLUP;
			GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
			//enable I2C peripheral
			//__HAL_RCC_I2C1_CONFIG(RCC_I2C1CLKSOURCE_PCLK1);
			__HAL_RCC_I2C1_CLK_ENABLE();
			ctx->base.i2c_base.Instance = I2C1;
#ifdef STM32F7
			ctx->base.i2c_base.Init.Timing = 0x2010091A;		//400Khz (Fast Mode)
#endif
#ifdef STM32F4
			ctx->base.i2c_base.Init.ClockSpeed = 200000;
			ctx->base.i2c_base.Init.DutyCycle = I2C_DUTYCYCLE_2;
#endif
			ctx->base.i2c_base.Init.OwnAddress1 = 0;
			ctx->base.i2c_base.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
			ctx->base.i2c_base.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			ctx->base.i2c_base.Init.OwnAddress2 = 0;
			ctx->base.i2c_base.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			ctx->base.i2c_base.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
			ctx->base.i2c_base.Mode= HAL_I2C_MODE_MASTER;
			
			__HAL_I2C_DISABLE((I2C_HandleTypeDef *)&ctx->base.i2c_base);
			HAL_I2C_Init((I2C_HandleTypeDef *)&ctx->base.i2c_base);
			//Configure Analogue filter 
			HAL_I2CEx_ConfigAnalogFilter((I2C_HandleTypeDef *)&ctx->base.i2c_base, I2C_ANALOGFILTER_ENABLE);
			//Configure Digital filter 
			HAL_I2CEx_ConfigDigitalFilter((I2C_HandleTypeDef *)&ctx->base.i2c_base, 0);
			__HAL_I2C_ENABLE((I2C_HandleTypeDef *)&ctx->base.i2c_base);
			if(HAL_I2C_IsDeviceReady((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, 10, 2000) == HAL_OK) {
				ret = 0;
			}
			ctx->comm_interface = PN532_COMM_I2C;
#endif	
			break;
		default:
//#if PN532_COMM_INTERFACE == PN532_COMM_SPI
			if_spi_init((rf_context_p)ctx, SPI_STATE_LSB_FIRST);
			ctx->comm_interface = PN532_COMM_SPI;
//#endif
			break;
	}
	ctx->begin = if_nfc_begin;
	ctx->wakeup = if_nfc_wakeup;
	ctx->writeCommand = if_nfc_write_command;
	ctx->readResponse = if_nfc_read_response;
	memset(ctx->uid, 0, 7);
	ctx->uidLen = 0;
	ctx->inListedTag = 0;
	ctx->nfc_state = 0;
	memset(ctx->key, 0, 6);
	version = if_nfc_get_firmware_version(ctx);
	//version = version;
	if(version != 0) {
		//device exist and initialized
		ctx->nfc_state |= NFC_STATE_INITIALIZED;
		ret = 0;
	}
	return ret;
}

uint8 if_nfc_sleep(nfc_context_p ctx) {
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;
	//disable RF field to decrease power consumption by 25mA
	if_nfc_set_rf_field(ctx, PN532_RF_DISABLE);
	//power down command
    pn532_packetbuffer[0] = PN532_COMMAND_POWERDOWN;
    //pn532_packetbuffer[1] = 0x28;  // SPI, RF detect
    pn532_packetbuffer[1] = 0x20;  // SPI, RF disabled
    HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 2, NULL, 0);
	return 0;
}

/**************************************************************************/
/*!
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t if_nfc_get_firmware_version(nfc_context_p ctx)
{
	int16 status;
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 1, NULL, 0)) {
        return 0;
    }

    // read data packet
    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);
    if (0 > status) {
        return 0;
    }

    response = pn532_packetbuffer[0];
    response <<= 8;
    response |= pn532_packetbuffer[1];
    response <<= 8;
    response |= pn532_packetbuffer[2];
    response <<= 8;
    response |= pn532_packetbuffer[3];

    return response;
}


/**************************************************************************/
/*!
    Writes an 8-bit value that sets the state of the PN532's GPIO pins

    @warning This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.

             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8 if_nfc_write_gpio(nfc_context_p ctx, uint8_t pinstate)
{
    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
    pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by I2C)

    //DMSG("Writing P3 GPIO: ");
    //DMSG_HEX(pn532_packetbuffer[1]);
    ///DMSG("\n");

    // Send the WRITEGPIO command (0x0E)
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 3, NULL, 0))
        return 0;

    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

/**************************************************************************/
/*!
    Reads the state of the PN532's GPIO pins

    @returns An 8-bit value containing the pin state where:

             pinState[0]  = P30
             pinState[1]  = P31
             pinState[2]  = P32
             pinState[3]  = P33
             pinState[4]  = P34
             pinState[5]  = P35
*/
/**************************************************************************/
uint8_t if_nfc_read_gpio(nfc_context_p ctx)
{
    pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 1, NULL, 0))
        return 0x0;

    HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);

    /* READGPIO response without prefix and suffix should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              P3 GPIO Pins
      b1              P7 GPIO Pins (not used ... taken by I2C)
      b2              Interface Mode Pins (not used ... bus select pins)
    */


    //DMSG("P3 GPIO: "); DMSG_HEX(pn532_packetbuffer[7]);
    //DMSG("P7 GPIO: "); DMSG_HEX(pn532_packetbuffer[8]);
    ///DMSG("I0I1 GPIO: "); DMSG_HEX(pn532_packetbuffer[9]);
    //DMSG("\n");

    return pn532_packetbuffer[0];
}

/**************************************************************************/
/*!
    @brief  Configures the SAM (Secure Access Module)
*/
/**************************************************************************/
uint8 if_nfc_sam_config(nfc_context_p ctx, uint8 mode)
{
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = mode; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;

    //DMSG("SAMConfig\n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 4, NULL, 0))
        return 0;

    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

/**************************************************************************/
/*!
    Sets the MxRtyPassiveActivation uint8_t of the RFConfiguration register

    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8 if_nfc_set_passive_activation_retries(nfc_context_p ctx, uint8_t maxRetries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries; // MxRtyPSL (default = 0x01)
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 5, NULL, 0))
        return 0x0;  // no ACK

    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

uint8 if_nfc_set_timing(nfc_context_p ctx, uint8_t atr_timeout, uint8 retry_timeout)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 2;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = atr_timeout; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = retry_timeout; // MxRtyPSL (default = 0x01)
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 4, NULL, 0))
        return 0x0;  // no ACK

    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

uint8 if_nfc_set_max_retry_com(nfc_context_p ctx, uint8 value){
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 4;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = value; // MxRtyATR (default = 0xFF)
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 3, NULL, 0))
        return 0x0;  // no ACK

    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

uint8 if_nfc_set_rf_field(nfc_context_p ctx, uint8 mode)
{
	uint8 ret;
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 1;    //config item 1
    pn532_packetbuffer[2] = mode; 		//bit 1 = auto-rfca, bit 0 = rf on/off
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 3, NULL, 0))
        return 0x0;  // no ACK

    ret = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);
	if(ret == 0) {
		if(mode & PN532_RF_ENABLE) {
			ctx->nfc_state |= NFC_STATE_ENABLED;
		} else {
			ctx->nfc_state &= ~NFC_STATE_ENABLED;
		}
	}
	return ret;
}

void if_nfc_set_parameters(nfc_context_p ctx, uint8 value) {
	pn532_packetbuffer[0] = PN532_COMMAND_SETPARAMETERS;
    pn532_packetbuffer[1] = value;    // Config item 5 (MaxRetries)
	//check for nfc_state
	if(ctx == NULL) return;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return;

    HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 2, NULL, 0);
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*!
    Waits for an ISO14443A target to enter the field

    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.
    @param  timeout       The number of tries before timing out
    @param  inlist        If set to true, the card will be inlisted

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8 if_nfc_read_passive_target_id(nfc_context_p ctx, uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout, uint8 inlist)
{
	uint8 i;
	uint16_t sens_res;
	int16 length = 3;
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;
	//check for nfc_state
	if(ctx == NULL) return 0;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return 0;
	if(cardbaudrate != 0) {
		pn532_packetbuffer[3] = 0x00;	// FF FF 01 00
		pn532_packetbuffer[4] = 0xFF;
		pn532_packetbuffer[5] = 0xFF;
		pn532_packetbuffer[6] = 0x00;
		pn532_packetbuffer[7] = 0x00;
		length = 8;
	}

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, length, NULL, 0)) {
        return 0x0;  // command failed
    }

    // read data packet
    if ((length = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout)) < 0) {
        return 0x0;
    }
	if(length < 0) return 0;
	if(pn532_packetbuffer[0] == 0) {
		return 0;
	}
	ctx->inListedTag = 1;
	

    return 1;
}


/***** Mifare Classic Functions ******/

/**************************************************************************/
/*!
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
*/
/**************************************************************************/
uint8 if_nfc_mfc_is_first_block (nfc_context_p ctx, uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock) % 4 == 0);
    else
        return ((uiBlock) % 16 == 0);
}

/**************************************************************************/
/*!
      Indicates whether the specified block number is the sector trailer
*/
/**************************************************************************/
uint8 if_nfc_mfc_is_trailer_block (nfc_context_p ctx, uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock + 1) % 4 == 0);
    else
        return ((uiBlock + 1) % 16 == 0);
}

/**************************************************************************/
/*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 bytes
                          key value

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mfc_authenticate (nfc_context_p ctx, uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData)
{
    uint8_t i;

    // Hang on to the key and uid data
    memcpy (ctx->key, keyData, 6);
    memcpy (ctx->uid, uid, uidLen);
	ctx->uidLen = uidLen;

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy (pn532_packetbuffer + 4, ctx->key, 6);
    for (i = 0; i < ctx->uidLen; i++) {
        pn532_packetbuffer[10 + i] = ctx->uid[i];              /* 4 bytes card ID */
    }

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 10 + ctx->uidLen, NULL, 0))
        return 0;

    // Read the response packet
    HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);

    // Check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (pn532_packetbuffer[0] != 0x00) {
        DMSG("Authentification failed\n");
        return 0;
    }

    return 1;
}

/**************************************************************************/
/*!
    Tries to read an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mfc_read_data_block (nfc_context_p ctx, uint8_t blockNumber, uint8_t *data)
{
    DMSG("Trying to read 16 bytes from block ");
    ///DMSG_INT(blockNumber);

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 4, NULL, 0)) {
        return 0;
    }

    /* Read the response packet */
    HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer),PN532_DEFAULT_TIMEOUT);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] != 0x00) {
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy (data, pn532_packetbuffer + 1, 16);

    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mfc_write_data_block (nfc_context_p ctx, uint8_t blockNumber, uint8_t *data)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
    memcpy (pn532_packetbuffer + 4, data, 16);        /* Data Payload */

    /* Send the command */
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 20, NULL, 0)) {
        return 0;
    }

    /* Read the response packet */
    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

/**************************************************************************/
/*!
    Formats a Mifare Classic card to store NDEF Records

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mfc_format_ndef (nfc_context_p ctx)
{
    uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
    // for the MAD sector in NDEF records (sector 0)

    // Write block 1 and 2 to the card
    if (!(if_nfc_mfc_write_data_block (ctx, 1, sectorbuffer1)))
        return 0;
    if (!(if_nfc_mfc_write_data_block (ctx, 2, sectorbuffer2)))
        return 0;
    // Write key A and access rights card
    if (!(if_nfc_mfc_write_data_block (ctx, 3, sectorbuffer3)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/**************************************************************************/
/*!
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mfc_write_ndefuri (nfc_context_p ctx, uint8_t sectorNumber, uint8_t uriIdentifier, const char *url)
{
    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    // Figure out how long the string is
    uint8_t len = strlen(url);
    uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, 0x00, 0xD1, 0x01, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	sectorbuffer1[3] = len + 5;
	sectorbuffer1[6] = len + 1;
	sectorbuffer1[8] = uriIdentifier;

    // Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) || (sectorNumber > 15))
        return 0;

    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38))
        return 0;

    // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    // in NDEF records

    if (len <= 6) {
        // Unlikely we'll get a url this short, but why not ...
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    } else if (len == 7) {
        // 0xFE needs to be wrapped around to next block
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    } else if ((len > 7) || (len <= 22)) {
        // Url fits in two blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    } else if (len == 23) {
        // 0xFE needs to be wrapped around to final block
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    } else {
        // Url fits in three blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, 16);
        memcpy (sectorbuffer3, url + 23, len - 24);
        sectorbuffer3[len - 22] = 0xFE;
    }

    // Now write all three blocks back to the card
    if (!(if_nfc_mfc_write_data_block (ctx, sectorNumber * 4, sectorbuffer1)))
        return 0;
    if (!(if_nfc_mfc_write_data_block (ctx, (sectorNumber * 4) + 1, sectorbuffer2)))
        return 0;
    if (!(if_nfc_mfc_write_data_block (ctx, (sectorNumber * 4) + 2, sectorbuffer3)))
        return 0;
    if (!(if_nfc_mfc_write_data_block (ctx, (sectorNumber * 4) + 3, sectorbuffer4)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/***** Mifare Ultralight Functions ******/

/**************************************************************************/
/*!
    Tries to read an entire 4-bytes page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
/**************************************************************************/
uint8_t if_nfc_mful_read_page (nfc_context_p ctx, uint8_t page, uint8_t *buffer)
{
    if (page >= 64) {
        DMSG("Page value out of range\n");
        return 0;
    }

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 4, NULL, 0)) {
        return 0;
    }

    /* Read the response packet */
    HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 bytes or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy (buffer, pn532_packetbuffer + 1, 4);
    } else {
        return 0;
    }

    // Return OK signal
    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 4-bytes data buffer at the specified page
    address.

    @param  page     The page number to write into.  (0..63).
    @param  buffer   The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t if_nfc_mful_write_page (nfc_context_p ctx, uint8_t page, uint8_t *buffer)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                           /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE_ULTRALIGHT; /* Mifare UL Write cmd = 0xA2 */
    pn532_packetbuffer[3] = page;                        /* page Number (0..63) */
    memcpy (pn532_packetbuffer + 4, buffer, 4);          /* Data Payload */

    /* Send the command */
    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 8, NULL, 0)) {
        return 0;
    }

    /* Read the response packet */
    return (0 < HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT));
}

/**************************************************************************/
/*!
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
uint8 if_nfc_data_exchange(nfc_context_p ctx, uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength)
{
    uint8_t i;
	uint8_t  length;
	int16 status;

    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = ctx->inListedTag;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 2, send, sendLength)) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, response, *responseLength, 5000);
    if (status < 0) {
        return -1;
    }

    if ((response[0] & 0x3f) != 0) {
        DMSG("Status code indicates an error\n");
        return -1;
    }

    length = status;
    length -= 1;

    if (length > *responseLength) {
        length = *responseLength; // silent truncation...
    }

    for ( i = 0; i < length; i++) {
        response[i] = response[i + 1];
    }
    *responseLength = length;

    return 0;
}

/**************************************************************************/
/*!
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
/**************************************************************************/
uint8 if_nfc_list_passive_target(nfc_context_p ctx, uint8 maxtg, uint8 type, uint8 * initiatorData, uint8 len)
{
	int16 status;
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = maxtg;
    pn532_packetbuffer[2] = type;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if((ctx->nfc_state & NFC_STATE_ENABLED) == 0) return -1;
	ctx->nfc_state &= (NFC_STATE_INITIALIZED | NFC_STATE_ENABLED);		//clear previous state, only mask initialized state
	
	if(type == NFC_MIFARE) {
		len = 0;
	} else {
		memcpy(pn532_packetbuffer + 3, initiatorData, len);
	}
	
    DMSG("inList passive target\n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, len + 3, NULL, 0)) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);
    if (status < 0) {
        return -1;
    }

    if (pn532_packetbuffer[0] != 1) {
        return -1;
    }
	//tag found
    ctx->inListedTag = pn532_packetbuffer[1];
	ctx->type = type;
	ctx->uidLen = 0;
	ctx->nfc_state |= NFC_STATE_CONNECTED;
	switch(type) {
		case NFC_MIFARE:
			ctx->uidLen = pn532_packetbuffer[5];

			for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
				ctx->uid[i] = pn532_packetbuffer[6 + i];
			}
			break;
	}
    return 0;
}


uint8 if_nfc_open(nfc_context_p ctx) {
	uint8 ret = NFC_BUSY(ctx);		//wait till nfc closed
	if(ret != 0) return ret;
	NFC_LOCK(ctx);
	return 0;
}

void if_nfc_close(nfc_context_p ctx) {
	NFC_UNLOCK(ctx);
}

uint8 if_nfc_get_atr(nfc_context_p ctx, uint8 * atr)
{
	int16 status;
	int16 length;
    pn532_packetbuffer[0] = PN532_COMMAND_INATR;
    pn532_packetbuffer[1] = ctx->inListedTag;
    pn532_packetbuffer[2] = 0;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    DMSG("inAtr \n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 3, NULL, 0)) {
        return 0;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 10000);
    if (status < 0) {
        return 0;
    }

    if (pn532_packetbuffer[0] != 1) {		//no error
        return 0;
    }
	if(status  < 12) return 0;			//no atr

    //ctx->inListedTag = pn532_packetbuffer[1];
	length = status -12;
	memcpy(atr, pn532_packetbuffer + 12, length);
    return length;
}

int8_t if_nfc_init_picc(nfc_context_p ctx) {
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if(NFC_BUSY(ctx) == 0) ctx->nfc_state &= (NFC_STATE_INITIALIZED | NFC_STATE_ENABLED);
	//if_nfc_set_parameters(ctx, PN532_PARAM_RATS | PN532_PARAM_PICC | PN532_PARAM_NAD);
	if_nfc_set_max_retry_com(ctx, 0x02);
	if_nfc_set_passive_activation_retries(ctx, 0x02);
	if_nfc_set_parameters(ctx, PN532_PARAM_RATS);
	if_nfc_sam_config(ctx, PN532_SAM_NORMAL); 		//normal mode
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
	return 0;
}

uint8 if_nfc_select(nfc_context_p ctx, uint8 logical_id)
{
	int16 status;
	int16 length;
    pn532_packetbuffer[0] = PN532_COMMAND_INSELECT;
    pn532_packetbuffer[1] = logical_id;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    DMSG("inSelect \n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 2, NULL, 0)) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 30000);
    if (status < 0) {
        return -1;
    }

    if (pn532_packetbuffer[0] != 0) {		//no error
        return -1;
    }
	return 0;
}

int8_t if_nfc_init_dep(nfc_context_p ctx) {
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if(NFC_BUSY(ctx) == 0) ctx->nfc_state &= (NFC_STATE_INITIALIZED | NFC_STATE_ENABLED);
	if_nfc_set_max_retry_com(ctx, 0x02);
	if_nfc_set_passive_activation_retries(ctx, 0x02);
	if_nfc_set_parameters(ctx, PN532_PARAM_RATS);
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
	if_nfc_sam_config(ctx, PN532_SAM_NORMAL); 		//normal mode
	return 0;
}

int8_t if_nfc_start_dep(nfc_context_p ctx, uint8 active, uint8 baudrate, uint8 * initiator) {
	int16 status;
	int16 length = 4;
    pn532_packetbuffer[0] = PN532_COMMAND_INJUMPFORDEP;
    pn532_packetbuffer[1] = active;
    pn532_packetbuffer[2] = baudrate;
	pn532_packetbuffer[3] = 0;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	ctx->nfc_state &= (NFC_STATE_INITIALIZED | NFC_STATE_ENABLED);		//clear previous state, only mask initialized state
	
	if(baudrate != 0 && initiator != NULL) {
		pn532_packetbuffer[3] = 0x01;
		memcpy(pn532_packetbuffer + 4, initiator, 5);
		length = 9;
	}

    DMSG("inJumpForDEP \n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, length, NULL, 0)) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 10000);
    if (status < 0) {
        return -1;
    }

    if (pn532_packetbuffer[0] != 0) {		//no error
        return -1;
    }
	ctx->inListedTag = pn532_packetbuffer[1];
	ctx->type = NFC_DEP;
	ctx->uidLen = 0;
	ctx->nfc_state |= NFC_STATE_CONNECTED;
	return 0;
}

int8_t if_nfc_start_psl(nfc_context_p ctx, uint8 active, uint8 baudrate, uint8 * initiator, uint8 ilen) {
	int16 status;
	int16 length = 4;
    pn532_packetbuffer[0] = PN532_COMMAND_INJUMPFORPSL;
    pn532_packetbuffer[1] = active;
    pn532_packetbuffer[2] = baudrate;
	pn532_packetbuffer[3] = 0;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if(initiator != NULL) {
		pn532_packetbuffer[3] |= 0x01;
		memcpy(pn532_packetbuffer + 4, initiator, ilen);
		length += ilen;
	}

    DMSG("inJumpForPSL \n");

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, length, NULL, 0)) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 2000);
    if (status < 0) {
        return -1;
    }

    if (pn532_packetbuffer[0] != 0) {		//no error
        return -1;
    }
	ctx->inListedTag = pn532_packetbuffer[1];
	return 0;
}



/**
 * Peer to Peer
 */
int8_t if_tag_init_dep(nfc_context_p ctx) {
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if_nfc_sam_config(ctx, PN532_SAM_NORMAL); 		//virtual card mode
	if_nfc_set_parameters(ctx, PN532_PARAM_ATR_RES);
	return 0;
}

int8_t if_tag_init_picc(nfc_context_p ctx) {
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	if_nfc_sam_config(ctx, PN532_SAM_NORMAL); 		//virtual card mode
	if_nfc_set_parameters(ctx, PN532_PARAM_PICC);
	return 0;
}

int8_t if_tag_connect_dep(nfc_context_p ctx) {
	uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET,
        0x03,
        0x04, 0x00,         	//SENS_RES
        0x00, 0x00, 0x00,   	//NFCID1
        0x40,               		//SEL_RES
	
		0x01, 0xFE,         // Parameters to build POL_RES
		0xA2, 0xA3, 0xA4,
		0xA5, 0xA6, 0xA7,
		0xC0, 0xC1, 0xC2,
		0xC3, 0xC4, 0xC5,
		0xC6, 0xC7, 
		0xFF, 0xFF,			//system code (POL_RES)
		0xAA, 0x99, 0x88, //NFCID3t (10 bytes)
		0x77, 0x66, 0x55, 0x44,
		0x33, 0x22, 0x11,

		0x01, 0x95, // length of general bytes
		0x03, 'o', 'r', 'b'  // length of historical bytes
    };
	return if_tag_init_as(ctx, command, sizeof(command), 1000); 
}

int8_t if_tag_connect_picc(nfc_context_p ctx) {
	uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET,
        0x05,
        0x04, 0x00,         	//SENS_RES
        0x00, 0x00, 0x00,   	//NFCID1
        0x20,               		//SEL_RES
	
		0x01, 0xFE,         // Parameters to build POL_RES
		0xA2, 0xA3, 0xA4,
		0xA5, 0xA6, 0xA7,
		0xC0, 0xC1, 0xC2,
		0xC3, 0xC4, 0xC5,
		0xC6, 0xC7, 
		0xFF, 0xFF,			//system code (POL_RES)
		0xAA, 0x99, 0x88, //NFCID3t (10 bytes)
		0x77, 0x66, 0x55, 0x44,
		0x33, 0x22, 0x11,

		0x01, 0x95, // length of general bytes
		0x03, 'o', 'r', 'b'  // length of historical bytes
    };
	return if_tag_init_as(ctx, command, sizeof(command), 1000); 
}

int8_t if_tag_init_as(nfc_context_p ctx, uint8_t* command, const uint8_t len, const uint16_t timeout) {
	int8 status;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	status = HAL(writeCommand)((nfc_context_p)ctx, command, len, NULL, 0);
    if (status < 0) {
        return -1;
    }

    status = HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
    if (status > 0) {
        return 1;
    } else if (PN532_TIMEOUT == status) {
        return 0;
    } else {
        return -2;
    }
}

uint8 if_tag_get_state(nfc_context_p ctx) {
	uint8 buf[8];
	uint8 status;
	uint8 len = sizeof(buf);
    buf[0] = PN532_COMMAND_TGGETTARGETSTATUS;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    if (HAL(writeCommand)((nfc_context_p)ctx, buf, 1, NULL, 0)) {
        return -1;
    }

	load_another_response:
    status = HAL(readResponse)((nfc_context_p)ctx, buf, len, 3000);
    return status & 0x0F;
}

int16_t if_tag_get_data(nfc_context_p ctx, uint8_t *buf, uint16_t len)
{
	uint8 i;
	uint16 length;
	int16 status;
	uint16 offset = 0;
	uint8 stat;
    buf[0] = PN532_COMMAND_TGGETDATA;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    if (HAL(writeCommand)((nfc_context_p)ctx, buf, 1, NULL, 0)) {
        return -1;
    }

	load_another_response:
    status = HAL(readResponse)((nfc_context_p)ctx, buf + offset, len, 30000);
    if (0 >= status) {
		switch(status) {
			case -1: goto load_another_response;		//still within range of reader
			default:
				return -1;
		}
    }
	if(status == 1) {
		return -1;
	}
	
    length = status - 1;
	stat = buf[offset] ;
    if ((stat & 0x3F) != 0) {
        DMSG("status is not ok\n");
        return -5;
    }

    for ( i = 0; i < length; i++) {
        buf[offset + i] = buf[offset + i + 1];
    }
	offset += length;
	
	if(stat & 0x40) {			//MI bit set
		goto load_another_response;
	}

    return offset;
}

uint8 if_tag_set_data(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen)
{
	int8 i;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
    if (hlen > (sizeof(pn532_packetbuffer) - 1)) {
        if ((body != 0) || (header == pn532_packetbuffer)) {
            DMSG("tgSetData:buffer too small\n");
            return -1;
        }

        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;
        if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 1, header, hlen)) {
            return -1;
        }
    } else {
        for (i = hlen; i != 0; i--){
            pn532_packetbuffer[i] = header[i-1];
        }
        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;

        if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, hlen + 1, body, blen)) {
            return -1;
        }
    }

    if (0 > HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000)) {
        return -1;
    }

    if (0 != pn532_packetbuffer[0]) {
        return -1;
    }

    return 0;
}

int16_t if_tag_release(nfc_context_p ctx, const uint8_t relevantTarget){

    pn532_packetbuffer[0] = PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = relevantTarget;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, 2, NULL, 0)) {
        return 0;
    }

    // read data packet
    return HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), PN532_DEFAULT_TIMEOUT);
}

int16_t if_tag_get_command(nfc_context_p ctx, uint8_t *buf, uint8_t len) {
	uint8 i;
	uint16 length;
	uint16 status;
	uint16 offset = 0;
	uint8 stat;
    buf[0] = PN532_COMMAND_TGGETINITIATORCOMMAND;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;

    if (HAL(writeCommand)((nfc_context_p)ctx, buf, 1, NULL, 0)) {
        return -1;
    }

	load_another_response:
    status = HAL(readResponse)((nfc_context_p)ctx, buf + offset, len, 3000);
    if (0 >= status) {
        return status;
    }
	
    length = status - 1;
	stat = buf[offset] ;
    if ((stat & 0x3F) != 0) {
        DMSG("status is not ok\n");
        return -5;
    }

    for ( i = 0; i < length; i++) {
        buf[offset + i] = buf[offset + i + 1];
    }
	offset += length;
	
	if(stat & 0x40) {			//MI bit set
		goto load_another_response;
	}

    return offset;
	
}

uint8 if_tag_set_response(nfc_context_p ctx, uint8_t *buf, uint8_t len) {
	pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;
	//check for nfc_state
	if(ctx == NULL) return -1;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return -1;
	memcpy(pn532_packetbuffer + 1, buf, len);
	if (HAL(writeCommand)((nfc_context_p)ctx, pn532_packetbuffer, len + 1, NULL, 0)) {
		return -1;
	}
    if (0 > HAL(readResponse)((nfc_context_p)ctx, pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000)) {
        return -1;
    }

    return pn532_packetbuffer[0];
}


#if PN532_COMM_INTERFACE == PN532_COMM_AUTO 

void if_nfc_i2c_begin(nfc_context_p ctx)
{
	uint8 i;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return;		//NFC not initialized
	//use I2C address select
	if(HAL_I2C_IsDeviceReady((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, 10, 2000) == HAL_OK) {
		i = 0;
	}
	//enable RF field
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
}


uint8 if_nfc_i2c_is_ready(nfc_context_p ctx)
{
	uint8 status = STATUS_READ;
	//HAL_I2C_Mem_Read((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, status, 1, &status, 1, 200);
	//HAL_I2C_Master_Receive((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, &status, 1, 200);
	uint32 tickstart;
	uint8 c;
	uint8 i;
	tickstart = HAL_GetTick();
	//read status byte
	status = 0x01;
    return status;
}

void if_nfc_i2c_write_frame(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen)
{
	uint8 sum;
	uint8 length;
	uint8 i;
	uint8 checksum;
	uint32 tickstart;
	uint8 cmdBuffer[300];
	uint16 cmdLen = 0;
	//cmdBuffer[cmdLen++]  = DATA_WRITE;
	cmdBuffer[cmdLen++]  = PN532_PREAMBLE;
	cmdBuffer[cmdLen++]  = PN532_STARTCODE1;
	cmdBuffer[cmdLen++]  = PN532_STARTCODE2;
    length = hlen + blen + 1;   // length of data field: TFI + DATA
	cmdBuffer[cmdLen++] = length;
	cmdBuffer[cmdLen++] = ~length + 1;
	cmdBuffer[cmdLen++] = PN532_HOSTTOPN532;
    sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
    for (i = 0; i < hlen; i++) {
		cmdBuffer[cmdLen++]  = header[i];
        sum += header[i];
	}
	//cmdLen = 7 + hlen;
    for (i = 0; i < blen; i++) {
		cmdBuffer[cmdLen++]  = body[i];
        sum += body[i];
    }
	//cmdLen += blen;
    checksum = ~sum + 1;        // checksum of TFI + DATA
	cmdBuffer[cmdLen++] = checksum;
	cmdBuffer[cmdLen++] = PN532_POSTAMBLE;
	
	tickstart = HAL_GetTick();
	HAL_I2C_Master_Transmit((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, cmdBuffer, cmdLen, 500);
}

int8_t if_nfc_i2c_read_ack_frame(nfc_context_p ctx)
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
	uint8 i;
    uint16_t time = 0;
	uint16_t timeout = 20;
    uint8_t ackBuf[sizeof(PN532_ACK) + 1];
	uint32 tickstart;
    uint8 c;
	uint16 index= 0;
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, ackBuf, sizeof(ackBuf), 50);
	if(status == HAL_ERROR || status == HAL_BUSY) {
		return PN532_TIMEOUT;
	} else {
		c = ackBuf[index++];
		if((c & 0x01) == 0x01) return 0;
		for ( i = 0; i < sizeof(PN532_ACK); i++) {
			ackBuf[i] = ackBuf[index++];
		}
	}
    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
}

int16_t if_nfc_i2c_read_response(nfc_context_p ctx, uint8_t buf[], uint16_t len, uint16_t timeout)
{
    int16_t result = PN532_TIMEOUT;
	uint8_t cmd;
	uint8_t length ;
	uint8 checksum;
    uint16_t time = 0;
	uint16 j;
	uint8 i, sum;
	uint32 tickstart;
    uint8 c;
	uint8 bbuf[280];
	uint16 index= 0;
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive((I2C_HandleTypeDef *)ctx, PN532_I2C_ADDR, bbuf, len, 100);
	if(status == HAL_ERROR || status == HAL_BUSY) {
		return PN532_TIMEOUT;
	} else {
		do {
			if (0x00 != bbuf[index++]     ||       // PREAMBLE
					0x00 != bbuf[index++]  ||       // STARTCODE1
					0xFF != bbuf[index++]           // STARTCODE2
			   ) {

				result = PN532_INVALID_FRAME;
				break;
			}
			length = bbuf[index++] ;
			if (0 != (uint8_t)(length + bbuf[index++])) {   // checksum of length
				result = PN532_INVALID_FRAME;
				break;
			}
			cmd = ctx->command + 1;               // response command
			if (PN532_PN532TOHOST != bbuf[index++]  || (cmd) != bbuf[index++] ) {
				result = PN532_INVALID_FRAME;
				break;
			}
			length -= 2;
			if (length > len) {
				result = PN532_NO_SPACE;  // not enough space
				break;
			}

			sum = PN532_PN532TOHOST + cmd;
			for ( i = 0; i < length; i++) {
				buf[i] = bbuf[index++];
				sum += buf[i];
			}
			
			checksum = bbuf[index++];
			if (0 != (uint8_t)(sum + checksum)) {
				result = PN532_INVALID_FRAME;
				break;
			}
			result = length;
		} while(0);
	}
    return result;
}


void if_nfc_spi_begin(nfc_context_p ctx)
{
	uint8 i;
	if((ctx->nfc_state & NFC_STATE_INITIALIZED) == 0) return;		//NFC not initialized
	//use CS pin to wakeup device
	((spi_context *)ctx)->select(ctx);
    if_delay(2);
	((spi_context *)ctx)->deselect(ctx);
	//enable RF field
	if_nfc_set_rf_field(ctx, PN532_RF_ENABLE | PN532_RFCA_ENABLE);
}


uint8 if_nfc_spi_is_ready(nfc_context_p ctx)
{
	uint8 status = STATUS_READ;
    //digitalWrite(_ss, LOW);
	((spi_context_p)ctx)->select(ctx);
    if_spi_write((spi_context_p)ctx, STATUS_READ);
    status = if_spi_read((spi_context_p)ctx, 0) & 1;
	((spi_context_p)ctx)->deselect(ctx);
    return status;
}

void if_nfc_spi_write_frame(nfc_context_p ctx, uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen)
{
	uint8 sum;
	uint8 length;
	uint8 i;
	uint8 checksum;
	((spi_context_p)ctx)->select(ctx);
    if_delay(2);               // wake up PN532

    if_spi_write((spi_context_p)ctx, DATA_WRITE);
    if_spi_write((spi_context_p)ctx, PN532_PREAMBLE);
    if_spi_write((spi_context_p)ctx, PN532_STARTCODE1);
    if_spi_write((spi_context_p)ctx, PN532_STARTCODE2);

    length = hlen + blen + 1;   // length of data field: TFI + DATA
	if_spi_write((spi_context_p)ctx, length);
    if_spi_write((spi_context_p)ctx, ~length + 1);         // checksum of length

    if_spi_write((spi_context_p)ctx, PN532_HOSTTOPN532);
    sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    //DMSG("write: ");

    for (i = 0; i < hlen; i++) {
        if_spi_write((spi_context_p)ctx, header[i]);
        sum += header[i];

        //DMSG_HEX(header[i]);
    }
    for (i = 0; i < blen; i++) {
        if_spi_write((spi_context_p)ctx, body[i]);
        sum += body[i];

        //DMSG_HEX(body[i]);
    }

    checksum = ~sum + 1;        // checksum of TFI + DATA
    if_spi_write((spi_context_p)ctx, checksum);
    if_spi_write((spi_context_p)ctx, PN532_POSTAMBLE);

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
}

int8_t if_nfc_spi_read_ack_frame(nfc_context_p ctx)
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
	uint8 i;
    uint16_t time = 0;
	uint16_t timeout = 20;
    uint8_t ackBuf[sizeof(PN532_ACK) + 1];

	((spi_context_p)ctx)->select(ctx);
    if_delay(1);
    if_spi_write((spi_context_p)ctx, DATA_READ);

    for ( i = 0; i < sizeof(PN532_ACK); i++) {
        ackBuf[i] = if_spi_read((spi_context_p)ctx, 0);
    }

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
}

int16_t if_nfc_spi_read_response(nfc_context_p ctx, uint8_t buf[], uint16_t len, uint16_t timeout)
{
    int16_t result;
	uint8_t cmd;
	uint8_t length ;
	uint8 checksum;
    uint16_t time = 0;
	uint16 j;
	uint8 i, sum;
    while (!if_nfc_is_ready(ctx)) {
        if_delay(10);
        time++;
        if (timeout > 0 && time > timeout) {
            return PN532_TIMEOUT;
        }
    }
    ((spi_context_p)ctx)->select(ctx);
	if_delay(1);
    do {
        if_spi_write((spi_context_p)ctx, DATA_READ);

        if (0x00 != if_spi_read((spi_context_p)ctx, 0)      ||       // PREAMBLE
                0x00 != if_spi_read((spi_context_p)ctx, 0)  ||       // STARTCODE1
                0xFF != if_spi_read((spi_context_p)ctx, 0)           // STARTCODE2
           ) {

            result = PN532_INVALID_FRAME;
            break;
        }

        length = if_spi_read((spi_context_p)ctx, 0);
        if (0 != (uint8_t)(length + if_spi_read((spi_context_p)ctx, 0))) {   // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }

        cmd = ctx->command + 1;               // response command
        if (PN532_PN532TOHOST != if_spi_read((spi_context_p)ctx, 0) || (cmd) != if_spi_read((spi_context_p)ctx, 0)) {
            result = PN532_INVALID_FRAME;
            break;
        }

        DMSG("read:  ");
        DMSG_HEX(cmd);

        length -= 2;
        if (length > len) {
            for (i = 0; i < length; i++) {
                DMSG_HEX(if_spi_read(ctx, 0));                 // dump message
            }
            DMSG("\nNot enough space\n");
            if_spi_read((spi_context_p)ctx, 0);
            if_spi_read((spi_context_p)ctx, 0);
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        sum = PN532_PN532TOHOST + cmd;
        for ( i = 0; i < length; i++) {
            buf[i] = if_spi_read((spi_context_p)ctx, 0);
            sum += buf[i];

            DMSG_HEX(buf[i]);
        }
        DMSG('\n');

        checksum = if_spi_read((spi_context_p)ctx, 0);
        if (0 != (uint8_t)(sum + checksum)) {
            DMSG("checksum is not ok\n");
            result = PN532_INVALID_FRAME;
            break;
        }
        if_spi_read((spi_context_p)ctx, 0);         // POSTAMBLE

        result = length;
    } while (0);

    //digitalWrite(_ss, HIGH);
	((spi_context_p)ctx)->deselect(ctx);
    return result;
}

#endif


