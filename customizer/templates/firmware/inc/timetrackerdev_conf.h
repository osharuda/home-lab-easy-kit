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
 *   \brief Generated include header of firmware part for TimeTrackerDev device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include "i2c_proto.h"

#define TIMETRACKERDEV_DEVICE_ENABLED 1
/// \addtogroup group_timetrackerdev
/// @{{

/// \def TIMETRACKERDEV_DEVICE_COUNT
/// \brief Number of TimeTrackerDev devices being used
#define TIMETRACKERDEV_DEVICE_COUNT {__TIMETRACKERDEV_DEVICE_COUNT__}

/// \def TIMETRACKERDEV_DEVICE_BUFFER_TYPE
/// \brief Type of the buffer used by device
#define TIMETRACKERDEV_DEVICE_BUFFER_TYPE DEV_CIRCULAR_BUFFER

/// \def TIMETRACKERDEV_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for TimeTrackerDev virtual devices
#define TIMETRACKERDEV_FW_DEV_DESCRIPTOR {{  {__TIMETRACKERDEV_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__TIMETRACKERDEV_SHARED_HEADER__}

/// \addtogroup group_timetrackerdev
/// @{{

/// \def TIMETRACKERDEV_FW_BUFFERS
/// \brief Defines memory blocks used for TimeTrackerDev circular buffers as data storage
#define TIMETRACKERDEV_FW_BUFFERS {__TIMETRACKERDEV_FW_BUFFERS__}

/// @}}




