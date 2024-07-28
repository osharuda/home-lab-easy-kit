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

/// \def BUTTON_COUNT
/// \brief Number of buttons.
#define BUTTON_COUNT			4

/// \def BUTTON_UP
/// \brief Up button index.
#define BUTTON_UP				0

/// \def BUTTON_DOWN
/// \brief Down button index.
#define BUTTON_DOWN				1

/// \def BUTTON_LEFT
/// \brief Left button index.
#define BUTTON_LEFT				2

/// \def BUTTON_RIGHT
/// \brief Right button index.
#define BUTTON_RIGHT			3

#pragma pack(push, 1)
/// \struct DeskDevData
/// \brief Desk data structure
struct DeskDevData{{
	uint8_t buttons[BUTTON_COUNT];  ///< Array with button press counters for each button.
	int8_t  encoder;                ///< Encoder rotation value.
}};
#pragma pack(pop)
