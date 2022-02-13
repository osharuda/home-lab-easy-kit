/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief C source file for communication protocol implementation over i2c bus.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "fw.h"

/// \addtogroup group_communication_i2c_impl
///  @{

/// \brief Timestamp when #i2c_pool_devices() was called for the last time. It is used to figure out which of virtual
///        devices should be notified with tag_DeviceContext#on_polling() callback.
volatile uint64_t g_last_usClock __attribute__ ((aligned));

/// \brief This value indicates direction of ongoing communication. Non zero indicates master (software) is reading data.
///        from #g_resp_header and corresponding virtual device buffer. Zero indicates software (master) is writing data
///        into #g_cmd_header receive buffer (#g_recv_buffer).
volatile uint8_t g_transmit = 0;	// shows current transfer direction

/// \brief This variable stores communication protocol status. It stores both current device id and communication status
///        flags (#COMM_STATUS_BUSY, #COMM_STATUS_FAIL, #COMM_STATUS_CRC, #COMM_STATUS_OVF)
volatile uint8_t g_comm_status = 0;

/// \brief This variable indicates ongoing communication operation. Non-zero means ongoing communication is in progress.
///        Zero means no active communication at the moment.
volatile uint8_t g_i2c_busy = 0;

/// \def BUS_CMD_NONE
/// \brief Indicates no operation is requested by master (software) for given virtual device.
#define BUS_CMD_NONE  0

/// \def BUS_CMD_WRITE
/// \brief Indicates command is received (wrote) from the master (software) for given virtual device. This command must be handled
///        by virtual device.
#define BUS_CMD_WRITE 1

/// \def BUS_CMD_READ
/// \brief Indicates virtual device buffer was read and transmitted to the master (software). Virtual device should be
///        notified via tag_DeviceContext#on_read_done() callback.
#define BUS_CMD_READ  2

/// \brief Indicates type of completed operation. It's value is used in #main() infinite loop in order to perform action
///        requested by master (software) for virtual device. It may be equal to either #BUS_CMD_NONE, #BUS_CMD_WRITE or
///        #BUS_CMD_READ
volatile uint8_t g_cmd_type = BUS_CMD_NONE;

// Receive variables
/// \brief Buffer for #tag_CommCommandHeader structure to be received from the master(software)
CommCommandHeader g_cmd_header;

/// \brief Pointer to the #g_cmd_header_ptr. This variable is used for clearer code to avoid multiple type conversions.
volatile uint8_t* g_cmd_header_ptr = (volatile uint8_t*)&g_cmd_header;

/// \brief Receive buffer that is used to receive data from master (software). This is a place where master sends data.
uint8_t g_recv_buffer[COMM_BUFFER_LENGTH];

/// \brief Total number of bytes received counter. It counts all bytes sent by master (software) to firmware except those
///        bytes which are exceed receive buffer (#g_recv_buffer) length defined by #COMM_BUFFER_LENGTH.
/// \note This value includes #g_cmd_header
volatile uint16_t g_recv_total_pos=0;

/// \brief Number of bytes wrote into receive buffer (#g_recv_buffer). Later this value is passed to tag_DeviceContext#on_command() callback.
/// \note This value excludes #g_cmd_header
volatile uint16_t g_recv_data_pos=0;

// Transmit variables
/// \brief Buffer for #tag_CommResponseHeader structure to be sent to the master(software)
CommResponseHeader g_resp_header;

/// \brief Pointer to the #g_resp_header. This variable is used for clearer code to avoid multiple type conversions.
volatile uint8_t* g_resp_header_ptr = (volatile uint8_t*)&g_resp_header;

/// \brief Total number of bytes transmitted counter. It counts all bytes sent to the master, including #COMM_BAD_BYTE
/// \note This variable may be decremented when transmission is complete. It is made because firmware preload a byte into
///       I2C data register. I2C stop condition is always unexpected, therefore one extra byte is preloaded on stop condition
///       in data register. Firmware should count this byte back on stop condition.
volatile uint16_t g_tran_total=0;   // Total number of bytes transmitted

