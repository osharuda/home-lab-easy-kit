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
#include <utools.h>
#include <i2c_proto.h>
#include <seqlock.h>

#ifdef DISABLE_NOT_TESTABLE_CODE
// This is used for test purposes only
extern int g_assert_param_count;
#else
#include "stm32f10x_conf.h"
#endif

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
/// \note All members of the structure must be aligned by their size. This is a requirement to have atomic reads and writes
///       to structure members.
///       For futher information read ARMv7-M :
///       In ARMv7-M, the single-copy atomic processor accesses are:
///       All byte accesses.
///       All halfword accesses to halfword-aligned locations.
///       All word accesses to word-aligned locations.
///

struct CircBuffer {

    /* LOCK */  struct sequential_lock lock;

    /* CONST */ volatile uint8_t *buffer;       ///< Buffer to be used as data storage for circular buffer.

    /* CONST */ volatile uint8_t *status;       ///< Optional status buffer to be prepended to the data to be sent to the software.

    /* DEBUG */ volatile uint8_t *current_block; ///< Pointer to the currently reserved block (for block mode only).

    /* CONST */ volatile uint16_t buffer_size;  ///< size of the #buffer, in bytes.

    /* CONST */ volatile uint16_t status_size;  ///< Size of the #status_buffer;

    /* STATE */ volatile uint16_t put_pos;      ///< Position, where next byte or block will be placed

    /* STATE */ volatile uint16_t data_len;     ///< Total amount of actual (or valid) data in the buffer

    /* READ CACHE */ volatile uint16_t read_pos;     ///< Current read position (since the last circbuf_start_read() call)

    /* READ CACHE */ volatile uint16_t bytes_read;   ///< Amount of bytes read (since the last circbuf_start_read() call)

    /* CONST */ volatile uint16_t free_size;    ///< Precalculated value that can be used to check if there is free space in buffer
                                    ///< (free_size>=data_len)

    /* CONST */ volatile uint16_t warn_low_thr;///< Low warning check threshold

    /* CONST */ volatile uint16_t warn_high_thr; ///< High warning check threshold

    /* CONST */ volatile uint16_t block_size;   ///< Circular buffer block size. If block size is more than 1 circular buffer works
                                    ///< in block mode. When block mode is enabled block mode functions only should be
                                    ///< used to put data. In this case circbuf_put_byte() must not be used. When block
                                    ///< mode is disabled (byte mode) circbuf_put_byte() must be used; block mode functions
                                    ///< must not be used in byte mode.

    /* CONST */ volatile uint8_t block_mode;    ///< Switches on block mode. 0: block mode disabled, non-zero block mode enabled.

    /* FLAGS */ volatile uint8_t ovf;           ///< indicates overflow, cleared by circbuf_clear_ovf()

    /* FLAGS */ volatile uint8_t wrn;           ///< warning status
};

/// \brief Initializes circular buffer data, except lock. Don't use for initialization, instead use circbuf_init()
/// \param circ - pointer to the circular buffer structure
/// \param buffer - pointer to the memory block used by circular buffer
/// \param length - length of the memory block
/// \note This function initializes circular buffer in byte mode.
void circbuf_init_data(volatile struct CircBuffer* circ, uint8_t *buffer, uint16_t length);

/// \brief Initializes circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param buffer - pointer to the memory block used by circular buffer
/// \param length - length of the memory block
/// \param context - additional parameter to be used to synchronize circular buffer. The value is the way circular buffer
///                  is being synchronized.
/// \note This function initializes circular buffer in byte mode.
__attribute__((always_inline))
static inline void circbuf_init(volatile struct CircBuffer* circ, uint8_t *buffer, uint16_t length, void* context) {
    circbuf_init_data(circ, buffer, length);
    seq_lock_init(&circ->lock, context);
}

/// \brief Initializes status data for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param status - pointer to the status data memory block.
/// \param length - length of the status data memory block.
/// \note Status data is sent first before any data from actual circular buffer. Status data allows to report some
///       data like extended device data or error codes to the software. It is somewhat allows to use usual buffer and
///       circular buffer as one object.
void circbuf_init_status(volatile struct CircBuffer* circ, volatile uint8_t* status, uint16_t length);

/// \brief Initializes warning check thresholds.
/// \param circ - pointer to the circular buffer structure
/// \param low_thr  - low threshold value. When size of accumulated data is lower than low_thr,
///        warning check returns zero.
/// \param high_thr - high threshold value. When size of accumulated data is greater than high_thr,
///        warning check returns non zero.
/// \note By default, low_thr and high_thr values are zero, and maximum capacity of the buffer.
///       Therefore, if circular buffer warning check is not initialized properly, warning check
///       will always return zero.
/// \note Warning check works as hysteresis.
void circbuf_init_warning(volatile struct CircBuffer* circ, uint16_t low_thr, uint16_t high_thr);

