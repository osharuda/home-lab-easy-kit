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
 *   \brief Generated include header of firmware part for info device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define INFO_DEVICE_ENABLED 1

/// \addtogroup group_info_dev
/// @{{

{__INFO_SHARED_HEADER__}

/// \def INFO_ADDR
/// \brief Device id of the INFODev virtual device
#define INFO_ADDR {__DEVICE_ID__}

/// \def INFO_UUID
/// \brief UUID that represents hash sum of the JSON configuration file
#define INFO_UUID {{ {__INFO_UUID__} }}

/// \def INFO_UUID_LEN
/// \brief Length of the #INFO_UUID
#define INFO_UUID_LEN {__INFO_UUID_LEN__}

/// @}}