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

#include "fw.h"


#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "sys_tick_counter.h"


#define ISR_EV_DEBUG_TRANSMIT 0

/// \def USE_I2C_TRACKING
/// \brief When this macro is not zero, a special array is filled with debug information related to the I2C communication.
///        It is possible to dump this array with 'p/x g_i2c_track' gdb command, and to use output value to get the latest
///        event inside I2C bus. If disabled tracking is not used.
#define USE_I2C_TRACKING 1

#if USE_I2C_TRACKING
struct _I2C_DBG_STRUCT {
    uint16_t ev_count;
    uint16_t sr1;
    uint16_t sr2;
    uint16_t location;
};

#define _I2C_TRACK_SIZE 256
struct _I2C_DBG_STRUCT g_i2c_track[_I2C_TRACK_SIZE];
volatile uint32_t g_i2c_track_count = 0;
volatile uint16_t g_i2c_ev_count = 0;
#define I2C_STATUS_TRACK(_sr1, _sr2, loc)   g_i2c_track[g_i2c_track_count].sr1 = (_sr1); \
                                g_i2c_track[g_i2c_track_count].sr2 = (_sr2);        \
                                g_i2c_track[g_i2c_track_count].ev_count = g_i2c_ev_count; \
                                g_i2c_track[g_i2c_track_count].location = (loc); \
                                g_i2c_track_count++;                                     \
                                g_i2c_track_count = g_i2c_track_count %  _I2C_TRACK_SIZE;                                                        \
                                assert_param(g_i2c_track_count < _I2C_TRACK_SIZE);

#define I2C_EV_COUNT g_i2c_ev_count++;


#else
#define I2C_STATUS_TRACK(_sr1, _sr2, loc)   (void)0
#define I2C_EV_COUNT (void)0
#endif


/// \addtogroup group_communication_i2c_impl
///  @{

#if ENABLE_SYSTICK!=0
/// \brief Timestamp when #i2c_pool_devices() was called for the last time. It is used to figure out which of virtual
///        devices should be notified with tag_DeviceContext#on_polling() callback.
volatile uint64_t g_last_usClock __attribute__ ((aligned));
#endif

/// \brief This value indicates direction of ongoing communication. Non zero indicates master (software) is reading data.
///        from #g_resp_header and corresponding virtual device buffer. Zero indicates software (master) is writing data
///        into #g_cmd_header receive buffer (#g_recv_buffer).
volatile uint8_t g_transmit = 0;	// shows current transfer direction

/// \brief This variable stores communication protocol status. (#COMM_STATUS_BUSY, #COMM_STATUS_FAIL,
///        #COMM_STATUS_CRC, #COMM_STATUS_OVF)
volatile uint8_t g_comm_status = 0;

/// \brief This variable stores status value returned by virtual device. It's value then being copied into g_comm_status upon
///        next transaction initialization.
volatile uint8_t g_returned_comm_status = 0;

/// \brief This flag is used to keep current device id.
uint8_t g_device_id = 0;

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

/// \def BUS_CMD_SYNC
/// \brief Defines synchronization command. This command is executed every time when received number of bytes is less then
///        sizeof(struct CommCommandHeader). It can be utilized to synchronize internal virtual device structures with device
///        status being send to the software.
#define BUS_CMD_SYNC  3

/// \brief Value to enable I2C peripherals via CR1
#define I2C_BUS_CR1_ENABLE              ((uint16_t)0x0001)


/// \brief Indicates type of completed operation. It's value is read periodically in #main() infinite loop to perform action
///        requested by master (software) for virtual device. It may be equal to either #BUS_CMD_NONE, #BUS_CMD_WRITE or
///        #BUS_CMD_READ
uint8_t g_cmd_type = BUS_CMD_NONE;

/// \brief Number of commands (virtual device callbacks calls) issued by I2C bus
/// \note This value is compared to g_processed_cmd_count in order to detect if all commands were processed.
volatile uint32_t g_cmd_count __attribute__ ((aligned)) = 0;

/// \brief Number of commands processed by virtual devices.
/// \note This value is compared to g_cmd_count in order to detect if all commands were processed.
volatile uint32_t g_processed_cmd_count __attribute__ ((aligned)) = 0;

/// Receive variables
/// \brief Buffer for #CommCommandHeader structure to be received from the master(software)
struct CommCommandHeader g_cmd_header;

