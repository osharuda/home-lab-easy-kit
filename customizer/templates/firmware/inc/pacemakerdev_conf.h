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
 *   \brief Generated include header of firmware part for PaceMakerDev device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define PACEMAKERDEV_DEVICE_ENABLED 1
/// \addtogroup group_pacemakerdev
/// @{{

/// \def PACEMAKERDEV_DEVICE_COUNT
/// \brief Number of PaceMakerDev devices being used
#define PACEMAKERDEV_DEVICE_COUNT {__PACEMAKERDEV_DEVICE_COUNT__}

/// \def PACEMAKERDEV_DEVICE_BUFFER_TYPE
/// \brief Type of the buffer used by device
#define PACEMAKERDEV_DEVICE_BUFFER_TYPE DEV_LINIAR_BUFFER

/// \def PACEMAKERDEV_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for PaceMakerDev virtual devices
#define PACEMAKERDEV_FW_DEV_DESCRIPTOR {{  {__PACEMAKERDEV_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__PACEMAKERDEV_SHARED_HEADER__}

/// \addtogroup group_pacemakerdev
/// @{{

/// \def PACEMAKERDEV_FW_BUFFERS
/// \brief Defines memory blocks used for PaceMakerDev circular buffers as data storage
#define PACEMAKERDEV_FW_BUFFERS {__PACEMAKERDEV_FW_BUFFERS__}

/// \def PACEMAKERDEV_FW_SET_GPIO_HEADERS
/// \brief Defines headers for set signals functions which set actual gpio
#define PACEMAKERDEV_FW_SET_GPIO_HEADERS   {__PACEMAKERDEV_FW_SET_GPIO_HEADERS__}

/// \def PACEMAKERDEV_FW_SET_GPIO_FUNCTIONS
/// \brief Defines macro with functions to set signals (gpio)
#define PACEMAKERDEV_FW_SET_GPIO_FUNCTIONS {__PACEMAKERDEV_FW_SET_GPIO_FUNCTIONS__}

/// \def PACEMAKERDEV_FW_INIT_GPIO_HEADERS
/// \brief Defines headers for initialization signals functions which set actual gpio
#define PACEMAKERDEV_FW_INIT_GPIO_HEADERS   {__PACEMAKERDEV_FW_INIT_GPIO_HEADERS__}

/// \def PACEMAKERDEV_FW_INIT_GPIO_FUNCTIONS
/// \brief Defines macro with functions to set signals (gpio)
#define PACEMAKERDEV_FW_INIT_GPIO_FUNCTIONS {__PACEMAKERDEV_FW_INIT_GPIO_FUNCTIONS__}

/// \def PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS
/// \brief Defines PaceMakerDev Main TIMER irq handlers
#define PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS {__PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS__}

/// \def PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS
/// \brief Defines PaceMakerDev Internal TIMER irq handlers
#define PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS {__PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS__}

/// @}}




