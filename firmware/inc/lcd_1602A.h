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
 *   \brief LCD1602a (HD44780) screen device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef LCD1602a_DEVICE_ENABLED

/// \defgroup group_lcd_1602a_dev LCD1602ADev
/// \brief Simple LCD1602a screen support
/// @{
/// \page page_lcd1602a_dev
/// \tableofcontents
///
/// \section sect_lcd1602a_dev_01 Implementation
/// LCD1602a is two lines with sixteen character length LCD screen. It's implementation is straightforward, however
/// a few thing should be separately noted:
/// - Blinking mode is organized via #ON_POLLING callback
/// - 4 bit mode is used. It means that each byte is sent with 4 bit line.
/// - Currently two line screens are supported, however it's not a big deal to extend it for other screens like LCD1604.
///

/// \addtogroup group_lcd_1602a_dev_modes
/// @{

/// \def LCD1602a_WAIT
/// \brief Wait to be used in LCD1602a commands.
#define LCD1602a_WAIT        1000 // microseconds

/// \def LCD1602a_MODE_CHR
/// \brief Character mode of the LCD1602a.
#define LCD1602a_MODE_CHR    1    // Character mode

/// \def LCD1602a_MODE_CMD
/// \brief Command mode of the LCD1602a.
#define LCD1602a_MODE_CMD    0    // Command mode

/// \def LCD1602a_LINE_1
/// \brief Top line.
#define LCD1602a_LINE_1      0x80 // LCD RAM address for the 1st line

/// \def LCD1602a_LINE_2
/// \brief Bottom line.
#define LCD1602a_LINE_2      0xC0 // LCD RAM address for the 2nd line

/// @}

/// \brief Init LCD1602a screen
void lcd_init();

/// \brief Write a line on a screen
/// \param message - zero terminated string to write
/// \param line - line a string to be put on (#LCD1602a_LINE_1 or #LCD1602a_LINE_2)
void lcd_string(volatile uint8_t* message, uint32_t line);

/// \brief Sends a byte into LCD1602a
/// \param byte - byte to be send
/// \param mode - mode to be used (#LCD1602a_MODE_CMD or #LCD1602a_MODE_CMD)
void lcd_byte(uint8_t byte, uint32_t mode);

/// \brief Set's backlight
/// \param enable - non-zero to enable backlight, zero to disable.
void lcd_set_backlight(uint8_t enable);

/// \brief Toggles enable bit
void lcd_toggle_enabled();

/// \brief Sends a half byte into LCD1602a
/// \param byte - half byte to send (four high bits are ignored)
/// \param mode - mode to be used (#LCD1602a_MODE_CMD or #LCD1602a_MODE_CMD)
void lcd_half_byte(uint8_t byte, uint32_t mode);

/// \brief #ON_COMMAND callback for LCDDev
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Result of the operation as communication status.
uint8_t lcd_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_POLLING callback for LCDDev
/// \param device_id - Device ID of the virtual device which data was read
void lcd_polling(uint8_t device_id);

/// @}

#endif