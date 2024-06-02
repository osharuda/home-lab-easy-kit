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
 *   \brief Interfacing desk device C source file.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include "fw.h"
#include "utools.h"
#include "i2c_bus.h"
#include "deskdev.h"
#include "extihub.h"
#include "sys_tick_counter.h"

#ifdef DESKDEV_DEVICE_ENABLED

DeskDevButtonState g_ddev_buttons[BUTTON_COUNT];
DeskDevEncoderState g_ddev_encoder;
DeskDevData g_ddev_data;
DeviceContext g_ddev_context __attribute__ ((aligned));

void CONTROLS_EXTI_HANDLER(uint64_t timestamp, volatile void* ctx);

void on_deskdev_read(uint8_t device_id, uint16_t length) {
	UNUSED(device_id);
	memset((void*)&g_ddev_data, 0, length < sizeof(g_ddev_data) ? length : sizeof(g_ddev_data) );
    comm_done(0);
}

void deskdev_init(void)
{
    memset((void*)&g_ddev_data, 0, sizeof(g_ddev_data));
    memset((void*)&g_ddev_context, 0, sizeof(g_ddev_context));
    memset((void*)g_ddev_buttons, 0, sizeof(g_ddev_buttons));
    memset((void*)&g_ddev_encoder, 0, sizeof(g_ddev_encoder));

    g_ddev_context.device_id = DESKDEV_ADDR;
    g_ddev_context.buffer = (uint8_t*)&g_ddev_data;
    g_ddev_context.bytes_available = sizeof(g_ddev_data);
    g_ddev_context.on_command = 0; // Works for read only
    g_ddev_context.on_read_done = on_deskdev_read;
    g_ddev_context.circ_buffer = 0;

    comm_register_device(&g_ddev_context);

    // Input buttons
    exti_register_callback(BUTTON_UP_PORT, BUTTON_UP_PIN, GPIO_Mode_IPU, BUTTON_UP_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)BUTTON_UP_PIN_MASK, 0);
    exti_register_callback(BUTTON_DOWN_PORT, BUTTON_DOWN_PIN, GPIO_Mode_IPU, BUTTON_DOWN_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)BUTTON_DOWN_PIN_MASK, 0);
    exti_register_callback(BUTTON_LEFT_PORT, BUTTON_LEFT_PIN, GPIO_Mode_IPU, BUTTON_LEFT_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)BUTTON_LEFT_PIN_MASK, 0);
    exti_register_callback(BUTTON_RIGHT_PORT, BUTTON_RIGHT_PIN, GPIO_Mode_IPU, BUTTON_RIGHT_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)BUTTON_RIGHT_PIN_MASK, 0);

	// Encoder
    exti_register_callback(ENCODER_A_PORT, ENCODER_A_PIN, GPIO_Mode_IPU, ENCODER_A_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)ENCODER_A_PIN_MASK, 0);
    exti_register_callback(ENCODER_B_PORT, ENCODER_B_PIN, GPIO_Mode_IPU, ENCODER_B_EXTICR, 1, 1, CONTROLS_EXTI_HANDLER, (volatile void*)ENCODER_B_PIN_MASK, 0);
}

void button_event(uint8_t button, uint8_t push)
{
	volatile DeskDevButtonState* btn = g_ddev_buttons + button;

	if (push==0 && btn->state != 0)
	{
		g_ddev_data.buttons[button]++;
	}

    btn->state = push;
}

void encoder_event(uint8_t channel, uint64_t timestamp)
{
	uint64_t last_ev_diff = get_tick_diff_64(g_ddev_encoder.last_ts, timestamp);
	if (g_ddev_encoder.ev_count >= 4) {
		if (g_ddev_encoder.last_ev == channel && last_ev_diff < ENCODER_LAST_EV_REJECT_MS) {
			// the very last event should be single, reject noise
			return;
		} else {
            g_ddev_encoder.ev_count = 0;
		}
	}

	
	if (last_ev_diff>ENCODER_STALE_DATA_MS) {
		// too long waited, start again
		g_ddev_encoder.ev_count = 0;
	}

    g_ddev_encoder.last_ts = timestamp;

	if (g_ddev_encoder.ev_count == 0) {
        g_ddev_encoder.last_ev = channel;
		g_ddev_encoder.ev_count++;
		return;
	} else if (g_ddev_encoder.last_ev == channel) {
		return;
	}

    g_ddev_encoder.last_ev = channel;
	g_ddev_encoder.ev_count++;

    if (g_ddev_encoder.ev_count == 4) {
		if (channel==ENCODER_A) {
			g_ddev_data.encoder++;
		} else {
			g_ddev_data.encoder--;
		}
	}
}
void CONTROLS_EXTI_HANDLER(uint64_t timestamp, volatile void* ctx)
{
    uint32_t events = (uint32_t)ctx;
	uint8_t push = 0;
	uint16_t tmp;

	if (events & BUTTON_UP_PIN_MASK)
	{
		tmp = GPIO_ReadInputData(BUTTON_UP_PORT);
		push = (tmp & BUTTON_UP_PIN_MASK)==0;
		button_event(BUTTON_UP, push);
	}

	if (events & BUTTON_DOWN_PIN_MASK)
	{
		tmp = GPIO_ReadInputData(BUTTON_DOWN_PORT);
		push = (tmp & BUTTON_DOWN_PIN_MASK)==0;
		button_event(BUTTON_DOWN, push);
	}

	if (events & BUTTON_RIGHT_PIN_MASK)
	{
		tmp = GPIO_ReadInputData(BUTTON_RIGHT_PORT);
		push = (tmp & BUTTON_RIGHT_PIN_MASK)==0;
		button_event(BUTTON_RIGHT, push);
	}


	if (events & BUTTON_LEFT_PIN_MASK)
	{
		tmp = GPIO_ReadInputData(BUTTON_LEFT_PORT);
		push = (tmp & BUTTON_LEFT_PIN_MASK)==0;
		button_event(BUTTON_LEFT, push);
	}


	if (events & ENCODER_A_PIN_MASK)
	{
		encoder_event(ENCODER_A, timestamp);
	}

	if (events & ENCODER_B_PIN_MASK)
	{
		encoder_event(ENCODER_B, timestamp);
	}
}

#endif