/// \brief Total number of bytes transmitted to master (software) from virtual device buffer. This value is passed to
///        tag_DeviceContext#on_read_done() callback.
volatile uint16_t g_tran_dev_pos=0;	// Number of bytes transmitted from device buffer

/// \brief Communication control sum accumulator. Start value is defined by #COMM_CRC_INIT_VALUE
volatile uint8_t g_crc = COMM_CRC_INIT_VALUE; // CRC accumulator

/// \brief Value of the last byte put into (or read from) I2C data register. It's value is used for correct control sum calculations.
volatile uint8_t g_last_byte = 0;

// Registered device
/// \brief Array of the device context pointers. This array is field
volatile PDeviceContext g_devices[COMM_MAX_DEV_ADDR + 1];

/// \brief Pointer to the current virtual device context
PDeviceContext g_cur_device = 0;
/// @}

static inline void i2c_transmit_stop(void);
static inline void i2c_receive_stop(void);
static inline void i2c_call_command_callback(void);
static inline void i2c_call_readdata_callback(void);

void i2c_bus_reinit(void) {
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    // Change GPIO as required
    gpio.GPIO_Pin = I2C_BUS_SDA_PIN_MASK;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_BUS_SDA_PORT, &gpio);

    gpio.GPIO_Pin = I2C_BUS_SCL_PIN_MASK;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_BUS_SCL_PORT, &gpio);

    // Setup I2C
    I2C_DeInit(I2C_BUS_PERIPH);

    i2c.I2C_Ack                     = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress     = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed              = I2C_BUS_CLOCK_SPEED;
    i2c.I2C_DutyCycle               = I2C_DutyCycle_2;
    i2c.I2C_Mode                    = I2C_Mode_I2C;
    i2c.I2C_OwnAddress1             = I2C_FIRMWARE_ADDRESS << 1;

    // Initialize and enable I2C
    DISABLE_IRQ
    I2C_Init(I2C_BUS_PERIPH, &i2c);
    ENABLE_IRQ

    // Disable two addresses
    I2C_DualAddressCmd(I2C_BUS_PERIPH, DISABLE);

    // Disable general call command
    I2C_GeneralCallCmd(I2C_BUS_PERIPH, DISABLE);

    // Disable stretching because of Raspberry PI issue
    I2C_StretchClockCmd(I2C_BUS_PERIPH, DISABLE);

    NVIC_SetPriority(I2C_BUS_EV_IRQ, IRQ_PRIORITY_I2C_EV);
    NVIC_EnableIRQ(I2C_BUS_EV_IRQ);

    NVIC_SetPriority(I2C_BUS_ER_IRQ, IRQ_PRIORITY_I2C_ER);
    NVIC_EnableIRQ(I2C_BUS_ER_IRQ);

    // Enable interrupts
    I2C_ITConfig(I2C_BUS_PERIPH, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, ENABLE);

    I2C_Cmd(I2C_BUS_PERIPH, ENABLE);
}

void i2c_bus_init(void)
{
    IS_ALIGNED(&g_last_usClock, sizeof(uint64_t));
    memset((void*)g_devices, 0, sizeof(g_devices));
    i2c_bus_reinit();
    g_last_usClock = get_us_clock();
}

void comm_register_device(PDeviceContext dev_ctx) {
	IS_ALIGNED(dev_ctx, sizeof(uint64_t));
	IS_ALIGNED(&(dev_ctx->polling_period), sizeof(uint64_t));
	IS_ALIGNED(&(dev_ctx->next_pooling_ev), sizeof(uint64_t));

	if (dev_ctx->device_id <= COMM_MAX_DEV_ADDR) {
		g_devices[dev_ctx->device_id] = dev_ctx;
		uint64_t now = get_us_clock();
		if (dev_ctx->on_polling!=0 && dev_ctx->polling_period>0) {
			dev_ctx->next_pooling_ev = now + dev_ctx->polling_period;
			dev_ctx->next_pooling_ovrrun = (now >= dev_ctx->next_pooling_ev) ? 1 : 0;
		}
	}
}


