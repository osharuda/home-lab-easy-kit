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
 *   \brief Generated include header of software part for SPWM (Software Pulse Width Modulation) device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#include <cstddef>

#define SPWM_DEVICE_ENABLED     1

/// \addtogroup group_spwm_dev
/// @{{

/// \def SPWM_DEVICE_NAME
/// \brief SPWMDev virtual device name as configured in JSON configuration file.
#define SPWM_DEVICE_NAME "{__SPWM_DEVICE_NAME__}"

{__SPWM_SHARED_HEADER__}

/// \defgroup group_spwm_dev_channel_indexes SPWMDev channel indexes
/// @{{
{__SPWM_CHANNEL_INDEXES__}
/// @}}

/// \struct tag_SPWM_SW_DESCRIPTOR
/// \brief SPWM channel descriptor
typedef struct tag_SPWM_SW_DESCRIPTOR {{
    size_t  port_index;        ///< Index of the port used
    size_t  pin_number;        ///< Pin number being used
    bool    def_val;           ///< Default value (may not be intermediate state, either on or off)
    const char* channel_name;  ///< Name of the channel
}} SPWM_SW_DESCRIPTOR;

/// \def SPWM_SW_DESCRIPTION
/// \brief Defines all SPWMDev channels
#define SPWM_SW_DESCRIPTION   {{ {__SPWM_SW_DESCRIPTION__} }}

/// @}}