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
 *   \brief Generated include header of software part for GPIO device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>


/// \addtogroup group_gpio_dev
/// @{




/// \def GPIO_DEV_INPUT
/// \brief Defines input pin.
#define GPIO_DEV_INPUT         1

/// \def GPIO_DEV_OUTPUT
/// \brief Defines output pin.
#define GPIO_DEV_OUTPUT        0


/// \struct tag_GPIO_descr
/// \brief Describes configured GPIO line.
typedef struct tag_GPIOPin {
    uint8_t pin_id;         ///< Pin identifier or pin index. Must be in range [0 ... #GPIO_PIN_COUNT). Note, there are constants
                            ///  defined for each pin_id here @ref group_gpio_dev_pin_indexes.
    uint8_t pin_type;       ///< Type of the pin. Must be either #GPIO_DEV_INPUT or #GPIO_DEV_OUTPUT.
    uint8_t default_val;    ///< Default pin value. Is meaningful for outputs only.
    const char* pin_name;   ///< Name of the pin given in JSON configuration file.
} GPIOPin;

/// \struct tag_GPIOConfig
/// \brief GPIO configuration structure.
typedef struct tag_GPIOConfig{
	uint8_t device_id;        ///< Configured device ID
	const char* device_name;  ///< Configured device name.
	size_t pin_number;
	const GPIOPin* pins;
} GPIOConfig;

/// @}