/// \brief Pointer to the #g_cmd_header_ptr. This variable is used for clearer code to avoid multiple type conversions.
uint8_t* g_cmd_header_ptr = (uint8_t*)&g_cmd_header;

/// \brief Receive buffer that is used to receive data from master (software). This is a place where master sends data.
uint8_t g_recv_buffer[COMM_BUFFER_LENGTH];

/// \brief Total number of bytes received counter. It counts all bytes sent by master (software) to firmware except those
///        bytes which are exceed receive buffer (#g_recv_buffer) length defined by #COMM_BUFFER_LENGTH.
/// \note This value includes #g_cmd_header
uint16_t g_recv_total_pos __attribute__ ((aligned)) = 0;

/// \brief Number of bytes wrote into receive buffer (#g_recv_buffer). Later this value is passed to tag_DeviceContext#on_command() callback.
/// \note This value excludes #g_cmd_header
uint16_t g_recv_data_pos __attribute__ ((aligned)) = 0;

// Transmit variables
/// \brief Buffer for #CommResponseHeader structure to be sent to the master(software)
struct CommResponseHeader g_resp_header;


/// \brief Pointer to the #g_resp_header. This variable is used for clearer code to avoid multiple type conversions.
uint8_t* g_resp_header_ptr = (/*volatile*/ uint8_t*)&g_resp_header;

/// \brief Total number of bytes transmitted counter. It counts all bytes sent to the master, including #COMM_BAD_BYTE
/// \note This variable may be decremented when transmission is complete. It is made because firmware preload a byte into
///       I2C data register. I2C stop condition is always unexpected, therefore one extra byte is preloaded on stop condition
///       in data register. Firmware should count this byte back on stop condition.
uint16_t g_tran_total __attribute__ ((aligned)) = 0;   // Total number of bytes transmitted

/// \brief Total number of bytes transmitted to master (software) from virtual device buffer. This value is passed to
///        tag_DeviceContext#on_read_done() callback.
uint16_t g_tran_dev_pos __attribute__ ((aligned)) = 0;	// Number of bytes transmitted from device buffer

/// \brief Communication control sum accumulator. Start value is defined by #COMM_CRC_INIT_VALUE
uint8_t g_crc = COMM_CRC_INIT_VALUE; // CRC accumulator

/// \brief Value of the last byte put into (or read from) I2C data register. It's value is used for correct control sum calculations.
uint8_t g_last_byte = 0;

/// \brief Cached next byte to be transmitted. It is updated by i2c_refresh_transmit_cache()
uint8_t g_i2c_transmit_cache;

#define I2C_BUS_READ_RESPONSE_HEADER -1
#define I2C_BUS_READ_LINEAR_BUFFER   0
#define I2C_BUS_READ_CIRC_BUFFER     1
#define I2C_BUS_READ_BAD_BYTE        2

int8_t i2c_bus_finite_state_macnine;

/// \brief Stores value to increment device buffer read counter (g_tran_dev_pos)
///        in the case prefetched byte is sent into the bus.
uint16_t i2c_device_buffer_increment __attribute__ ((aligned));

/// Registered device
/// \brief Array of the device context pointers. This array is field
struct DeviceContext* g_devices[COMM_MAX_DEV_ADDR + 1];

/// \brief Pointer to the current virtual device context
struct DeviceContext* g_cur_device = 0;
/// @}

/// \brief This function initializes I2C communication peripherals
__attribute__((always_inline)) static inline
void i2c_bus_init_peripherals(void) {
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    IS_SIZE_ALIGNED(&g_cmd_count);
    IS_SIZE_ALIGNED(&g_processed_cmd_count);
    IS_SIZE_ALIGNED(&g_tran_total);
    IS_SIZE_ALIGNED(&g_tran_dev_pos);
    IS_SIZE_ALIGNED(&g_recv_data_pos);
    IS_SIZE_ALIGNED(&g_recv_total_pos);
    IS_SIZE_ALIGNED(&i2c_device_buffer_increment);

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
    I2C_Init(I2C_BUS_PERIPH, &i2c);

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

    I2C_BUS_PERIPH->CR1 |= I2C_BUS_CR1_ENABLE;
}

