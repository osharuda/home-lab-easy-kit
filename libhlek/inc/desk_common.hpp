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
 *   \brief Generated include header of software part for interfacing desk device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>

/// \addtogroup group_desk_dev
/// @{



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
/// \struct tag_DeskDevData
/// \brief Desk data structure
typedef struct tag_DeskDevData{
	uint8_t buttons[BUTTON_COUNT];  ///< Array with button press counters for each button.
	int8_t  encoder;                ///< Encoder rotation value.
} DeskDevData;
#pragma pack(pop)

typedef volatile DeskDevData* PDeskDevData;

/// \struct tag_DeskConfig
/// \brief Desk configuration structure.
typedef struct tag_DeskConfig{
	uint8_t device_id;        ///< Configured device ID
	const char* device_name;  ///< Configured device name.
} DeskConfig;

/// @}
