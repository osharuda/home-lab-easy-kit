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
 *   \brief LCD1602a (HD44780) screen device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef LCD1602a_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "lcd_1602A.h"
#include "sys_tick_counter.h"
#include "lcd1602a_conf.h"



/// \defgroup group_lcd_1602a_dev LCD1602ADev
/// \brief Simple LCD1602a screen support
/// @{
/// \brief LCDDev #tag_DeviceContext structure
struct DeviceContext g_lcd_ctx __attribute__ ((aligned));

/// \brief Specifies if backlight is currently on
volatile uint8_t         g_lcd_light_on = 0;

/// \brief Specifies if blinking mode is currently used
volatile uint8_t         g_lcd_blink = 0;
/// @}

void lcd_init()
{
    START_PIN_DECLARATION;

    DECLARE_PIN(LCD1602a_ENABLE_PORT, 			LCD1602a_ENABLE_PIN_MASK, 			GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_REGISTER_SELECT_PORT, 	LCD1602a_REGISTER_SELECT_PIN_MASK, 	GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_DATA4_PORT, 			LCD1602a_DATA4_PIN_MASK, 				GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_DATA5_PORT, 			LCD1602a_DATA5_PIN_MASK, 				GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_DATA6_PORT, 			LCD1602a_DATA6_PIN_MASK, 				GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_DATA7_PORT, 			LCD1602a_DATA7_PIN_MASK, 				GPIO_Mode_Out_PP);
    DECLARE_PIN(LCD1602a_LIGHT_PORT, 			LCD1602a_LIGHT_PIN_MASK, 				GPIO_Mode_Out_PP);

    delay_ms(15);
    lcd_half_byte(0x03, LCD1602a_MODE_CMD);
    delay_ms(5);
    lcd_half_byte(0x03, LCD1602a_MODE_CMD);
    delay_ms(1);
    lcd_half_byte(0x03, LCD1602a_MODE_CMD);
    lcd_half_byte(0x02, LCD1602a_MODE_CMD);

    lcd_byte(0x28, LCD1602a_MODE_CMD); // Function set: 4 bit interface,
    lcd_byte(0x08, LCD1602a_MODE_CMD); // Display off
    lcd_byte(0x01, LCD1602a_MODE_CMD); // Display clear
    lcd_byte(0x06, LCD1602a_MODE_CMD); // Clear display
    lcd_byte(0x0C, LCD1602a_MODE_CMD); // Display on

    memset((void*)&g_lcd_ctx, 0, sizeof(g_lcd_ctx));

    g_lcd_ctx.device_id = LCD1602a_ADDR;
    g_lcd_ctx.buffer = 0;	// No buffer available
	g_lcd_ctx.on_command = lcd_dev_execute;
    g_lcd_ctx.on_read_done = 0;
    g_lcd_ctx.on_polling = lcd_polling;
    g_lcd_ctx.polling_period = LCD1602a_BLINK_EVERY_US;

    comm_register_device(&g_lcd_ctx);
    g_lcd_blink = 0;
    lcd_set_backlight(1);

    lcd_string((uint8_t*)LCD1602a_WELCOME_1, LCD1602a_LINE_1);
    lcd_string((uint8_t*)LCD1602a_WELCOME_2, LCD1602a_LINE_2);
}

void lcd_polling(uint8_t device_id) {
    UNUSED(device_id);
	if (g_lcd_blink) {
		lcd_set_backlight(g_lcd_light_on == 0 ? 1 : 0);
	}
}

