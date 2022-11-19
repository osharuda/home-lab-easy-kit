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
 *   \brief Generated include header of firmware part for GPIO device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define GPIODEV_DEVICE_ENABLED 1

/// \addtogroup group_gpio_dev
/// @{{

{__GPIO_SHARED_HEADER__}

/// \def GPIODEV_ADDR
/// \brief GPIODev device id
#define GPIODEV_ADDR						{__DEVICE_ID__}

/// \def GPIO_PIN_COUNT
/// \brief Number of pins configured
#define GPIO_PIN_COUNT         {__GPIO_PIN_COUNT__}

/// \defgroup group_gpio_dev_pin_indexes GPIO pin indexes
/// @{{
{__C_GPIO_DEV_PINS_DECLARATION__}
/// @}}

#define GPIO_DESCRIPTOR {{ {__GPIO_FW_DEV_DESCRIPTION__} }}
/// @}}