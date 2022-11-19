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
 *   \brief GPIO device C source file.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include "fw.h"
#include "utools.h"
#include "i2c_bus.h"
#include "gpio_dev.h"

#ifdef GPIODEV_DEVICE_ENABLED

typedef struct tag_GPIO_descr {
    GPIOMode_TypeDef type;
    GPIO_TypeDef* port;
    uint8_t pin_number;
    uint8_t default_val;
} GPIO_descr;

GPIO_descr gpio_descriptor[] = GPIO_DESCRIPTOR;

#define GPIO_COUNT          ((uint16_t)(sizeof(gpio_descriptor)/sizeof(GPIO_descr)))
#define GPIO_BUFFER_SIZE    ((GPIO_COUNT/8) + 1)

uint8_t gpio_out_pins[GPIO_COUNT];
uint8_t gpio_buffer[GPIO_BUFFER_SIZE];
volatile DeviceContext gpio_ctx __attribute__ ((aligned));

void gpio_update_values(uint8_t* buffer) {
	for (uint16_t i=0; i<GPIO_COUNT; i++) {
		uint16_t nbyte = i >> 3;
		uint8_t nbit = i & 0x07;
		uint16_t bitmask = 1 << nbit;
		uint16_t pin_mask = 1 << gpio_descriptor[i].pin_number;
		BitAction ba;

		if (gpio_out_pins[i]!=0) {
			// output
			ba = 1 & ((buffer[nbyte] & bitmask) >> nbit);
			GPIO_WriteBit(gpio_descriptor[i].port, pin_mask, ba);
		} else {
			// input
			ba = GPIO_ReadInputDataBit(gpio_descriptor[i].port, pin_mask);
			buffer[nbyte] = (buffer[nbyte] & ~(bitmask)) | (ba << nbit);
		}
	}
}

void gpio_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
	UNUSED(cmd_byte);

	if (length==GPIO_BUFFER_SIZE) {
		memcpy(gpio_buffer, data, GPIO_BUFFER_SIZE);
	}

	gpio_update_values(gpio_buffer);
    comm_done(0);
}

void gpio_read_done(uint8_t device_id, uint16_t length) {
	UNUSED(device_id);
	UNUSED(length);
    comm_done(0);
}


void gpio_init() {
	START_PIN_DECLARATION;
	BitAction ba;
	uint16_t bit_mask;
	memset(&gpio_buffer, 0, sizeof(gpio_buffer));

	// initialize pins
	for (uint16_t i=0; i<GPIO_COUNT; i++) {

		bit_mask = 1 << gpio_descriptor[i].pin_number;

		if (	gpio_descriptor[i].type==GPIO_Mode_Out_OD ||
				gpio_descriptor[i].type==GPIO_Mode_Out_PP) {
			gpio_out_pins[i]=1;
		} else {
			gpio_out_pins[i]=0;
		}

		if (gpio_out_pins[i]) {
			// configure output
			DECLARE_PIN(gpio_descriptor[i].port, bit_mask,gpio_descriptor[i].type);
			ba = gpio_descriptor[i].default_val==0 ? Bit_RESET : Bit_SET;
			GPIO_WriteBit(gpio_descriptor[i].port, bit_mask, ba);
			gpio_buffer[i >> 3] |= (ba << i % 0x07);
		} else {
			// configure input
			DECLARE_PIN(gpio_descriptor[i].port, bit_mask, gpio_descriptor[i].type);
		}
	}

	// initialize device context
	memset((void*)&gpio_ctx, 0, sizeof(gpio_ctx));
	gpio_ctx.device_id = GPIODEV_ADDR;
	gpio_ctx.buffer = gpio_buffer;
	gpio_ctx.bytes_available = GPIO_BUFFER_SIZE;
	gpio_ctx.on_command = gpio_dev_execute;
	gpio_ctx.on_read_done = gpio_read_done;
    comm_register_device(&gpio_ctx);

}

#endif