void i2c_bus_init(void)
{
#if ENABLE_SYSTICK!=0
    IS_ALIGNED(&g_last_usClock, sizeof(uint64_t));
#endif
    memset((void*)g_devices, 0, sizeof(g_devices));
    i2c_bus_init_peripherals();
#if ENABLE_SYSTICK!=0
    g_last_usClock = get_us_clock();
#endif
}

void comm_register_device(struct DeviceContext* dev_ctx) {
	IS_ALIGNED(dev_ctx, sizeof(uint64_t));
	IS_ALIGNED(&(dev_ctx->polling_period), sizeof(uint64_t));
	IS_ALIGNED(&(dev_ctx->next_pooling_ev), sizeof(uint64_t));

	if (dev_ctx->device_id <= COMM_MAX_DEV_ADDR) {
		g_devices[dev_ctx->device_id] = dev_ctx;

#if ENABLE_SYSTICK!=0
		uint64_t now = get_us_clock();
		if (dev_ctx->on_polling!=0 && dev_ctx->polling_period>0) {
			dev_ctx->next_pooling_ev = now + dev_ctx->polling_period;
			dev_ctx->next_pooling_ovrrun = (now >= dev_ctx->next_pooling_ev) ? 1 : 0;
		}
#endif

        // The value should be either I2C_BUS_READ_LINEAR_BUFFER (0) or I2C_BUS_READ_CIRC_BUFFER (1)
        dev_ctx->i2c_circular_buffer = (dev_ctx->circ_buffer != 0) ? I2C_BUS_READ_CIRC_BUFFER : I2C_BUS_READ_LINEAR_BUFFER;

        // From known every device must have at least 1 byte linear or circular output buffer.
        // This will help to avoid additional state to check, which will be faster and will decrease IRQ handlers
        // execution time. Therefore, devices without output buffers are forbidden!
        assert_param( (dev_ctx->circ_buffer != 0) || (dev_ctx->buffer != 0) );
	} else {
        assert_param(0);
    }
}


struct DeviceContext* comm_dev_context(uint8_t cmd_byte) {
	return g_devices[cmd_byte & COMM_MAX_DEV_ADDR];
}

// Called to check if there were command and to call device callbacks
void i2c_check_command(void) {
    uint8_t status;
    uint32_t cmd_count = g_cmd_count;
    assert_param( ( cmd_count == g_processed_cmd_count) ||
                  ( cmd_count == (g_processed_cmd_count + 1)) );

    if (cmd_count != g_processed_cmd_count) {
        switch (g_cmd_type) {
            case BUS_CMD_READ:
                status = g_cur_device->on_read_done(g_cur_device->device_id, g_tran_dev_pos);
                g_returned_comm_status = status & (~COMM_MAX_DEV_ADDR);
                g_processed_cmd_count++;
            break;

            case BUS_CMD_WRITE:
                status = g_cur_device->on_command(g_cmd_header.command_byte, g_recv_buffer, g_recv_data_pos);
                g_returned_comm_status = status & (~COMM_MAX_DEV_ADDR);
                g_processed_cmd_count++;
            break;

            case BUS_CMD_SYNC:
                status = g_cur_device->on_sync(g_cmd_header.command_byte, g_recv_total_pos);
                g_returned_comm_status = status & (~COMM_MAX_DEV_ADDR);
                g_processed_cmd_count++;
            break;
        }
    }
}

