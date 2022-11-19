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
 *   \brief Generated include header of firmware part for AD9850Dev device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define AD9850DEV_DEVICE_ENABLED 1
/// \addtogroup group_ad9850dev
/// @{{

/// \def AD9850DEV_DEVICE_COUNT
/// \brief Number of AD9850Dev devices being used
#define AD9850DEV_DEVICE_COUNT {__AD9850_DEVICE_COUNT__}

/// \def AD9850DEV_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for AD9850Dev virtual devices
#define AD9850DEV_FW_DEV_DESCRIPTOR {{  {__AD9850_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__AD9850_SHARED_HEADER__}

/// \addtogroup group_ad9850dev
/// @{{
/// \def AD9850DEV_FW_SET_DATA_HEADERS
/// \brief Defines headers for set of functions which set data on data bus
#define AD9850DEV_FW_SET_DATA_HEADERS   {__AD9850_FW_SET_DATA_HEADERS__}

/// \def AD9850DEV_FW_SET_DATA_FUNCTIONS
/// \brief Defines macro with functions to set data on data bus
#define AD9850DEV_FW_SET_DATA_FUNCTIONS {__AD9850_FW_SET_DATA_FUNCTIONS__}

/// @}}