PDeviceContext comm_dev_context(uint8_t cmd_byte) {
	return g_devices[cmd_byte & COMM_MAX_DEV_ADDR];
}

void i2c_set_sync(volatile uint8_t* v, uint8_t nv) {
	uint8_t done = 0;
	do {
		DISABLE_IRQ
		if (g_i2c_busy==0) {
			*v = nv;
			done = 1;
		}
		ENABLE_IRQ
	} while(done==0);
}

void comm_done(uint8_t status) {
	uint8_t done = 0;
	do {
		DISABLE_IRQ
		if (g_i2c_busy==0) {
			g_comm_status = (status & (~COMM_MAX_DEV_ADDR)) | (g_comm_status & COMM_MAX_DEV_ADDR);
			done = 1;
		}
		ENABLE_IRQ
	} while(done==0);
}

static inline void i2c_call_command_callback(void) {
			g_cur_device->on_command(g_cmd_header.command_byte, g_recv_buffer, g_recv_data_pos);
			i2c_bus_reinit();	// Re-enable I2C
}

static inline void i2c_call_readdata_callback(void) {
			g_cur_device->on_read_done(g_cur_device->device_id, g_tran_dev_pos);
			i2c_bus_reinit();	// Re-enable I2C
}

// Called to check if there were command and to call device callbacks
void i2c_check_command(void) {

	if (g_cmd_type==BUS_CMD_WRITE) {
		i2c_call_command_callback();
		i2c_set_sync(&g_cmd_type, BUS_CMD_NONE);
	} else if (g_cmd_type==BUS_CMD_READ) {
		i2c_call_readdata_callback();
		i2c_set_sync(&g_cmd_type, BUS_CMD_NONE);
	}
}

void i2c_pool_devices(void) {
	uint64_t now = get_us_clock();
	if (g_last_usClock >= now) {
		// overrun
		for (uint32_t i=0; i <= COMM_MAX_DEV_ADDR; i++) {
			PDeviceContext pdev = g_devices[i];
			if (pdev!=0) {
				g_devices[i]->next_pooling_ovrrun = 0;
			}
		}
	}

	for (uint8_t i=0; i <= COMM_MAX_DEV_ADDR; i++) {
		PDeviceContext pdev = g_devices[i];
		if (pdev!=0 &&
			pdev->on_polling!=0 &&
			pdev->polling_period>0 &&
			pdev->next_pooling_ovrrun==0 &&
			pdev->next_pooling_ev<=now)
		{
			pdev->on_polling(i);
			pdev->next_pooling_ev = now + pdev->polling_period;
			pdev->next_pooling_ovrrun = (now >= pdev->next_pooling_ev) ? 1 : 0;
		}
	}

	g_last_usClock = now;
}

void i2c_addr_init(uint8_t transmit, uint8_t first_byte_sent) {
	uint8_t prev_crc = g_crc;
	g_crc = COMM_CRC_INIT_VALUE;

	g_i2c_busy = 1; // Lock changes during communication
	g_cmd_type = BUS_CMD_NONE;
	g_tran_total = 0;

	if (transmit) {
		// Transmit
		g_tran_dev_pos = 0;
		g_transmit = 1;
		g_tran_total = first_byte_sent;

		if (first_byte_sent) {
			g_crc = g_crc ^ prev_crc;
		}

		g_resp_header.comm_status = g_comm_status;
		g_resp_header.last_crc = prev_crc;
		g_resp_header.length = 0;

		if (IS_CLEARED(g_resp_header.comm_status, COMM_STATUS_BUSY)) {
			if (g_cur_device!=0) {		// if a device is selected
				if (g_cur_device->buffer!=0) {
					g_resp_header.length = g_cur_device->bytes_available;
 				} else if (g_cur_device->circ_buffer!=0) {
 					// For circular buffer use circbuf_len()
 					circbuf_start_read(g_cur_device->circ_buffer);
 					g_resp_header.length = circbuf_total_len(g_cur_device->circ_buffer);

 					if (circbuf_get_ovf(g_cur_device->circ_buffer)) {
 	 					g_resp_header.comm_status |= COMM_STATUS_OVF;
 					}
 				}
			}
		}
	} else {
		// Receive (this is new command)
		g_transmit = 0;

		if (IS_CLEARED(g_comm_status, COMM_STATUS_BUSY)) {
			g_recv_total_pos=0;
			g_recv_data_pos=0;

			// Transmit (just in case)
			g_tran_dev_pos = 0;
			g_tran_total = 0;
			memset(&g_resp_header, 0, sizeof(g_resp_header));
		}
	}
}

