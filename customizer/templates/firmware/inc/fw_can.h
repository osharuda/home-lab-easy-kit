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
 *   \brief Generated include header of firmware part for Can device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define CAN_DEVICE_ENABLED 1
/// \addtogroup group_can
/// @{{

/// \def CAN_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for Can virtual devices
#define CAN_FW_DEV_DESCRIPTOR {{  {__CAN_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__CAN_SHARED_HEADER__}

/// \addtogroup group_can
/// @{{

/// \def CAN_FW_BUFFERS
/// \brief Defines memory blocks used for Can circular buffers as data storage
#define CAN_FW_BUFFERS {__CAN_FW_BUFFERS__}

/// \def CAN_FW_IRQ_HANDLERS
/// \brief Defines Can irq handlers
#define CAN_FW_IRQ_HANDLERS {__CAN_FW_IRQ_HANDLERS__}

/// \def CAN_POLLING_EVERY_US
/// \brief Polling period for recovery from BUS-OFF state.
#define CAN_POLLING_EVERY_US   250000

/// @}}




