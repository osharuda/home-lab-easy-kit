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
 *   \brief IRRC (Infra Red Remote Control) device C source file.
 *   \author Oleh Sharuda
 */
#include "fw.h"
#ifdef IRRC_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "irrc.h"
#include "extihub.h"
#include "sys_tick_counter.h"
#include "irrc_conf.h"


struct IRRCPrivData{
    uint64_t signal_start;
    uint64_t last_bit_start;
    struct CircBuffer circ;
    uint32_t data;
    uint8_t	state;
    uint8_t bitcounter;
    uint8_t last_actual;
    uint8_t last_ir_address;
    uint8_t last_ir_command;
    uint8_t buffer[IRRC_BUF_LEN];
};


struct IRRCPrivData irrc_data;
struct DeviceContext irrc_ctx __attribute__ ((aligned));

void IRRC_EXTI_HANDLER(uint64_t now, volatile void* ctx);

static inline
void irrc_recv_init() {
    irrc_data.state = IRRC_NEC_NO_SIGNAL;
    irrc_data.bitcounter = 0;
    irrc_data.data = 0;
    irrc_data.last_bit_start = 0;
}

void irrc_init() {
    memset((void*)&irrc_data, 0, sizeof(struct IRRCPrivData));
    circbuf_init(&irrc_data.circ,irrc_data.buffer,IRRC_BUF_LEN);

    circbuf_init_block_mode(&irrc_data.circ, sizeof(uint16_t));

	memset((void*)&irrc_ctx, 0, sizeof(struct DeviceContext));
	irrc_ctx.device_id = IRRC_ADDR;
	irrc_ctx.buffer = 0;
	irrc_ctx.circ_buffer = &(irrc_data.circ);
	irrc_ctx.on_read_done = irrc_readdone;

    comm_register_device(&irrc_ctx);

    // register exti interrupt callback
    exti_register_callback(IRRC_OUT_PORT,
                                IRRC_OUT_PIN,
                                GPIO_Mode_IPU,
                                IRRC_EXTICR,
                                0,
                                1,
                                IRRC_EXTI_HANDLER,
                                &irrc_data, 0);

    irrc_data.signal_start = get_us_clock()-IRRC_NEC_REPEAT_MAX;
    irrc_recv_init();
}

uint8_t irrc_readdone(uint8_t device_id, uint16_t length) {
	UNUSED(device_id);
	circbuf_stop_read(&irrc_data.circ, length);
	circbuf_clear_ovf(&irrc_data.circ);
    return COMM_STATUS_OK;
}

void IRRC_EXTI_HANDLER(uint64_t now, volatile void* ctx) {
    UNUSED(ctx);
    now = SYSTICK_TO_uS(now);
	volatile uint64_t diff = get_tick_diff_64(irrc_data.signal_start, now);
	if (diff>=IRRC_NEC_REPEAT_MAX) {
		// seems like uncompleted transmission, clear previous data
		irrc_recv_init();
		irrc_data.last_actual = 0;
		irrc_data.last_ir_address = 0;
		irrc_data.last_ir_command = 0;
	}

	if (irrc_data.state==IRRC_NEC_NO_SIGNAL) {
		irrc_data.signal_start = now;
		irrc_data.state = IRRC_NEC_LEAD_PULSE;
	} else if (irrc_data.state==IRRC_NEC_LEAD_PULSE) {
		if (diff>=(IRRC_NEC_LEAD_PULSE_MIN + IRRC_NEC_LEAD_SPACE_MIN) && diff<=(IRRC_NEC_LEAD_PULSE_MAX + IRRC_NEC_LEAD_SPACE_MAX)) {
			// code is sent
			irrc_data.state = IRRC_NEC_DATA;
			irrc_data.last_bit_start = now;
		} else if (diff>=(IRRC_NEC_LEAD_PULSE_MIN + IRRC_NEC_LEAD_RPT_SPACE_MIN) && diff<=(IRRC_NEC_LEAD_PULSE_MAX + IRRC_NEC_LEAD_RPT_SPACE_MAX) && irrc_data.last_actual!=0) {
            volatile uint8_t* data = circbuf_reserve_block(&irrc_data.circ);
            if (data) {
                data[0] = irrc_data.last_ir_address;
                data[1] = irrc_data.last_ir_command;
                circbuf_commit_block(&irrc_data.circ);
            }
			irrc_recv_init();
		} else {
			irrc_recv_init();	// something wrong, may be noise ?
		}
	} else if (irrc_data.state==IRRC_NEC_DATA) {
		diff = get_tick_diff_64(irrc_data.last_bit_start, now);
		if (diff>=IRRC_NEC_1_MIN && diff<=IRRC_NEC_1_MAX) {
			// 1
			irrc_data.data |= (1 << irrc_data.bitcounter);
		} else if (diff>=IRRC_NEC_0_MIN && diff<=IRRC_NEC_0_MAX) {
		} else {
			irrc_recv_init();	// something wrong, may be noise ?
			return;
		}
		irrc_data.last_bit_start=now;
		irrc_data.bitcounter++;

		if (irrc_data.bitcounter>IRRC_NEC_MAX_BIT) {

			// enough, check data
			diff = get_tick_diff_64(irrc_data.signal_start, now);
			uint8_t* pb = (uint8_t*)&irrc_data.data;

			uint8_t ir_address = pb[1];
			uint8_t ir_address_chk = ~pb[0];
			uint8_t ir_cmd = pb[3];
			uint8_t ir_cmd_chk = ~pb[2];

			if (diff >= IRRC_NEC_ALL_SEQUANCE_MIN && diff <= IRRC_NEC_ALL_SEQUANCE_MAX && ir_address==ir_address_chk &&  ir_cmd==ir_cmd_chk){
				// add data to circular buffer
				irrc_data.last_actual = 1;
				irrc_data.last_ir_address = ir_address;
				irrc_data.last_ir_command = ir_cmd;
			}

			irrc_recv_init();
		}
	}
}

#endif