void i2c_receive_byte(void) {
	g_last_byte = I2C_BUS_PERIPH->DR;

	if (IS_SET(g_comm_status, COMM_STATUS_BUSY))	{
		// Do not do anything until BUSY flag is not cleared by device
	} else
	if (g_recv_total_pos < sizeof(CommCommandHeader)) {
		// receive command header
		g_cmd_header_ptr[g_recv_total_pos++] = g_last_byte;


		if (g_recv_total_pos == COMM_COMMAND_BYTE_OFFSET + 1) {
			// Command byte received, find device
			uint8_t dev_id = (g_last_byte & COMM_MAX_DEV_ADDR);
			g_cur_device = g_devices[dev_id];
			g_comm_status = COMM_MAX_DEV_ADDR & dev_id;
		}

		// Calculated control sum (except for control_crc byte)
		if (g_recv_total_pos != COMM_CRC_OFFSET + 1) {
			g_crc = g_crc ^ g_last_byte;
		}
	} else if (g_recv_data_pos < COMM_BUFFER_LENGTH) {
		// receive actual command data
		g_recv_buffer[g_recv_data_pos++] = g_last_byte;
		g_recv_total_pos++;
		g_crc = g_crc ^ g_last_byte;
	} else {
		SET_FLAGS(g_comm_status, COMM_STATUS_FAIL);
	}
}

void i2c_transmit_byte(void) {
	if (g_tran_total<sizeof(CommResponseHeader)) {
		g_last_byte = g_resp_header_ptr[g_tran_total];
		I2C_BUS_PERIPH->DR = g_last_byte;
	} else if (g_cur_device->buffer!=0 && g_tran_dev_pos<g_resp_header.length) {
		g_last_byte = g_cur_device->buffer[g_tran_dev_pos];
		I2C_BUS_PERIPH->DR = g_last_byte;
		g_tran_dev_pos++;
	} else if (g_cur_device->circ_buffer!=0 && g_tran_dev_pos<g_resp_header.length) {
	    circbuf_get_byte(g_cur_device->circ_buffer, &g_last_byte);
		I2C_BUS_PERIPH->DR = g_last_byte;
		g_tran_dev_pos++;
	} else {
		// We may be here because of:
		// 1. Device is BUSY
		// 2. Buffer is empty
		// 3. Device doesn't support read operations
		g_last_byte = COMM_BAD_BYTE;
		I2C_BUS_PERIPH->DR = g_last_byte;
	}
	g_tran_total++;
	g_crc = g_crc ^ g_last_byte;
}

static inline void i2c_transmit_stop(void) {
	if (	g_cur_device->on_read_done!=0 &&
            IS_CLEARED(g_comm_status, COMM_STATUS_FAIL | COMM_STATUS_CRC) &&
			g_tran_dev_pos>0) {
		g_i2c_busy = 0; // Unlock protocol before calling callback
		g_cmd_type = BUS_CMD_READ;
	}
}

static inline void i2c_receive_stop(void) {
	if(g_cmd_header.length!=g_recv_data_pos) {
		SET_FLAGS(g_comm_status, COMM_STATUS_FAIL);
	}

	if (g_cmd_header.control_crc!=g_crc) {
		SET_FLAGS(g_comm_status, COMM_STATUS_CRC);
	}

	if (g_cur_device->on_command!=0 &&
		IS_CLEARED(g_comm_status, COMM_STATUS_FAIL | COMM_STATUS_CRC)) {
		SET_FLAGS(g_comm_status, COMM_STATUS_BUSY);
		g_i2c_busy = 0; // Unlock protocol before calling callback
		g_cmd_type = BUS_CMD_WRITE;
	}
}

