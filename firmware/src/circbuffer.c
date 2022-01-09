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
#include "i2c_proto.h"
#include "circbuffer.h"
#include "utools.h"

#ifdef DISABLE_NOT_TESTABLE_CODE
// This is used for test purposes only
#define DISABLE_IRQ
#define ENABLE_IRQ
extern int g_assert_param_count;
#endif

void circbuf_init(volatile PCircBuffer circ, uint8_t* buffer, uint16_t length) {
	memset((void*)circ, 0, sizeof(CircBuffer));
	circ->buffer = buffer;
	circ->buffer_size = length;
    circ->free_size = length - 1; // Byte mode by default
    circ->block_size = 1;
    circ->status = 0;
    circ->status_size = 0;
}

void circbuf_init_status(volatile PCircBuffer circ, volatile uint8_t* status, uint16_t length) {
    circ->status = status;
    circ->status_size = length;
}

void circbuf_reset(volatile PCircBuffer circ) {
    assert_param(circ->current_block==0); // must not be called during any of operation
    DISABLE_IRQ
    circ->put_pos = 0;
    circ->start_pos = 0;
    circ->data_len = 0;
    circ->read_pos = 0;
    circ->bytes_read = 0;
    circ->free_size = circ->buffer_size - circ->block_size;
    circ->current_block = 0;
    circ->ovf = 0;
    ENABLE_IRQ
}

void circbuf_reset_no_irq(volatile PCircBuffer circ) {
    assert_param(circ->current_block==0); // must not be called during any of operation
    circ->put_pos = 0;
    circ->start_pos = 0;
    circ->data_len = 0;
    circ->read_pos = 0;
    circ->bytes_read = 0;
    circ->free_size = circ->buffer_size - circ->block_size;
    circ->current_block = 0;
    circ->ovf = 0;
}

uint16_t circbuf_len(volatile PCircBuffer circ) {
	uint16_t len = 0;
	DISABLE_IRQ
	len = circ->data_len;
	ENABLE_IRQ
	return len;
}

uint16_t circbuf_total_len(volatile PCircBuffer circ) {
    uint16_t len = circ->status_size;
    DISABLE_IRQ
    len += circ->data_len;
    ENABLE_IRQ
    return len;
}

uint16_t circbuf_total_len_no_irq(volatile PCircBuffer circ) {
    uint16_t len = circ->status_size;
    len += circ->data_len;
    return len;
}

// returns 1 if overflow
void circbuf_put_byte(volatile PCircBuffer circ, uint8_t b) {
    assert_param(circ->block_size == 1);
    assert_param(circ->current_block == 0);
	DISABLE_IRQ
	if (circ->data_len<circ->buffer_size) {
		circ->buffer[circ->put_pos++] = b;
		circ->data_len++;
		if (circ->put_pos>=circ->buffer_size)
			circ->put_pos = 0;
	} else {
		circ->ovf=1;
	}
	ENABLE_IRQ
}

void circbuf_start_read(volatile PCircBuffer circ) {
	DISABLE_IRQ
	circ->read_pos = circ->start_pos;
	circ->bytes_read = 0;
	ENABLE_IRQ
}

// returns number of bytes returned (1 or 0)
uint8_t circbuf_get_byte(volatile PCircBuffer circ, volatile uint8_t* b) {
	uint8_t res = 1;
	DISABLE_IRQ

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
	ENABLE_IRQ

	return res;
}

void circbuf_clear_ovf(volatile PCircBuffer circ) {
	DISABLE_IRQ
		circ->ovf = 0;
	ENABLE_IRQ
}

uint8_t circbuf_get_ovf(volatile PCircBuffer circ) {
	return circ->ovf;
}

uint16_t circbuf_stop_read(volatile PCircBuffer circ, uint16_t num_bytes) {
	uint16_t bytes_remain;
	DISABLE_IRQ

	if (num_bytes>circ->status_size) {
	    num_bytes-=circ->status_size; // decrement status size
	} else {
	    goto done; // status only was read - circular buffer state should not change
	}

	assert_param((num_bytes % circ->block_size)==0); // Do not allow reading from buffer by unaligned blocks.

	if (num_bytes>circ->data_len) {
		num_bytes = circ->data_len;
	}

	circ->data_len-=num_bytes;
	circ->start_pos += num_bytes;
	if (circ->start_pos>=circ->buffer_size) {
		circ->start_pos -= circ->buffer_size;
	}

done:
	bytes_remain = circ->data_len;
	ENABLE_IRQ
	return bytes_remain;
}

void circbuf_init_block_mode(volatile PCircBuffer circ, uint16_t bs) {
    assert_param(circ->block_size == 1);
    assert_param(bs > 1);
    assert_param(circ->buffer_size >= bs);
    assert_param((circ->buffer_size % bs) == 0);

    DISABLE_IRQ
    circ->block_size = bs;
    circ->buffer_size = (circ->buffer_size / bs) * bs; // make buffer multiply by dma_block_size anyway

    // all the data existed in circular buffer is discarded
    circ->put_pos = 0;
    circ->start_pos = 0;
    circ->data_len = 0;
    circ->read_pos = 0;
    circ->bytes_read = 0;
    circ->ovf = 0;
    circ->free_size = circ->buffer_size - circ->block_size;
    ENABLE_IRQ
}

void* circbuf_reserve_block(volatile PCircBuffer circ) {
    void* res;
    assert_param(circ->block_size > 1); // we must be in block mode
    DISABLE_IRQ

    // check for double allocation in debug only
    assert_param(circ->current_block == 0);

    // check there is a space
    if (circ->free_size<circ->data_len) {
        circ->ovf = 1;
        res = 0;
    } else {
        // figure out which block should be allocated
        res = (void*)(circ->buffer + circ->put_pos);

        // reserve it
        circ->current_block = res;
    }

    ENABLE_IRQ

    return res;
}

void circbuf_commit_block(volatile PCircBuffer circ) {
    DISABLE_IRQ

    assert_param(circ->block_size > 1); // we must be in block mode
    // check for unallocated block commit in debug only
    assert_param(circ->current_block != 0);

    // commit it
    circ->put_pos+=circ->block_size;
    if (circ->put_pos >= circ->buffer_size)
        circ->put_pos = 0;

    circ->data_len+=circ->block_size;
    circ->current_block = 0;
    ENABLE_IRQ
}

void circbuf_cancel_block(volatile PCircBuffer circ) {
    assert_param(circ->block_size > 1); // we must be in block mode
    DISABLE_IRQ

    // check for unallocated block commit in debug only
    assert_param(circ->current_block != 0);

    // cancel it
    circ->current_block = 0;

    ENABLE_IRQ
}
