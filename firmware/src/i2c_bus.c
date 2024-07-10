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

/// \brief I2C bus may work with circula buffers from different devices. Circular buffers require synchronization with device,
///        which is guaranteed by sequential locks. The following macro specify how i2c bus reader will enter/leave into critical
///        section for the sequential lock.
#define SEQ_LOCK_I2C_READER 1

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "sys_tick_counter.h"


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

/// \brief Value to enable I2C peripherals via CR1
#define I2C_BUS_CR1_ENABLE              ((uint16_t)0x0001)

#define I2C_BUS_CR1_SW_RESET_SET        ((uint16_t)0x8000)
#define I2C_BUS_CR1_SW_RESET_CLEAR      ((uint16_t)0x7FFF)


/// \brief Indicates type of completed operation. It's value is read periodically in #main() infinite loop to perform action
///        requested by master (software) for virtual device. It may be equal to either #BUS_CMD_NONE, #BUS_CMD_WRITE or
///        #BUS_CMD_READ
volatile uint8_t g_cmd_type = BUS_CMD_NONE;

// Receive variables
/// \brief Buffer for #CommCommandHeader structure to be received from the master(software)
struct CommCommandHeader g_cmd_header;

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
struct CommResponseHeader g_resp_header;

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

/// \brief These variables are used to quickly restore i2c state after software reset.
volatile uint16_t g_i2c_cr1, g_i2c_cr2, g_i2c_oar1, g_i2c_oar2, g_i2c_ccr, g_i2c_trise;

// Registered device
/// \brief Array of the device context pointers. This array is field
volatile PDeviceContext g_devices[COMM_MAX_DEV_ADDR + 1];

/// \brief Pointer to the current virtual device context
PDeviceContext g_cur_device = 0;
/// @}

static inline void i2c_call_command_callback(void);
static inline void i2c_call_readdata_callback(void);

/// \brief This function initializes I2C communication peripherals
__attribute__((always_inline)) static inline
void i2c_bus_init_peripherals(void) {
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

    // Backup registers
    g_i2c_cr1 = I2C_BUS_PERIPH->CR1 | I2C_BUS_CR1_ENABLE;
    g_i2c_cr2 = I2C_BUS_PERIPH->CR2;
    g_i2c_ccr = I2C_BUS_PERIPH->CCR;
    g_i2c_oar1 = I2C_BUS_PERIPH->OAR1;
    g_i2c_oar2 = I2C_BUS_PERIPH->OAR2;
    g_i2c_trise = I2C_BUS_PERIPH->TRISE;

    I2C_BUS_PERIPH->CR1 |= I2C_BUS_CR1_ENABLE;
}

/// \brief This function reset I2C communication peripherals in order to prepare communication for the next
///        receive/transmit from the software side
/// \note When this function is being executed I2C is not available and software may get failure in attempt to read/write
///       from I2C device. Thus, software should be prepared and treat such failures as #COMM_STATUS_BUSY
/// \note This function is called in the following situations:
///       - stop condition is received but command was not dispatched to corresponding virtual device (#g_cmd_type is #BUS_CMD_NONE)
///       - after virtual device tag_DeviceContext#on_command() is executed (#g_cmd_type is #BUS_CMD_WRITE)
///       - after virtual device tag_DeviceContext#on_read_done() is executed (#g_cmd_type is #BUS_CMD_READ)
void i2c_bus_reset(void) {
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


__attribute__((always_inline)) static inline
void i2c_bus_fast_reset(void) {
    I2C_BUS_PERIPH->CR1 |= I2C_BUS_CR1_SW_RESET_SET;
    I2C_BUS_PERIPH->CR1 &= I2C_BUS_CR1_SW_RESET_CLEAR;

    I2C_BUS_PERIPH->CCR = g_i2c_ccr;
    I2C_BUS_PERIPH->OAR1 = g_i2c_oar1;
    I2C_BUS_PERIPH->OAR2 = g_i2c_oar2;
    I2C_BUS_PERIPH->TRISE = g_i2c_trise;
    I2C_BUS_PERIPH->CR2 = g_i2c_cr2;
    I2C_BUS_PERIPH->CR1 = g_i2c_cr1;
}

void i2c_bus_init(void)
{
    //enable_debug_pins();


#if ENABLE_SYSTICK!=0
    IS_ALIGNED(&g_last_usClock, sizeof(uint64_t));
#endif
    memset((void*)g_devices, 0, sizeof(g_devices));
    i2c_bus_init_peripherals();
#if ENABLE_SYSTICK!=0
    g_last_usClock = get_us_clock();
#endif
}

void comm_register_device(PDeviceContext dev_ctx) {
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
	}
}


PDeviceContext comm_dev_context(uint8_t cmd_byte) {
	return g_devices[cmd_byte & COMM_MAX_DEV_ADDR];
}