void i2c_stop(void) {
	I2C_Cmd(I2C_BUS_PERIPH, DISABLE);

	if (g_transmit==1 && g_tran_total>1) {
		// special case : if series of bytes are transmitted one (the last) byte is always discarded because it is stored in data register, while NACK
		// is returned when shift register becomes empty. In the same time DR is not empty, therefore we have to reverse this byte back

		// decrease counters
		if (g_tran_dev_pos>0 && g_tran_total-g_tran_dev_pos==sizeof(CommResponseHeader)) {
			// MCU was required to sent fake buffers because no more data was in buffer
			g_tran_dev_pos--;
		}
		g_tran_total--;

		// correct CRC - just XOR one more time last byte sent
		g_crc = g_crc ^ g_last_byte;
	}

	if (g_transmit==1 && IS_CLEARED(g_resp_header.comm_status, COMM_STATUS_BUSY) && g_tran_dev_pos > 0) {
		i2c_transmit_stop();
	} else if (g_transmit==0 && IS_CLEARED(g_comm_status, COMM_STATUS_BUSY) && g_recv_total_pos >= sizeof(CommCommandHeader)) {
		i2c_receive_stop();
	}

	g_i2c_busy = 0;

	if (g_cmd_type == BUS_CMD_NONE) {
		// No callback will be called, enable i2c here
		i2c_bus_reinit();
	}
}

MAKE_ISR(I2C_BUS_ER_ISR) {
	volatile uint32_t flags = (I2C_BUS_PERIPH->SR2) | (I2C_BUS_PERIPH->SR1 << 16); // NOTE: This sequence doesn't clear ADD. Do not do this here, until DR is not set in transmit mode
	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_AF, I2C_SR2_TRA)) {
		// STOPF event is not called during transmission when NACK is received, therefore this event should be received here.
		i2c_stop();
		CLEAR_FLAGS(I2C_BUS_PERIPH->SR1, I2C_SR1_AF);
	}
}

MAKE_ISR(I2C_BUS_EV_ISR) {
	volatile uint32_t flags = (I2C_BUS_PERIPH->SR2) | (I2C_BUS_PERIPH->SR1 << 16); // NOTE: This sequence doesn't clear ADD. Do not do this here, until DR is not set in transmit mode

   	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_ADDR | I2C_SR1_TXE, I2C_SR2_BUSY)) {
   		// SPECIAL CASE: We have to write first byte of the CommResponseHeader structure here ASAP. The first byte will be CRC sum for the previous operation
		I2C_BUS_PERIPH->DR = g_crc;
		i2c_addr_init(1, 1);
	} else
	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_ADDR, I2C_SR2_BUSY)) {
		if (CHECK_SR1SR2_FLAGS(flags, 0, I2C_SR2_TRA)) {
			i2c_addr_init(1, 0); // Transmit
		} else {
			i2c_addr_init(0, 0); // Receive
		}
	}

   	flags = (I2C_BUS_PERIPH->SR1 << 16) | (I2C_BUS_PERIPH->SR2);  // NOTE: This sequence clear ADD. It should be done here

   	// --------------------- RECEIVING DATA ---------------------
	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_RXNE, I2C_SR2_BUSY)) {
		i2c_receive_byte();
	}

	// --------------------- TRANSMITING DATA ---------------------
	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_TXE, I2C_SR2_BUSY)) {
		i2c_transmit_byte();
	}

	// --------------------- STOP ---------------------
	if (CHECK_SR1SR2_FLAGS(flags, I2C_SR1_STOPF, 0))
	{
		i2c_stop();

		I2C_Cmd(I2C_BUS_PERIPH, ENABLE); // This sequence should clear STOPF flag
		flags = (I2C_BUS_PERIPH->SR1 << 16) | (I2C_BUS_PERIPH->SR2); // reread flags
	}
}
