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

#if defined(__arm__)
    #define ATOMIC_DECLARE(t,var_name)              volatile t var_name __attribute__ ((aligned))
    #define ATOMIC_TEST(var_name)                   IS_SIZE_ALIGNED((var_name))
    #define ATOMIC_READ(var_name, result)           (result) = (var_name)
    #define ATOMIC_INC(var_name, increment)         (var_name) += (increment)
#elif defined(__x86_64__)
    #define ATOMIC_DECLARE(t,var_name)               volatile t var_name __attribute__ ((aligned))
    #define ATOMIC_TEST(var_name)                    (void)(0)
    #define ATOMIC_READ(var_name, result)           __atomic_load(&(var_name), &(result), __ATOMIC_SEQ_CST)
    #define ATOMIC_INC(var_name, increment)         __atomic_fetch_add(&(var_name), (increment), __ATOMIC_SEQ_CST)
#else
    static_assert(0, "Unknown architecture !");
#endif


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

/* READER STATE */
struct __attribute__ ((aligned)) CircBufferReaderState {
    uint8_t* get_ptr;
    uint8_t* reader_ptr;
    int32_t  bytes_read;
};

/* WRITER STATE */
struct __attribute__ ((aligned)) CircBufferWriterState {
    uint8_t* put_ptr;
};



struct __attribute__ ((aligned)) CircBuffer {
    /* WRITER PRIV STATE */
    struct CircBufferWriterState writer_state __attribute__ ((aligned));

    /* READER PRIV STATE */
    struct CircBufferReaderState reader_state __attribute__ ((aligned));

    ///< [CONST] Buffer to be used as data storage for circular buffer.
    uint8_t *buffer __attribute__ ((aligned));

    ///< [CONST] Pointer to the first byte beyond buffer (use >= to check if pointer passed the buffer margin)
    uint8_t* buffer_end __attribute__ ((aligned));

    ///< [CONST] Optional status buffer to be prepended to the data to be sent to the software.
    uint8_t *status __attribute__ ((aligned));

    ///< [SHARED STATE (WR by WRITER)] Holds number of bytes successfully put into the buffer.
    ///< \warning Must not be modified by reader.
    ///< \warning This value may overlap through the maximum type limits. Therefor it's value alone has no practical sense,
    ///<          only (put_bytes_counter - get_bytes_counter) has meaning, which is current amount of data in the buffer.
    ///< \note Value doesn't include bytes read from status buffer.
    ATOMIC_DECLARE(int32_t, put_bytes_counter);
    // int32_t put_bytes_counter __attribute__ ((aligned));

    ///< [SHARED STATE (WR by READER)] Holds number of bytes successfully read from the buffer.
    ///< \warning Must not be modified by writer.
    ///< \warning This value may overlap through the maximum type limits. Therefor it's value alone has no practical sense,
    ///<          only (put_bytes_counter - get_bytes_counter) has meaning, which is current amount of data in the buffer.
    ///< \note Value doesn't include bytes read from status buffer.
    ATOMIC_DECLARE(int32_t, get_bytes_counter);
    // int32_t get_bytes_counter __attribute__ ((aligned));

    ///< [CONST] Size of the #buffer, in bytes.
    int32_t buffer_size __attribute__ ((aligned));

    ///< [CONST] Size of the #status_buffer;
    int32_t status_size __attribute__ ((aligned));

    ///< [CONST] Precalculated value that can be used to check if there is free space in buffer (free_size>=data_len)
    int32_t free_size __attribute__ ((aligned));

    ///< [CONST] Low warning check threshold
    int32_t warn_low_thr __attribute__ ((aligned));

    ///< [CONST] High warning check threshold
    int32_t warn_high_thr __attribute__ ((aligned));

    ///< [CONST] Circular buffer block size. Is actual only if block mode is used. Must be > 0.
    int32_t block_size __attribute__ ((aligned));

    ///< Switches on/off block mode. 0: block mode disabled, non-zero block mode enabled.
    ///< When block mode is enabled block mode functions only should be used to put data.
    ///< In this case circbuf_put_byte() must not be used.
    ///< When block mode is disabled (byte mode) circbuf_put_byte() must be used; block mode functions
    ///< must not be used in byte mode.
    uint8_t block_mode;