/// \brief This function allows to write some value that belongs to I2C communication implementation from #main() infinite
///        loop context synchronously.
/// \param v - pointer to value to be set.
/// \param nv - new value to be set
/// \details This function is required in order to set #g_cmd_type safely from #main() infinite loop. This variable shouldn't
///          be set if I2C communication is ongoing. Ongoing communication is specified by #g_i2c_busy variable, thus
///          this function check it in order to write safely. Also this function disables interrupts for synchronous operation.
/// \warning This function disables interrupts, thus it's not allowed to call this function when interrupts are disabled.
///          If the restriction is violated, this function asserts in debug builds.
__attribute__((always_inline)) static inline
void i2c_set_sync(volatile uint8_t* v, uint8_t nv) {
    ASSERT_IRQ_ENABLED;
	uint8_t done = 0;
	do {
        while (g_i2c_busy) {
            /* just wait */
        }
        __disable_irq();
		if (g_i2c_busy==0) {
			*v = nv;
            __enable_irq();
			done = 1;
		} else {
            __enable_irq();
        }

	} while(done==0);
}

void comm_done(uint8_t status) {
    ASSERT_IRQ_ENABLED;
	uint8_t done = 0;
    status = status & (~COMM_MAX_DEV_ADDR);

	do {
        while (g_i2c_busy) {
            /*nope*/
        }
        __disable_irq();
		if (g_i2c_busy==0) {
			g_comm_status = (status) | (g_comm_status & COMM_MAX_DEV_ADDR);
            __enable_irq();
			done = 1;
		} else {
            __enable_irq();
        }

	} while(done==0);
}

static inline void i2c_call_command_callback(void) {
			g_cur_device->on_command(g_cmd_header.command_byte, g_recv_buffer, g_recv_data_pos);
			i2c_bus_reset();	// Re-enable I2C
}

static inline void i2c_call_readdata_callback(void) {
			g_cur_device->on_read_done(g_cur_device->device_id, g_tran_dev_pos);
			i2c_bus_reset();	// Re-enable I2C
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

#if ENABLE_SYSTICK!=0
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

    g_i2c_busy = 1; // Lock changes during communication
    g_cmd_type = BUS_CMD_NONE;

    // Transmit
    g_tran_dev_pos = 0;
    g_transmit = 1;
    g_tran_total = 2;

    g_resp_header.last_crc = prev_crc;
    g_resp_header.dummy = COMM_DUMMY_BYTE;
    g_resp_header.comm_status = g_comm_status;
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
}

/// \brief This function initializes I2C communication for transmit when I2C peripherals status has just ADDR flag set
///        (I2C address is matched). Note: TXE flag is not set, in this case we shouldn't write to DR yet.
__attribute__((always_inline)) static inline
void i2c_transmit_init_no_send(void) {
    uint8_t prev_crc = g_crc;
    g_crc = COMM_CRC_INIT_VALUE;

    g_i2c_busy = 1; // Lock changes during communication
    g_cmd_type = BUS_CMD_NONE;

    // Transmit
    g_tran_dev_pos = 0;
    g_transmit = 1;
    g_tran_total = 0;

    g_resp_header.last_crc = prev_crc;
    g_resp_header.dummy = COMM_DUMMY_BYTE;
    g_resp_header.comm_status = g_comm_status;
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
}