#if ENABLE_SYSTICK!=0
void i2c_pool_devices(void) {
	uint64_t now = get_us_clock();
	if (g_last_usClock >= now) {
		// overrun
		for (uint32_t i=0; i <= COMM_MAX_DEV_ADDR; i++) {
			struct DeviceContext* pdev = g_devices[i];
			if (pdev!=0) {
				g_devices[i]->next_pooling_ovrrun = 0;
			}
		}
	}

	for (uint8_t i=0; i <= COMM_MAX_DEV_ADDR; i++) {
        struct DeviceContext* pdev = g_devices[i];
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
#endif

/// \brief This function initializes I2C communication for transmit when I2C peripherals status has ADDR and TXE flags set
///        (I2C address is matched and DR address is found empty)
/// \note Firmware must write first two bytes into data register as soon as possible (second byte is a dummy byte),
///       otherwise there is a risks software will receive invalid data (device address will be received) or underrun
///       error may happen.
__attribute__((always_inline)) static inline
void i2c_transmit_init_with_send(void) {

    uint8_t prev_crc = g_crc;
    g_crc = COMM_CRC_INIT_VALUE ^ COMM_DUMMY_BYTE ^ prev_crc;

    // region I2C_TRACE #0xD2
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD2);
    // endregion

    // Transmit
    g_tran_dev_pos = 0;
    g_transmit = 1;
    g_tran_total = 2;
    g_comm_status = g_returned_comm_status;
    g_resp_header.comm_status = g_comm_status | g_device_id;

    if (g_cur_device->i2c_circular_buffer) {
        circbuf_start_read(g_cur_device->circ_buffer);
        g_resp_header.length = circbuf_total_len(g_cur_device->circ_buffer);

        if (circbuf_get_ovf(g_cur_device->circ_buffer)) {
            g_resp_header.comm_status |= COMM_STATUS_OVF;
        }
    } else {
        g_resp_header.length = g_cur_device->bytes_available;
    }

}

/// \brief This function initializes I2C communication for transmit when I2C peripherals status has just ADDR flag set
///        (I2C address is matched). Note: TXE flag is not set, in this case we shouldn't write to DR yet.
__attribute__((always_inline)) static inline
void i2c_transmit_init_no_send(void) {
    uint8_t prev_crc = g_crc;
    g_crc = COMM_CRC_INIT_VALUE;

    // Transmit
    g_tran_dev_pos = 0;
    g_transmit = 1;
    g_tran_total = 0;

    g_resp_header.last_crc = prev_crc;
    g_resp_header.dummy = COMM_DUMMY_BYTE;
    g_comm_status = g_returned_comm_status;
    g_resp_header.comm_status = g_comm_status | g_device_id;

    if (g_cur_device->i2c_circular_buffer) {
        circbuf_start_read(g_cur_device->circ_buffer);
        g_resp_header.length = circbuf_total_len(g_cur_device->circ_buffer);

        if (circbuf_get_ovf(g_cur_device->circ_buffer)) {
            g_resp_header.comm_status |= COMM_STATUS_OVF;
        }
    } else {
        g_resp_header.length = g_cur_device->bytes_available;
    }
}

/// \brief This function initializes I2C communication for receive when I2C peripherals matches address (I2C_SR1_ADDR flag).
__attribute__((always_inline)) static inline
void i2c_receive_init(void) {
    g_crc = COMM_CRC_INIT_VALUE;
    g_tran_total = 0;

    // Receive (this is new command)
    g_transmit = 0;

    I2C_STATUS_TRACK(g_crc, g_recv_total_pos, 0xC6);

    g_comm_status = g_returned_comm_status;
    if (IS_CLEARED(g_comm_status, COMM_STATUS_BUSY)) {
        g_recv_total_pos=0;
        g_recv_data_pos=0;

        I2C_STATUS_TRACK(g_comm_status, 0, 0xC7);

        // Transmit (just in case)
        g_tran_dev_pos = 0;
        g_tran_total = 0;
        memset(&g_resp_header, 0, sizeof(g_resp_header));
    } else {
        assert_param(0);
    }
}


/// \brief This function is called from I2C event interrupt handler to handle receive of new byte from master (software).
__attribute__((always_inline)) static inline
void i2c_receive_byte(void) {
	g_last_byte = I2C_BUS_PERIPH->DR;

    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xC0);

	if (IS_SET(g_comm_status, COMM_STATUS_BUSY))	{
		// Do not do anything until BUSY flag is not cleared by device
        I2C_STATUS_TRACK(0, 0, 0xC1);
	} else
	if (g_recv_total_pos < sizeof(struct CommCommandHeader)) {
		// receive command header

		g_cmd_header_ptr[g_recv_total_pos++] = g_last_byte;

        I2C_STATUS_TRACK(g_crc, g_recv_total_pos, 0xC5);
		if (g_recv_total_pos == COMM_COMMAND_BYTE_OFFSET + 1) {
			// Command byte received, find device

			uint8_t dev_id = (g_last_byte & COMM_MAX_DEV_ADDR);
			g_cur_device = g_devices[dev_id];
            g_device_id = dev_id;
            assert_param(IS_CLEARED(g_comm_status, COMM_STATUS_BUSY));
			// g_comm_status = 0;
            I2C_STATUS_TRACK(0, dev_id, 0xC2);
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
        I2C_STATUS_TRACK(g_crc, g_recv_total_pos, 0xC3);
	} else {
		SET_FLAGS(g_comm_status, COMM_STATUS_FAIL);
        I2C_STATUS_TRACK(g_comm_status | g_device_id, 0, 0xC4);
	}

    // region I2C_TRACE #0xD3
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD3);
    // endregion
}

