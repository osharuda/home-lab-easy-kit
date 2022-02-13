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
 *   \brief Circular Buffer header.
 *   \author Oleh Sharuda
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup group_circ_buffer Circular buffer
/// \brief Circular buffer implementation
/// @{
/// \page page_circ_buffer
/// \tableofcontents
///
/// Circular buffer may be used for some features where data is read continuously from device. In this case simple linear
/// buffer is not sufficient. Here, and after "circular buffer" will refer to this project implementation of circular buffer.
/// Below usage of circular buffer is described. It is important to note that writing and reading data may be concurrent
/// (or simultaneous).
///
/// \section sect_circ_buffer_01 Initializing circular buffer.
///
/// In order to initialize circular buffer this procedure must be followed:
/// 1. Allocate memory to be used as storage for data.
/// 2. Allocate memory for #tag_CircBuffer to be used as circular buffer control structure.
/// 3. Call #circbuf_init().
/// 4. Optionally, if block mode is required, #circbuf_init_block_mode() should be called. It's important to note that
///    circular buffer size should be multiple to the block size, otherwise noncontinuous block appear in circular buffer.
///
/// \section sect_circ_buffer_02 Writing data into circular buffer.
///
/// Writes to the circular buffer may be done in two modes: byte mode and block mode.
///
/// Byte mode allows to put single byte into the buffer. It is used for simple devices (like UARTDev) that return data by
/// bytes. Usually these devices work in pool or interrupt modes. Use the following function to write data in byte mode:
/// #circbuf_put_byte(). Here and after this function is referred as "byte mode write function". Do not use block mode
/// write functions in byte mode.
///
/// Block mode is required for devices that work in DMA mode. Use the following functions to write data in block mode:
/// #circbuf_reserve_block(), #circbuf_commit_block(), #circbuf_cancel_block(). Here and after these three functions are
/// referred as "block mode write functions". Do not use byte mode write function in block mode.To write data in block
/// mode this procedure must be followed:
/// 1. Reserve memory block in circular buffer. Call #circbuf_reserve_block() to reserve a block.
/// 2. Establish DMA operation as required by device.
/// 3. When DMA operation is complete successfully call #circbuf_commit_block() to fix it in circular buffer.
/// 4. If DMA operation failed, canceled or data read is non-valid #circbuf_cancel_block() must be called in order to
///    cancel block reservation.
///
/// Note, it is not possible to reserve more than one block at the moment.
///
/// \section sect_circ_buffer_03 Reading data from circular buffer.
///
/// Reading data from circular buffer works in the same way in byte and block write mode. To read a data this procedure
/// must be followed:
/// 1. Call #circbuf_start_read(). This call will prepare circular buffer for reading.
/// 2. Call #circbuf_get_byte() in cycle to read as much data as you need. You may check result of this function to understand
///    if valid data was actually read.
/// 3. If read completed, and caller decided it is ok, #circbuf_stop_read() must be called. This function will fix read
///    operation in circular buffer.
///
/// Broken reads. Sometimes it is required to unread data from the circular buffer. In this case #circbuf_stop_read()
/// must NOT be called, thus read operation will not be fixed in circular buffer, and next call to the #circbuf_start_read()
/// will setup read from the very same place where this read had started.
///
/// Also this behavior allows to make some optimization on #circbuf_get_byte() result check. It is possible to read as
/// much bytes as required in cycle and do "AND" operation on #circbuf_get_byte() result. This common result may be
/// analyzed after reading loop, thus whole loop will take less cycles because there will be just one comparison.
///

/// \struct tag_CircBuffer
/// \brief Structure that represents circular buffer.
typedef struct tag_CircBuffer {

    volatile uint8_t *buffer;       ///< Buffer to be used as data storage for circular buffer.

    volatile uint8_t *status;       ///< Optional status buffer to be prepended to the data to be sent to the software.

    volatile uint8_t *current_block; ///< Pointer to the currently reserved block (for block mode only).

    volatile uint16_t buffer_size;  ///< size of the #buffer, in bytes.

    volatile uint16_t status_size;  ///< Size of the #status_buffer;

    volatile uint16_t put_pos;      ///< Position, where next byte or block will be placed

    volatile uint16_t start_pos;    ///< Start position of actual (or valid) data in the buffer

    volatile uint16_t data_len;     ///< Total amount of actual (or valid) data in the buffer

    volatile uint16_t read_pos;     ///< Current read position (since the last circbuf_start_read() call)

    volatile uint16_t bytes_read;   ///< Amount of bytes read (since the last circbuf_start_read() call)

    volatile uint16_t free_size;    ///< Precalculated value that can be used to check if there is free space in buffer
                                    ///< (free_size>=data_len)

    volatile uint16_t block_size;   ///< Circular buffer block size. If block size is more than 1 circular buffer works
                                    ///< in block mode. When block mode is enabled block mode functions only should be
                                    ///< used to put data. In this case circbuf_put_byte() must not be used. When block
                                    ///< mode is disabled (byte mode) circbuf_put_byte() must be used; block mode functions
                                    ///< must not be used in byte mode.

    volatile uint8_t ovf;           ///< indicates overflow, cleared by circbuf_clear_ovf()
} CircBuffer, *PCircBuffer;

