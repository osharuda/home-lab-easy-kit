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

#include <stdint.h>
#include <cstddef>

/// \addtogroup group_spwm_dev
/// @{



#pragma pack(push, 1)

/// \struct tag_PWM_ENTRY
/// \brief Defines single PWM entry
typedef struct tag_PWM_ENTRY {
	uint16_t n_periods; ///< Amount of timer periods until the next PWM entry. This value is put in ARR timer register.
    uint16_t data[];    ///< PORT values to be set.
} PWM_ENTRY;
typedef volatile PWM_ENTRY* PPWM_ENTRY;
#pragma pack(pop)

#define PWM_ENTRY_SIZE(n_ports) (sizeof(PWM_ENTRY) + (n_ports)*sizeof(uint16_t))
#define GET_PWM_ENTRY_BY_INDEX(entry_array, index, entry_size) ((PPWM_ENTRY)( (uint8_t*)(entry_array) + (entry_size)*(index) ))




/// \struct tag_SPWMChannel
/// \brief SPWM channel configuration structure.
typedef struct tag_SPWMChannel{
    size_t  port_index;        ///< Index of the port used
    size_t  pin_number;        ///< Pin number being used
    bool    def_val;           ///< Default value (may not be intermediate state, either on or off)
    const char* channel_name;  ///< Name of the channel
} SPWMChannel;

/// \struct tag_SPWMConfig
/// \brief SPWM configuration structure.
typedef struct tag_SPWMConfig{
	uint8_t dev_id;            ///< Configured device ID.
	const char* dev_name;      ///< Configured device name.
	double default_freq;       ///< Default frequency.
	uint32_t prescaller;       ///< Prescaller value.
    size_t  port_number;       ///< Number of the ports.
    size_t  channel_count;     ///< Index of the port used.
    const SPWMChannel* channels;
} SPWMConfig;

/// @}