/// \brief Initializes block mode for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param bs - size of the block. Circular buffer size must be multiple to this value.
void circbuf_init_block_mode(volatile struct CircBuffer* circ, uint16_t bs);

/// \brief Performs circular buffer warning (synchronized).
/// \param circ - pointer to the circular buffer structure
/// \return Zero if size of accumulated data is lower than low threshold, non zero, if size of acccumulated data is
///         greater than high threshold. Hysteresis behaviour, if size of accumulated data is between low and high
///         thresholds.
/// \warning Occuracy of this function is not guranteed. Result may become invalid immidiately after the call.
///          This restriction should be understood. Also, note this function is read only, and doesn't change actual
///          buffer state. wrn member is not a buffer state. It's a variable to track history for histeresis calculation,
///          the latest value should be used.
__attribute__((always_inline))
static inline uint8_t circbuf_check_warning(volatile struct CircBuffer* circ){
    if (circ->data_len <= circ->warn_low_thr) {
        circ->wrn = 0;
    } else if (circ->data_len >= circ->warn_high_thr) {
        circ->wrn = 1;
    }
    return circ->wrn;
}

/// \brief Resets content of the buffer
/// \param circ - pointer to the circular buffer structure
/// \warning This function writes to circular buffer state, therefor corresponding lock will be obtained.
__attribute__((always_inline))
static inline void circbuf_reset  (volatile struct CircBuffer* circ) {
    seq_lock_write_acquire(&circ->lock);
    seq_lock_write_update(&circ->lock);
    assert_param(circ->current_block==0); // must not be called during any of operation
    circ->put_pos = 0;
    circ->data_len = 0;
    circ->read_pos = 0;
    circ->bytes_read = 0;
    circ->free_size = circ->buffer_size - circ->block_size;
    circ->current_block = 0;
    circ->ovf = 0;
    circ->wrn = (circ->data_len > circ->warn_high_thr);
    circbuf_check_warning(circ);
    seq_lock_write_release(&circ->lock);
}


/// \brief Returns amount of data stored in circular buffer (not including status data memory block)
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer, in bytes.
__attribute__((always_inline))
static inline uint16_t circbuf_len(volatile struct CircBuffer* circ){
    return circ->data_len;
}

/// \brief Returns amount of data stored in circular buffer including status data memory block.
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer and in status data memory block, in bytes.
__attribute__((always_inline))
static inline uint16_t circbuf_total_len(volatile struct CircBuffer* circ){
    return circ->status_size + circ->data_len;
}

/// \brief Put a byte into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \param b - byte to be put into circular buffer
/// \warning This function must be used for circular buffer working in byte mode only (block mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
__attribute__((always_inline))
static inline void circbuf_put_byte(volatile struct CircBuffer* circ, uint8_t b) {
    assert_param(circ->block_size == 1);
    assert_param(circ->block_mode == 0);
    seq_lock_write_acquire(&circ->lock);
    uint16_t data_len = circ->data_len;
    uint16_t put_pos = circ->put_pos++;
    uint8_t ovf = 0;
    if (data_len<circ->buffer_size) {
        circ->buffer[put_pos++] = b;
        data_len++;
        if (put_pos>=circ->buffer_size)
            put_pos = 0;
    } else {
        ovf=1;
    }

    seq_lock_write_update(&circ->lock);
    circ->data_len = data_len;
    circ->put_pos = put_pos;
    circ->ovf = ovf;
    seq_lock_write_release(&circ->lock);
}

/// \brief Initializes read operation from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \warning This function reads buffer state, therefor it uses sequential lock to prevent modifications during the call.
__attribute__((always_inline))
static inline void circbuf_start_read(volatile struct CircBuffer* circ) {
    seq_lock_read_acquire(&circ->lock);
    if (circ->put_pos >= circ->data_len) {
        circ->read_pos = circ->put_pos - circ->data_len;
    } else {
        circ->read_pos = circ->put_pos + circ->buffer_size - circ->data_len;
    }

    circ->bytes_read = 0;
    seq_lock_read_release(&circ->lock);
}


