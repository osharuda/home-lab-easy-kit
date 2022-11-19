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
 *   \brief Generated include header of firmware part for {DevName} device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define {DEVNAME}_DEVICE_ENABLED 1
/// \addtogroup group_{devname}
/// @{{

/// \def {DEVNAME}_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for {DevName} virtual devices
#define {DEVNAME}_FW_DEV_DESCRIPTOR {{  {__{DEVNAME}_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__{DEVNAME}_SHARED_HEADER__}

/// \addtogroup group_{devname}
/// @{{

/// \def {DEVNAME}_FW_BUFFERS
/// \brief Defines memory blocks used for {DevName} circular buffers as data storage
#define {DEVNAME}_FW_BUFFERS {__{DEVNAME}_FW_BUFFERS__}

/// @}}




