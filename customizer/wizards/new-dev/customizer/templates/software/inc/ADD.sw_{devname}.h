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
 *   \brief Generated include header of software part for {DevName} device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define {DEVNAME}_DEVICE_ENABLED 1

{__{DEVNAME}_SHARED_HEADER__}

/// \addtogroup group_{devname}
/// @{{

/// \struct tag_{DevName}Instance
/// \brief Describes {DevName} configuration
typedef struct tag_{DevName}Instance {{
	uint8_t         dev_id;             ///< Device ID for {DevName} virtual device
	const char*     dev_name;           ///< Name of the {DevName} virtual device as given in JSON configuration file
	uint16_t        dev_buffer_len;     ///< Length of the {DevName} internal buffer
}} {DevName}Instance;

typedef {DevName}Instance* P{DevName}Instance;

/// \def {DEVNAME}_SW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for {DevName} virtual devices.
#define {DEVNAME}_SW_DEV_DESCRIPTOR {{  {__{DEVNAME}_SW_DEV_DESCRIPTOR__} }}

/// @}}