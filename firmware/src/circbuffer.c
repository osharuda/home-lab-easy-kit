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

void circbuf_init_data(volatile struct CircBuffer* circ, uint8_t* buffer, int32_t length) {
    assert_param(length > 0);
    // Check members are aligned by size, it is important because otherwise speed may degrade,
    // and operation atomicity may be broken.
    IS_SIZE_ALIGNED(&(circ->reader_state.reader_ptr));
    IS_SIZE_ALIGNED(&(circ->reader_state.get_ptr));
    IS_SIZE_ALIGNED(&(circ->reader_state.bytes_read));
    ATOMIC_TEST(&(circ->get_bytes_counter));
    ATOMIC_TEST(&(circ->put_bytes_counter));
    IS_SIZE_ALIGNED(&(circ->writer_state.put_ptr));
    IS_SIZE_ALIGNED(&g_i2c_bus_writer_lock_flag);
    IS_SIZE_ALIGNED(&(circ->buffer));
    IS_SIZE_ALIGNED(&(circ->status));
    IS_SIZE_ALIGNED(&(circ->buffer_size));
    IS_SIZE_ALIGNED(&(circ->status_size));
    IS_SIZE_ALIGNED(&(circ->free_size));
    IS_SIZE_ALIGNED(&(circ->warn_low_thr));
    IS_SIZE_ALIGNED(&(circ->warn_high_thr));
    IS_SIZE_ALIGNED(&(circ->block_size));
    IS_SIZE_ALIGNED(&(circ->block_mode));
    IS_SIZE_ALIGNED(&(circ->ovf));
    IS_SIZE_ALIGNED(&(circ->wrn));

	memset((void*)circ, 0, sizeof(struct CircBuffer));
	circ->buffer        = buffer;
    circ->buffer_end    = buffer + length;
    circ->reader_state.get_ptr = buffer;
    circ->reader_state.reader_ptr = buffer;
    circ->writer_state.put_ptr = buffer;
	circ->buffer_size   = length;
    circ->free_size     = length - 1; // Byte mode by default
    circ->block_size    = 1;
    circ->warn_high_thr = circ->buffer_size;
}

void circbuf_init_status(volatile struct CircBuffer* circ, uint8_t* status, int32_t length) {
    assert_param( length > 0 );
    circ->status = status;
    circ->status_size = length;
}

void circbuf_init_block_mode(volatile struct CircBuffer* circ, int32_t bs) {
    assert_param(circ->block_size == 1);
    assert_param(circ->block_mode == 0);
    assert_param(bs >= 1);
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
    int32_t data_len = circbuf_len(circ);
    circ->warn_low_thr = low_thr;
    circ->warn_high_thr = high_thr;
    circ->wrn = (data_len > circ->warn_high_thr);
    circ->wrn = circbuf_check_warning(circ, data_len);
}