/// \brief This function initializes I2C communication for receive when I2C peripherals matches address (I2C_SR1_ADDR flag).
__attribute__((always_inline)) static inline
void i2c_receive_init(void) {
    g_crc = COMM_CRC_INIT_VALUE;

    g_i2c_busy = 1; // Lock changes during communication
    g_cmd_type = BUS_CMD_NONE;
    g_tran_total = 0;

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


/// \brief This function is called from I2C event interrupt handler to handle receive of new byte from master (software).
__attribute__((always_inline)) static inline
void i2c_receive_byte(void) {
	g_last_byte = I2C_BUS_PERIPH->DR;

	if (IS_SET(g_comm_status, COMM_STATUS_BUSY))	{
		// Do not do anything until BUSY flag is not cleared by device
	} else
	if (g_recv_total_pos < sizeof(struct CommCommandHeader)) {
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

    // region I2C_TRACE #0xD3
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD3);
    // endregion
}

/// \brief This function is called from I2C event interrupt handler to send a byte to master (software).
__attribute__((always_inline)) static inline
void i2c_transmit_byte(void) {
	if (g_tran_total<sizeof(struct CommResponseHeader)) {
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

    // region I2C_TRACE #0xD4
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD4);
    // endregion
}

/// \brief This function is called from I2C event and error interrupt handlers to stop I2C bus communication with master (software)
__attribute__((always_inline)) static inline
void i2c_stop(void) {
    set_debug_pin_2();
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
            g_cmd_type = BUS_CMD_READ;
        }

	} else if (g_transmit==0 && IS_CLEARED(g_comm_status, COMM_STATUS_BUSY) && g_recv_total_pos >= sizeof(struct CommCommandHeader)) {
        if(g_cmd_header.length!=g_recv_data_pos) {
            SET_FLAGS(g_comm_status, COMM_STATUS_FAIL);
        }

        if (g_cmd_header.control_crc!=g_crc) {
            SET_FLAGS(g_comm_status, COMM_STATUS_CRC);
        }

        if (g_cur_device->on_command!=0 &&
            IS_CLEARED(g_comm_status, COMM_STATUS_FAIL | COMM_STATUS_CRC)) {
            SET_FLAGS(g_comm_status, COMM_STATUS_BUSY);
            g_cmd_type = BUS_CMD_WRITE;
        }
	}

    // Make sure g_transmit is initialized the same way every time communication starts (when ADDR is received).
    // It will allow reliably distinguish between transmit and receive. It is related to the way how EV ISR detects
    // operation mode
    g_transmit = 0;

	g_i2c_busy = 0;

	if (g_cmd_type == BUS_CMD_NONE) {
		// No callback will be called, enable i2c here
		i2c_bus_reset();
	}

    // region I2C_TRACE #0xD5
    I2C_STATUS_TRACK(g_crc, g_last_byte, 0xD5);
    // endregion

    clear_debug_pin_2();
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

MAKE_ISR(I2C_BUS_EV_ISR) {

    volatile uint16_t sr1;
    volatile uint16_t sr2;

    sr2 = I2C_BUS_PERIPH->SR2;
    sr1 = I2C_BUS_PERIPH->SR1;

    set_debug_pin_0();

    if (IS_SET(sr1, I2C_SR1_ADDR | I2C_SR1_TXE) && IS_SET(sr2, I2C_SR2_BUSY)) {
        // SPECIAL CASE: We have to write first byte of the CommResponseHeader structure here ASAP. The first byte will be CRC sum for the previous operation
        I2C_BUS_PERIPH->DR = g_crc;
// region I2C_TRACE #0x00, #0xD0
        I2C_STATUS_TRACK(sr1, sr2, 0);
        I2C_STATUS_TRACK(COMM_CRC_INIT_VALUE, g_crc, 0xD0);
// endregion
        READ_FLAGS_CLEAR_ADDR
        I2C_BUS_PERIPH->DR = COMM_DUMMY_BYTE;
// region I2C_TRACE #0xD1
        I2C_STATUS_TRACK(COMM_CRC_INIT_VALUE, COMM_DUMMY_BYTE, 0xD1);
// endregion
        g_last_byte = COMM_DUMMY_BYTE;
        i2c_transmit_init_with_send();
    } else
    if (IS_SET(sr1, I2C_SR1_ADDR) && IS_SET(sr2, I2C_SR2_BUSY)) {
        if (IS_SET(sr2, I2C_SR2_TRA)) {
// region I2C_TRACE #0x01
            I2C_STATUS_TRACK(sr1, sr2, 1);
// endregion
            i2c_transmit_init_no_send();
        } else {
// region I2C_TRACE #0x02
            I2C_STATUS_TRACK(sr1, sr2, 2);
// endregion
            i2c_receive_init();
        }
    } else {
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

    // --------------------- RECEIVING DATA ---------------------
    if (IS_SET(sr1, I2C_SR1_RXNE) && IS_SET(sr2, I2C_SR2_BUSY)) {
        i2c_receive_byte();
    }

    // --------------------- TRANSMITING DATA ---------------------
    if (IS_SET(sr1, I2C_SR1_TXE) && IS_SET(sr2, I2C_SR2_BUSY)) {
        i2c_transmit_byte();
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
    READ_FLAGS;
    I2C_STATUS_TRACK(sr1, sr2, 5);
    I2C_EV_COUNT;
    clear_debug_pin_0();
// endregion

    UNUSED(sr1);
    UNUSED(sr2);
}

MAKE_ISR(I2C_BUS_ER_ISR) {
    uint16_t sr1,sr2;

    set_debug_pin_1();


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
    }

    if (sr1 & (I2C_SR1_SB | I2C_SR1_ADDR | I2C_SR1_ADD10 | I2C_SR1_RXNE | I2C_SR1_TXE | I2C_SR1_BERR | I2C_SR1_ARLO |
               I2C_SR1_PECERR | I2C_SR1_TIMEOUT | I2C_SR1_SMBALERT)) {
        assert_param(0);
    }

// region I2C_TRACE EV++
    I2C_EV_COUNT;
// endregion

    clear_debug_pin_1();

    UNUSED(sr1);
    UNUSED(sr2);

    return;
}