    ///< [FLAGS] Indicates overflow, i. e. situation when data may not be put or read.
    volatile uint8_t ovf;

    ///< [FLAGS] Warning status, due to hysteresis values specified.
    volatile uint8_t wrn;
};

/// \brief [INITIALIZER] Initializes circular buffer data, except lock. Don't use for initialization, instead use circbuf_init()
/// \param circ - pointer to the circular buffer structure
/// \param buffer - pointer to the memory block used by circular buffer
/// \param length - length of the memory block
/// \note This function initializes circular buffer in byte mode.
void circbuf_init_data(volatile struct CircBuffer* circ, uint8_t *buffer, int32_t length);


/// \brief [INITIALIZER] Initializes circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param buffer - pointer to the memory block used by circular buffer
/// \param length - length of the memory block
/// \note This function initializes circular buffer in byte mode.
__attribute__((always_inline))
static inline void circbuf_init(volatile struct CircBuffer* circ, uint8_t *buffer, int32_t length) {
    circbuf_init_data(circ, buffer, length);
}


/// \brief [INITIALIZER] Initializes status data for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param status - pointer to the status data memory block.
/// \param length - length of the status data memory block.
/// \note Status data is sent first before any data from actual circular buffer. Status data allows to report some
///       data like extended device data or error codes to the software. It is somewhat allows to use usual buffer and
///       circular buffer as one object.
void circbuf_init_status(volatile struct CircBuffer* circ, uint8_t* status, int32_t length);


/// \brief [INITIALIZER] Initializes warning check thresholds.
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


/// \brief [INITIALIZER] Initializes block mode for circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param bs - size of the block. Must be >1. Circular buffer size must be multiple to this value.
void circbuf_init_block_mode(volatile struct CircBuffer* circ, int32_t bs);


/// \brief Performs circular buffer warning check (as macro).
/// \param circ - pointer to the circular buffer structure.
/// \param data_len - pre-calculated size of the buffer.
/// \return Zero if size of accumulated data is lower than low threshold, non zero, if size of acccumulated data is
///         greater than high threshold. Hysteresis behaviour, if size of accumulated data is between low and high
///         thresholds.
/// \warning Accuracy of this function is not guaranteed. Result may become invalid immediately after the call, because
///          some data may by put or read in context of the interrupt. This restriction should be understood.
///          Also, note this macro is read only, and doesn't change circular buffer state.
#define circbuf_check_warning(circ, data_len) ( ( (circ)->wrn && ( (data_len) > (circ)->warn_low_thr) ) || \
                                                ( (data_len) >= (circ)->warn_high_thr ) )


/// \brief Performs circular buffer reset.
/// \param circ - pointer to the circular buffer structure.
/// \warning !!!____T_H_I_S____F_U_N_C_T_I_O_N____I_S____N_O_T____M_U_L_T_I_T_H_R_E_A_D___S_A_F_E___!!!
///          Caller device must make everything to make asynchronous access to circular buffer impossible at the moment
///          of the call.
///
///          !!!_________________D_O____N_O_T____D_I_S_A_B_L_E____I_N_T_E_R_R_U_P_T_S_______________!!!
///          Disabling interrupts or I2C interrupt will break I2C communication. Alternative is the following:
///          1. Software must disable device prior to reset it.
///          2. Software sends 'reset' command.
///          3. Software reads CommResponseHeader (may be with status) until it's length member is not zeroed and
///             COMM_STATUS_BUSY is cleared.
///
///          So, might be difficult, but works:
///          (1) Ensures device is not writing into circular buffer in any context.
///          (2) Reset's circular buffer.
///          (3) Ensures I2C bus is not reading from buffer, because CommResponseHeader is not part of the circular buffer.
__attribute__((always_inline))
static inline void circbuf_reset(volatile struct CircBuffer* circ) {
    /// Writing from writer to reader state because reader_state.get_ptr may be unaligned by block size,
    /// while writer_state.put_ptr is always aligned by block size.
    circ->reader_state.get_ptr = circ->writer_state.put_ptr;
    circ->get_bytes_counter = circ->put_bytes_counter;
    circ->ovf = 0;
    circ->wrn = circbuf_check_warning(circ, 0);
}