/// \brief This function is called from I2C event interrupt handler to send a byte to master (software).
__attribute__((always_inline)) static inline
void i2c_transmit_byte(void) {
    I2C_BUS_PERIPH->DR = g_i2c_transmit_cache;
    g_last_byte = g_i2c_transmit_cache;
    g_tran_total++;
    g_tran_dev_pos += i2c_device_buffer_increment;
    g_crc = g_crc ^ g_last_byte;

#if ISR_EV_DEBUG_TRANSMIT
    // region I2C_TRACE #0xD4
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD4);
    // endregion
#endif
}

__attribute__((always_inline)) static inline
void i2c_init_transmit_cache(void) {
    assert_param(g_tran_total < sizeof(struct CommResponseHeader));
    g_i2c_transmit_cache = g_resp_header_ptr[g_tran_total];
    i2c_bus_finite_state_macnine = I2C_BUS_READ_RESPONSE_HEADER;
    i2c_device_buffer_increment = 0;
}

// Must be called AFTER byte is transmitted
__attribute__((always_inline)) static inline
void i2c_update_transmit_cache(void) {
    switch (i2c_bus_finite_state_macnine) {
        case I2C_BUS_READ_RESPONSE_HEADER:
            g_i2c_transmit_cache = g_resp_header_ptr[g_tran_total];
            i2c_device_buffer_increment = 0;

            i2c_bus_finite_state_macnine = ( g_tran_total < ( sizeof(struct CommResponseHeader) - 1) ) ?
                            I2C_BUS_READ_RESPONSE_HEADER:
                            g_cur_device->i2c_circular_buffer;


            if (g_tran_total == (sizeof(struct CommResponseHeader) - 1)) {
                assert_param(i2c_bus_finite_state_macnine == I2C_BUS_READ_LINEAR_BUFFER ||
                             i2c_bus_finite_state_macnine == I2C_BUS_READ_CIRC_BUFFER);
            }

            break;

        case I2C_BUS_READ_CIRC_BUFFER:
            circbuf_get_byte(g_cur_device->circ_buffer, &g_i2c_transmit_cache);
            i2c_device_buffer_increment = 1;

            if (g_tran_dev_pos+1>=g_resp_header.length) {
                i2c_bus_finite_state_macnine = I2C_BUS_READ_BAD_BYTE;
            }
            break;

        case I2C_BUS_READ_LINEAR_BUFFER:
            g_i2c_transmit_cache = g_cur_device->buffer[g_tran_dev_pos];
            assert_param(g_i2c_transmit_cache!=0);
            i2c_device_buffer_increment = 1;

            if (g_tran_dev_pos+1>=g_resp_header.length) {
                i2c_bus_finite_state_macnine = I2C_BUS_READ_BAD_BYTE;
            }
            break;

        case I2C_BUS_READ_BAD_BYTE:
            g_i2c_transmit_cache = COMM_BAD_BYTE;
            i2c_device_buffer_increment = 0;
            break;

        default:
            assert_param(0);
    };
}