/// \brief Reads a byte from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param b - pointer to a byte to be written by read value. If buffer is empty #COMM_BAD_BYTE is returned.
/// \return non-zero if successfull, 0 if circular buffer is empty.
/// \warning This function doesn't check circular buffer state, so it calls read lock.
__attribute__((always_inline))
static inline uint8_t circbuf_get_byte(volatile struct CircBuffer* circ, volatile uint8_t *b) {
    uint8_t res = 1;
    seq_lock_read_acquire(&circ->lock);
    if (circ->bytes_read >= (circ->data_len + circ->status_size) ) {
        circ->ovf = 1;
        *b = COMM_BAD_BYTE;
        res = 0;
    } else if (circ->bytes_read >= circ->status_size) {
        *b = circ->buffer[circ->read_pos++];
        if (circ->read_pos>=circ->buffer_size) {
            circ->read_pos = 0;
        }
        circ->bytes_read++;
    } else {
        *b = circ->status[circ->bytes_read++];
    }
    seq_lock_read_release(&circ->lock);
    return res;
}



/// \brief Returns buffer overflow flag.
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if overflow flag is set, zero if overflow flag is cleared.
__attribute__((always_inline))
static inline uint8_t circbuf_get_ovf(volatile struct CircBuffer* circ) {
    return circ->ovf;
}

/// \brief Clears overflow flag.
/// \param circ - pointer to the circular buffer structure
__attribute__((always_inline))
static inline void circbuf_clear_ovf(volatile struct CircBuffer* circ) {
    circ->ovf = 0;
}

/// \brief Stop read operation from circular buffer. This will "commit" changes caused by read operations into circular
///        buffer structure.
/// \param circ - pointer to the circular buffer structure
/// \param num_bytes - number of bytes that was read. This number may be less then the actual number of bytes read.
/// \return number of bytes remaining in the buffer
/// \warning This function modifies state of the circular buffer, therefore write lock is aquired.
__attribute__((always_inline))
static inline uint16_t circbuf_stop_read(volatile struct CircBuffer* circ, uint16_t num_bytes) {

    if (num_bytes>circ->status_size) {
        num_bytes-=circ->status_size; // decrement status size
    } else {
        goto done; // status only was read - circular buffer state should not change
    }

    assert_param((num_bytes % circ->block_size)==0); // Do not allow reading from buffer by unaligned blocks.

    seq_lock_write_acquire(&circ->lock);
    if (num_bytes>circ->data_len) {
        num_bytes = circ->data_len;
    }
    seq_lock_write_update(&circ->lock);
    circ->data_len-=num_bytes;
    seq_lock_write_release(&circ->lock);

    done:
    return circ->data_len;
}

/// \brief Reserves block from circular buffer
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if success, if circular buffer may not allocate block, returns zero
/// \warning It is not possible to reserve more than one block at the moment.
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
__attribute__((always_inline))
static inline volatile void* circbuf_reserve_block(volatile struct CircBuffer* circ) {
    volatile uint8_t* current_block;
    uint8_t ovf;

    assert_param(circ->block_size > 0);
    assert_param(circ->block_mode == 1); // we must be in block mode

    // check for double allocation in debug only
    assert_param(circ->current_block == 0);

    seq_lock_write_acquire(&circ->lock);
    ovf = circ->ovf;
    current_block = circ->current_block;
    if (circ->free_size<circ->data_len) {
        ovf = 1;
        current_block = 0;
    } else {
        // figure out which block should be allocated
        current_block = (volatile uint8_t*)(circ->buffer + circ->put_pos);
    }

    seq_lock_write_update(&circ->lock);
    circ->current_block = current_block;
    circ->ovf = ovf;
    seq_lock_write_release(&circ->lock);

    return current_block;
}

/// \brief Commits block into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
__attribute__((always_inline))
static inline void circbuf_commit_block(volatile struct CircBuffer* circ) {
    assert_param(circ->block_size > 0);
    assert_param(circ->block_mode == 1); // we must be in block mode
    // check for unallocated block commit in debug only
    assert_param(circ->current_block != 0);

    seq_lock_write_acquire(&circ->lock);
    uint16_t put_pos = circ->put_pos + circ->block_size;
    uint16_t data_len = circ->data_len + circ->block_size;

    if (put_pos >= circ->buffer_size)
        put_pos = 0;

    seq_lock_write_update(&circ->lock);
    circ->current_block = 0;
    circ->put_pos = put_pos;
    circ->data_len = data_len;
    seq_lock_write_release(&circ->lock);
}


/// \brief Cancel reserved memory block
/// \param circ - pointer to the circular buffer structure
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
__attribute__((always_inline))
static inline void circbuf_cancel_block(volatile struct CircBuffer* circ) {
    assert_param(circ->block_size > 0);
    assert_param(circ->block_mode == 1); // we must be in block mode
    assert_param(circ->current_block != 0); // check for unallocated block commit in debug only
    // cancel it
    circ->current_block = 0;
}

/// @}

#ifdef __cplusplus
}
#endif