uint8_t lcd_positional_write(uint8_t* data, uint16_t length) {
	uint8_t status = 0;
	uint8_t text_len;
	uint8_t tmp;
	struct LcdPositionalText* pos_text = (struct LcdPositionalText*)data;

	if (length<sizeof(struct LcdPositionalText)) {
		status = COMM_STATUS_FAIL;
		goto done;
	}

	if (	pos_text->line<LCD1602a_POSITION_MINLINE ||
			pos_text->line>LCD1602a_POSITION_MAXLINE) {
		status = COMM_STATUS_FAIL;
		goto done;
	}

	text_len = length-sizeof(struct LcdPositionalText);
	if (pos_text->position>=LCD1602a_WIDTH ||
		pos_text->position+text_len>LCD1602a_WIDTH) {
		status = COMM_STATUS_FAIL;
		goto done;

	}

	tmp = (pos_text->line == LCD1602a_POSITION_MINLINE) ? LCD1602a_LINE_1 : LCD1602a_LINE_2;
	tmp += pos_text->position;
	lcd_byte(tmp, LCD1602a_MODE_CMD); // set address

	for (uint32_t i=0; i<text_len; i++)
	{
		lcd_byte(pos_text->text[i], LCD1602a_MODE_CHR);
	}

done:
	return status;
}

uint8_t lcd_full_write(uint8_t* data, uint16_t length) {
	uint8_t status = 0;

	if (length!=LCD1602a_WIDTH*LCD1602a_POSITION_MAXLINE) {
		status = COMM_STATUS_FAIL;
		goto done;
	}

	lcd_string(data, LCD1602a_LINE_1);
	lcd_string(data + LCD1602a_WIDTH, LCD1602a_LINE_2);

done:
	return status;
}

uint8_t lcd_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
	uint8_t status = 0;

	// Manage back light
	g_lcd_blink = cmd_byte & LCD1602a_BLINK;
	lcd_set_backlight(cmd_byte & LCD1602a_LIGHT);

	// Is it valid command
	if (length==0) {
		goto done;// light only command
	}

	// Positional write
	if (cmd_byte & LCD1602a_POSITION) {
		status = lcd_positional_write(data, length);
	} else {
		status = lcd_full_write(data, length);
	}

done:
    return status;
}

void lcd_toggle_enabled()
{
    delay_us(LCD1602a_WAIT);
    GPIO_WriteBit(LCD1602a_ENABLE_PORT, LCD1602a_ENABLE_PIN_MASK, 1);
    delay_us(LCD1602a_WAIT);
    GPIO_WriteBit(LCD1602a_ENABLE_PORT, LCD1602a_ENABLE_PIN_MASK, 0);
}

void lcd_half_byte(uint8_t byte, uint32_t mode)
{
    GPIO_WriteBit(LCD1602a_REGISTER_SELECT_PORT, LCD1602a_REGISTER_SELECT_PIN_MASK, mode);     // Select mode

    GPIO_WriteBit(LCD1602a_DATA4_PORT, LCD1602a_DATA4_PIN_MASK, (byte >> 0) & 1);              // Set higher bits
    GPIO_WriteBit(LCD1602a_DATA5_PORT, LCD1602a_DATA5_PIN_MASK, (byte >> 1) & 1);              // Set higher bits
    GPIO_WriteBit(LCD1602a_DATA6_PORT, LCD1602a_DATA6_PIN_MASK, (byte >> 2) & 1);              // Set higher bits
    GPIO_WriteBit(LCD1602a_DATA7_PORT, LCD1602a_DATA7_PIN_MASK, (byte >> 3) & 1);              // Set higher bits

    lcd_toggle_enabled();                                                    // Toggle enable pin
}

void lcd_byte(uint8_t byte, uint32_t mode)
{
    lcd_half_byte( (byte >> 4) & 0x0F, mode);
    lcd_half_byte( byte & 0x0F, mode);
}

void lcd_string(volatile uint8_t* message, uint32_t line)
{
    lcd_byte(line, LCD1602a_MODE_CMD);

    for (uint32_t i=0; i<LCD1602a_WIDTH && message[i]!=0; i++)
    {
        lcd_byte(message[i], LCD1602a_MODE_CHR);
    }
}

void lcd_set_backlight(uint8_t enable)
{
    g_lcd_light_on = (enable == 0 ? 0 : 1);
	GPIO_WriteBit(LCD1602a_LIGHT_PORT, LCD1602a_LIGHT_PIN_MASK, g_lcd_light_on);
}

#endif