/// \brief This function is called from I2C event and error interrupt handlers to stop I2C bus communication with master (software)
__attribute__((always_inline)) static inline
void i2c_stop(void) {
	if (g_transmit==1 && g_tran_total>1) {
		// special case : if series of bytes are transmitted one (the last) byte is always discarded because it is stored in data register, while NACK
		// is returned when shift register becomes empty. In the same time DR is not empty, therefore we have to reverse this byte back

		// decrease counters
		if (g_tran_dev_pos>0 && g_tran_total-g_tran_dev_pos==sizeof(struct CommResponseHeader)) {
			// MCU was required to sent fake buffers because no more data was in buffer
			g_tran_dev_pos--;
		}
		g_tran_total--;

		// correct CRC - just XOR one more time last byte sent
		g_crc = g_crc ^ g_last_byte;
	}

	if (g_transmit==1 && IS_CLEARED(g_resp_header.comm_status, COMM_STATUS_BUSY) && g_tran_dev_pos > 0) {
        if (	g_cur_device->on_read_done!=0 &&
                IS_CLEARED(g_comm_status, COMM_STATUS_FAIL | COMM_STATUS_CRC) &&
                g_tran_dev_pos>0) {
            assert_param(g_cmd_count == g_processed_cmd_count);
            SET_FLAGS(g_comm_status, COMM_STATUS_BUSY);
            g_cmd_type = BUS_CMD_READ;
            g_cmd_count++;
        }
	} else if (g_transmit==0 &&
                IS_CLEARED(g_comm_status, COMM_STATUS_BUSY)) {
        if (g_recv_total_pos >= sizeof(struct CommCommandHeader)) {
            if(g_cmd_header.length!=g_recv_data_pos) {
                SET_FLAGS(g_comm_status, COMM_STATUS_FAIL);
            }

            if (g_cmd_header.control_crc!=g_crc) {
                SET_FLAGS(g_comm_status, COMM_STATUS_CRC);
            }

            if (g_cur_device->on_command!=0 &&
                IS_CLEARED(g_comm_status, COMM_STATUS_FAIL | COMM_STATUS_CRC)) {
                assert_param(g_cmd_count == g_processed_cmd_count);
                SET_FLAGS(g_comm_status, COMM_STATUS_BUSY);
                g_cmd_type = BUS_CMD_WRITE;
                g_cmd_count++;
            }
        } else if (g_cur_device->on_sync!=0) {
            assert_param(g_cmd_count == g_processed_cmd_count);
            SET_FLAGS(g_comm_status, COMM_STATUS_BUSY);
            g_cmd_type = BUS_CMD_SYNC;
            g_cmd_count++;
        }
	}

    // Make sure g_transmit is initialized the same way every time communication starts (when ADDR is received).
    // It will allow reliably distinguish between transmit and receive. It is related to the way how EV ISR detects
    // operation mode
    g_transmit = 0;
    g_recv_total_pos = 0;

    // region I2C_TRACE #0xD5
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD5);
    // endregion
}

/// \def READ_FLAGS
/// \brief Defines sequence to read status registers without changing to actual registers
#define READ_FLAGS              sr2 = I2C_BUS_PERIPH->SR2; \
                                sr1 = I2C_BUS_PERIPH->SR1;

/// \def READ_FLAGS_CLEAR_ADDR
/// \brief Defines sequence to read status registers and clear ADDR flag
#define READ_FLAGS_CLEAR_ADDR   sr1 = I2C_BUS_PERIPH->SR1; \
                                sr2 = I2C_BUS_PERIPH->SR2;

/// \def READ_FLAGS_CLEAR_STOPF
/// \brief Defines sequence to read status registers and clear ADDR flag
#define READ_FLAGS_CLEAR_STOPF  sr1 = I2C_BUS_PERIPH->SR1; \
                                I2C_BUS_PERIPH->CR1 |= I2C_BUS_CR1_ENABLE; \
                                sr1 = I2C_BUS_PERIPH->SR1;



//#pragma GCC push_options
//#pragma GCC optimize ("O2")

