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

 /* --------------------> END OF THE TEMPLATE HEADER <-------------------- */

/// \addtogroup group_lcd_1602a_dev
/// @{{

/// \defgroup group_lcd_1602a_dev_modes LCD1602a parameters
/// @{{

/// \def LCD1602a_LIGHT
/// \brief Specifies that backlight is on.
#define LCD1602a_LIGHT                   128

/// \def LCD1602a_POSITION
/// \brief Specifies that positional write must be made.
#define LCD1602a_POSITION                64

/// \def LCD1602a_BLINK
/// \brief Specifies that backlight is blinking (once every #LCD1602a_BLINK_EVERY_US)
#define LCD1602a_BLINK                   32

/// \def LCD1602a_OFF
/// \brief Specifies that backlight is turned off.
#define LCD1602a_OFF                     0

/// @}}

/// \struct tag_LcdPositionalText
/// \brief Used to make positional text write
#pragma pack(push, 1)
struct LcdPositionalText{{
	uint8_t line;       ///< Specify line to write.
	uint8_t position;   ///< Specify character position to write.
	uint8_t text[];     ///< Text to write (size of the string is structure length substracted by sizeof(tag_LcdPositionalText).
}};
#pragma pack(pop)

/// \def LCD1602a_BLINK_EVERY_US
/// \brief Half period of blinking, in microseconds.
#define LCD1602a_BLINK_EVERY_US          500000

/// \def LCD1602a_POSITION_MINLINE
/// \brief Minimum line of the screen (top button)
#define LCD1602a_POSITION_MINLINE        1

/// \def LCD1602a_POSITION_MAXLINE
/// \brief Maximum line of the screen (bottom button)
#define LCD1602a_POSITION_MAXLINE        2

/// \def LCD1602a_WIDTH
/// \brief Number of characters in line
#define LCD1602a_WIDTH                   16

/// \def LCD1602a_MAX_POSIOTION
/// \brief Maximum position of the character in line
#define LCD1602a_MAX_POSIOTION           (LCD1602a_POSITION_CHARS-1)

/// @}}