/// \brief [CONST] Returns amount of data stored in circular buffer (not including status data memory block)
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer, in bytes.
/// \note Returned number may not decrease for reader, therefor it is safe to use it while writer is writing data,
///       even asynchronously (no risk to read from empty buffer).
///       Also, returned number may not increase for writer, so it is safe to use it while reader is reading data,
///       evemt asynchronously (no risk to write to a full buffer).
__attribute__((always_inline))
static inline int32_t circbuf_len(volatile struct CircBuffer* circ){
    int32_t p,g,r;
    ATOMIC_READ(circ->put_bytes_counter, p);
    ATOMIC_READ(circ->get_bytes_counter, g);
    r = p - g;
    assert_param( r >= 0 );
    return r;
}


/// \brief [CONST] Returns amount of data stored in circular buffer including status data memory block.
/// \param circ - pointer to the circular buffer structure
/// \return amount of data stored in circular buffer and in status data memory block, in bytes.
__attribute__((always_inline))
static inline uint16_t circbuf_total_len(volatile struct CircBuffer* circ){
    return circbuf_len(circ) + circ->status_size;
}


/// \brief [CONST] Returns buffer overflow flag.
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if overflow flag is set, zero if overflow flag is cleared.
__attribute__((always_inline))
static inline uint8_t circbuf_get_ovf(volatile struct CircBuffer* circ) {
    return circ->ovf;
}

/// \brief [FLAGS] Returns buffer overflow flag.
/// \param circ - pointer to the circular buffer structure
/// \return Non-zero if overflow flag is set, zero if overflow flag is cleared.
__attribute__((always_inline))
static inline uint8_t circbuf_clear_ovf(volatile struct CircBuffer* circ) {
    // KEEP FOR A WHILE: NEW LOGIC TO BE REMOVED:
    // /// \warning If buffer doesn't have at least one free block, overflow is not cleared.
    // int32_t data_len = circbuf_len(circ);
    // circ->ovf = (circ->free_size < data_len);
    circ->ovf = 0;
    return circ->ovf;
}

/// \brief [CONST] Returns buffer warning flag.
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if warning flag is set, zero if warning flag is cleared.
__attribute__((always_inline))
static inline uint8_t circbuf_get_wrn(volatile struct CircBuffer* circ) {
    return circ->wrn;
}


/// \brief [READER] Initializes read operation from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \warning This function reads buffer state, therefor it uses sequential lock to prevent modifications during the call.
__attribute__((always_inline))
static inline void circbuf_start_read(volatile struct CircBuffer* circ) {
    circ->reader_state.reader_ptr = circ->reader_state.get_ptr;
    circ->reader_state.bytes_read = 0;
}


/// \brief [READER] Reads a byte from circular buffer.
/// \param circ - pointer to the circular buffer structure
/// \param b - pointer to a byte to be written by read value. If buffer is empty #COMM_BAD_BYTE is returned.
/// \return non-zero if successfull, 0 if circular buffer is empty.
/// \note This function may set overflow flag to indicate an attempt to read from empty buffer.
///       Also, it doesn't affect wrn flag.
__attribute__((always_inline))
static inline uint8_t circbuf_get_byte(volatile struct CircBuffer* circ, volatile uint8_t *b) {
    uint8_t res = 1;
    int32_t data_len = circbuf_len(circ);

    if (circ->reader_state.bytes_read < circ->status_size) {
        /// Read from status
        *b = circ->status[circ->reader_state.bytes_read];
        circ->reader_state.bytes_read++;
    } else if (circ->reader_state.bytes_read < (data_len + circ->status_size)) {
        /// Read from buffer
        *b = *circ->reader_state.reader_ptr;
        circ->reader_state.reader_ptr++;
        if (circ->reader_state.reader_ptr >= circ->buffer_end) {
            circ->reader_state.reader_ptr = circ->buffer;
        }
        circ->reader_state.bytes_read++;
    } else {
        /// Buffer underflow
        circ->ovf = 1;
        *b = COMM_BAD_BYTE;
        res = 0;
    }

    return res;
}