/// \brief Initializes circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param buffer - pointer to the memory block used by circular buffer
/// \param length - length of the memory block
/// \note This function initializes circular buffer in byte mode.
void circbuf_init(volatile PCircBuffer circ, uint8_t *buffer, uint16_t length);

/// \brief Initializes status data for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param status - pointer to the status data memory block.
/// \param length - length of the status data memory block.
/// \note Status data is sent first before any data from actual circular buffer. Status data allows to report some
///       data like extended device data or error codes to the software. It is somewhat allows to use usual buffer and
///       circular buffer as one object.
void circbuf_init_status(volatile PCircBuffer circ, volatile uint8_t* status, uint16_t length);


/// \brief Resets content of the buffer
/// \param circ - pointer to the circular buffer structure
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_reset(volatile PCircBuffer circ);

/// \brief Resets content of the buffer (doesn't disable IRQ internally).
/// \param circ - pointer to the circular buffer structure
/// \warning This function doesn't disable interrupts with #DISABLE_IRQ macro. Synchronization is responsibility of the caller.
void circbuf_reset_no_irq(volatile PCircBuffer circ);

/// \brief Returns amount of data stored in circular buffer (not including status data memory block)
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer, in bytes.
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
uint16_t circbuf_len(volatile PCircBuffer circ);

/// \brief Returns amount of data stored in circular buffer including status data memory block.
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer and in status data memory block, in bytes.
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
uint16_t circbuf_total_len(volatile PCircBuffer circ);

/// \brief Returns amount of data stored in circular buffer including status data memory block  (doesn't disable IRQ internally).
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer and in status data memory block, in bytes.
/// \warning This function doesn't disable interrupts with #DISABLE_IRQ macro. Synchronization is responsibility of the caller.
uint16_t circbuf_total_len_no_irq(volatile PCircBuffer circ);

/// \brief Put a byte into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \param b - byte to be put into circular buffer
/// \warning This function must be used for circular buffer working in byte mode only (block mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_put_byte(volatile PCircBuffer circ, uint8_t b);

/// \brief Initializes read operation from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_start_read(volatile PCircBuffer circ);

/// \brief Reads a byte from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param b - pointer to a byte to be written by read value. If buffer is empty #COMM_BAD_BYTE is returned.
/// \return non-zero if successfull, 0 if circular buffer is empty.
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
uint8_t circbuf_get_byte(volatile PCircBuffer circ, volatile uint8_t *b);

/// \brief Returns buffer overflow flag.
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if overflow flag is set, zero if overflow flag is cleared.
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
uint8_t circbuf_get_ovf(volatile PCircBuffer circ);

/// \brief Clears overflow flag.
/// \param circ - pointer to the circular buffer structure
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_clear_ovf(volatile PCircBuffer circ);

/// \brief Stop read operation from circular buffer. This will "commit" changes caused by read operations into circular
///        buffer structure.
/// \param circ - pointer to the circular buffer structure
/// \param num_bytes
/// \return
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
uint16_t circbuf_stop_read(volatile PCircBuffer circ, uint16_t num_bytes);

/// \brief Initializes block mode for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param bs - size of the block. Circular buffer size must be multiple to this value.
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_init_block_mode(volatile PCircBuffer circ, uint16_t bs);

/// \brief Reserves block from circular buffer
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if success, if circular buffer may not allocate block, returns zero
/// \warning It is not possible to reserve more than one block at the moment.
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void *circbuf_reserve_block(volatile PCircBuffer circ);

/// \brief Commits block into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_commit_block(volatile PCircBuffer circ);

/// \brief Cancel reserved memory block
/// \param circ - pointer to the circular buffer structure
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void circbuf_cancel_block(volatile PCircBuffer circ);

/// @}

#ifdef __cplusplus
}
#endif