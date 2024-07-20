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
 *   \brief Circular Buffer C source file.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include <circbuffer.h>

volatile uint32_t g_i2c_bus_writer_lock_flag = 0;


#ifdef DISABLE_NOT_TESTABLE_CODE
extern int g_assert_param_count;
#endif

void circbuf_init_data(volatile struct CircBuffer* circ, uint8_t* buffer, uint16_t length) {
	memset((void*)circ, 0, sizeof(struct CircBuffer));
	circ->buffer = buffer;
	circ->buffer_size = length;
    circ->free_size = length - 1; // Byte mode by default
    circ->block_size = 1;
    circ->warn_high_thr = circ->buffer_size;

    // As a bonus verify all members are aligned by word size.
    // If stuck somewhere below then check if circbuffer structure is packed and aligned properly.
    IS_SIZE_ALIGNED(&g_i2c_bus_writer_lock_flag);
    IS_SIZE_ALIGNED(&(circ->lock.context));
    IS_SIZE_ALIGNED(&(circ->lock.counter));
    IS_SIZE_ALIGNED(&(circ->buffer));
    IS_SIZE_ALIGNED(&(circ->status));
    IS_SIZE_ALIGNED(&(circ->buffer_size));
    IS_SIZE_ALIGNED(&(circ->status_size));
    IS_SIZE_ALIGNED(&(circ->put_pos));
    IS_SIZE_ALIGNED(&(circ->data_len));
    IS_SIZE_ALIGNED(&(circ->read_pos));
    IS_SIZE_ALIGNED(&(circ->bytes_read));
    IS_SIZE_ALIGNED(&(circ->free_size));
    IS_SIZE_ALIGNED(&(circ->warn_low_thr));
    IS_SIZE_ALIGNED(&(circ->warn_high_thr));
    IS_SIZE_ALIGNED(&(circ->block_size));
    IS_SIZE_ALIGNED(&(circ->block_mode));
    IS_SIZE_ALIGNED(&(circ->ovf));
    IS_SIZE_ALIGNED(&(circ->wrn));
}

void circbuf_init_status(volatile struct CircBuffer* circ, volatile uint8_t* status, uint16_t length) {
    circ->status = status;
    circ->status_size = length;
}

void circbuf_init_block_mode(volatile struct CircBuffer* circ, uint16_t bs) {
    assert_param(circ->block_size == 1);
    assert_param(circ->block_mode == 0);
    assert_param(bs > 0);
    assert_param(circ->buffer_size >= bs);
    assert_param((circ->buffer_size % bs) == 0);

    circ->block_size = bs;
    circ->buffer_size = (circ->buffer_size / bs) * bs; // make buffer multiply by dma_block_size anyway
    circ->free_size = circ->buffer_size - circ->block_size;
    circ->block_mode = 1;
}

void circbuf_init_warning(volatile struct CircBuffer* circ, uint16_t low_thr, uint16_t high_thr) {
    assert_param(low_thr < high_thr);
    assert_param(high_thr <= circ->buffer_size);
    circ->warn_low_thr = low_thr;
    circ->warn_high_thr = high_thr;
    circ->wrn = (circ->data_len > circ->warn_high_thr);
    circbuf_check_warning(circ);
}