/// \brief [READER] Stop read operation from circular buffer. This will "commit" changes caused by read operations into circular
///        buffer structure.
/// \param circ - pointer to the circular buffer structure
/// \param num_bytes - number of bytes that was read. It must include number of bytes read from status buffer!
///        This number may be less then the actual number of bytes read.
/// \return number of bytes remaining in the buffer
/// \warning This function may clear warning flag, but it will NOT clear overflow flag, which should be cleared by virtual
///          device if required. It was made like this because device may be interested to report software an attempt to
///          read empty buffer.
__attribute__((always_inline))
static inline int32_t circbuf_stop_read(volatile struct CircBuffer* circ, int32_t num_bytes) {
    int32_t data_len = circbuf_len(circ);

    if (num_bytes>circ->status_size) {
        num_bytes-=circ->status_size; // decrement status size

        if (num_bytes>data_len) {
            num_bytes = data_len;
        }

        uint8_t* next_get_ptr = circ->reader_state.get_ptr + num_bytes;
        if (next_get_ptr >= circ->buffer_end) {
            next_get_ptr = circ->buffer + (next_get_ptr - circ->buffer_end);
        }
        circ->reader_state.get_ptr = next_get_ptr;
        ATOMIC_INC(circ->get_bytes_counter, num_bytes);
        //circ->get_bytes_counter += num_bytes;
        data_len -= num_bytes;

        circ->wrn = circbuf_check_warning(circ, data_len);
    }

    return data_len;
}


/// \brief [WRITER] Put a byte into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \param b - byte to be put into circular buffer
/// \return Number of bytes added ( 1 - success, 0 - failure ).
/// \warning This function must be used for circular buffer working in byte mode only (block mode is disabled).
__attribute__((always_inline))
static inline uint8_t circbuf_put_byte(volatile struct CircBuffer* circ, uint8_t b) {
    assert_param(circ->block_size == 1);
    assert_param(circ->block_mode == 0);

    uint16_t data_len = circbuf_len(circ);
    if (data_len < circ->buffer_size) {
        *circ->writer_state.put_ptr = b;

        circ->writer_state.put_ptr += 1;
        if (circ->writer_state.put_ptr >= circ->buffer_end) {
            circ->writer_state.put_ptr = circ->buffer;
        }

        data_len++;
        circ->wrn = circbuf_check_warning(circ, data_len);
        ATOMIC_INC(circ->put_bytes_counter, 1);
        return 1;
    } else {
        circ->ovf = 1;
        return 0;
    }
}


/// \brief [WRITER] Reserves block from circular buffer
/// \param circ - pointer to the circular buffer structure
/// \return non-zero if success, if circular buffer may not allocate block, returns zero
/// \warning It is not possible to reserve more than one block at the moment.
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
__attribute__((always_inline))
static inline volatile void* circbuf_reserve_block(volatile struct CircBuffer* circ) {
    volatile uint8_t* current_block;

    assert_param(circ->block_size > 0);
    assert_param(circ->block_mode == 1); // we must be in block mode

    int32_t data_len = circbuf_len(circ);
    if (circ->free_size < data_len) {
        current_block = 0;
        circ->ovf = 1;
    } else {
        current_block = circ->writer_state.put_ptr;
    }

    return current_block;
}


/// \brief [WRITER] Commits block into circular buffer
/// \param circ - pointer to the circular buffer structure
/// \warning This function must be used for circular buffer working in block mode only (byte mode is disabled).
__attribute__((always_inline))
static inline void circbuf_commit_block(volatile struct CircBuffer* circ) {
    assert_param(circ->block_mode == 1); // we must be in block mode
    assert_param(circ->block_size > 0);
    assert_param( (circ->buffer_size - circbuf_len(circ)) >= circ->block_size);

    uint8_t* next_put_ptr = circ->writer_state.put_ptr += circ->block_size;
    if (next_put_ptr >= circ->buffer_end) {
        next_put_ptr = circ->buffer + (next_put_ptr - circ->buffer_end);
    }
    circ->writer_state.put_ptr = next_put_ptr;
    //circ->put_bytes_counter += circ->block_size;
    ATOMIC_INC(circ->put_bytes_counter, circ->block_size);
    circ->wrn = circbuf_check_warning(circ, circbuf_len(circ));
}

/// @}

#ifdef __cplusplus
}
#endif