MAKE_ISR(I2C_BUS_EV_ISR) {

    uint16_t sr1;
    uint16_t sr2;

    if (IS_SET(I2C_BUS_PERIPH->SR1, I2C_SR1_ADDR | I2C_SR1_TXE) /* && IS_SET(sr2, I2C_SR2_BUSY) */) {
//------------------------------ ADDR TXE -------------------------------------------------
        // SPECIAL CASE: We have to write first byte of the CommResponseHeader structure here ASAP. The first byte will be CRC sum for the previous operation
        I2C_BUS_PERIPH->DR = g_crc;



#if ISR_EV_DEBUG_TRANSMIT
        // region I2C_TRACE #0x00, #0xD0
        I2C_STATUS_TRACK(sr1, sr2, 0);
        I2C_STATUS_TRACK(COMM_CRC_INIT_VALUE, g_crc, 0xD0);
        // endregion
#endif

        do {
            I2C_BUS_PERIPH->DR = COMM_DUMMY_BYTE;
            READ_FLAGS_CLEAR_ADDR;
        } while (IS_SET(sr1, I2C_SR1_TXE));

#if ISR_EV_DEBUG_TRANSMIT
// region I2C_TRACE #0xD1
        I2C_STATUS_TRACK(COMM_CRC_INIT_VALUE, COMM_DUMMY_BYTE, 0xD1);
// endregion
#endif
        g_last_byte = COMM_DUMMY_BYTE;
        i2c_transmit_init_with_send();
        i2c_init_transmit_cache();
    } else if (IS_SET(I2C_BUS_PERIPH->SR1, I2C_SR1_ADDR)) {
//------------------------------ ADDR NO TXE -------------------------------------------------
        READ_FLAGS_CLEAR_ADDR;
        if (IS_SET(sr2, I2C_SR2_TRA)) {
#if ISR_EV_DEBUG_TRANSMIT
// region I2C_TRACE #0x01
            I2C_STATUS_TRACK(sr1, sr2, 1);
// endregion
#endif
            i2c_transmit_init_no_send();
            i2c_init_transmit_cache();
        } else {
#if ISR_EV_DEBUG_TRANSMIT
// region I2C_TRACE #0x02
            I2C_STATUS_TRACK(sr1, sr2, 2);
// endregion
#endif
            i2c_receive_init();
        }
    } else {
//------------------------------ OTHER -------------------------------------------------

        READ_FLAGS;
        // region I2C_TRACE #0x03
        I2C_STATUS_TRACK(sr1, sr2, 3);
        // endregion


    }

    // NOTE: This sequence clears ADDR and updates TXE/RXE flags. It should be done here
    sr1 = I2C_BUS_PERIPH->SR1;
    sr2 = I2C_BUS_PERIPH->SR2;
// region I2C_TRACE #0x04
    I2C_STATUS_TRACK(sr1, sr2, 4);
// endregion

    // --------------------- TRANSMITING DATA ---------------------
    while (IS_SET(sr1, I2C_SR1_TXE)) {
        i2c_transmit_byte();
        i2c_update_transmit_cache();
        READ_FLAGS;
    }

    // --------------------- RECEIVING DATA ---------------------
    if (IS_SET(sr1, I2C_SR1_RXNE)) {
        i2c_receive_byte();
    }

    // --------------------- STOP ---------------------
    if (IS_SET(sr1, I2C_SR1_STOPF))
    {
        i2c_stop();
        READ_FLAGS_CLEAR_STOPF;
// region I2C_TRACE #0xFE
        I2C_STATUS_TRACK(0xFFFF, 0xFFFF, 0xFE);
// endregion
    }

// region I2C_TRACE #5
#if USE_I2C_TRACKING
    READ_FLAGS;
#endif
    I2C_STATUS_TRACK(sr1, sr2, 5);
    I2C_EV_COUNT;
// endregion

    UNUSED(sr1);
    UNUSED(sr2);
}

//#pragma GCC pop_options

MAKE_ISR(I2C_BUS_ER_ISR) {
    uint16_t sr1,sr2;

    READ_FLAGS;
// region I2C_TRACE #0xF0
    I2C_STATUS_TRACK(sr1, sr2, 0xF0);
// endregion

    if (sr1 & (I2C_SR1_OVR | I2C_SR1_AF)) {
        // One of the following has happened:
        // 1. Overflow on receive.
        // 2. Underrun on transmit.
        // 3. Acknowledge has failed.
        i2c_stop();
        CLEAR_FLAGS(I2C_BUS_PERIPH->SR1, (uint16_t)(I2C_SR1_OVR | I2C_SR1_AF));

        READ_FLAGS;
// region I2C_TRACE #0xF1, #0xFF
        I2C_STATUS_TRACK(sr1, sr2, 0xF1);
        I2C_STATUS_TRACK(0xFFFF, 0xFFFF, 0xFF);
// endregion
    } else {
        /* TEST CODE */
        assert_param(0);
    }

    if (sr1 & (I2C_SR1_SB | I2C_SR1_ADDR | I2C_SR1_ADD10 | I2C_SR1_RXNE | I2C_SR1_TXE | I2C_SR1_BERR | I2C_SR1_ARLO |
               I2C_SR1_PECERR | I2C_SR1_TIMEOUT | I2C_SR1_SMBALERT)) {
        assert_param(0);
    }

// region I2C_TRACE EV++
    I2C_EV_COUNT;
// endregion

    UNUSED(sr1);
    UNUSED(sr2);

    return;
}
