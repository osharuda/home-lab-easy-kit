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
 *   \brief Generated include header of software part for PaceMakerDev device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>

#include <cstddef>

#include "ekit_error.hpp"

{__PACEMAKERDEV_SHARED_HEADER__}

/// \addtogroup group_pacemakerdev
/// @{{


/// \struct tag_PaceMakerSignal
/// \brief PaceMakerDev signal descriptor.
typedef struct tag_PaceMakerSignal {{
    size_t      signal_mask;   ///< Bitmask which defines state of the signals state
    int         default_value; ///< Default value (may not be intermediate state, either 0 or 1)
    const char* name;          ///< Name of the signal
}} PaceMakerSignal;

/// \struct PaceMakerDevConfig
/// \brief PaceMakerDev configuration structure.
typedef struct tag_PaceMakerDevConfig{{
    uint8_t         dev_id;             ///< Device ID for PaceMakerDev virtual device.
    const char*     dev_name;           ///< Name of the PaceMakerDev virtual device as given in JSON configuration file.
    size_t          dev_buffer_len;     ///< Length of the PaceMakerDev internal buffer, 0 if no buffer is used.
    uint32_t        main_timer_freq;    ///< Main timer frequency.
    uint32_t        internal_timer_freq;///< Internal timer frequency.
    size_t          signals_number;     ///< Number of signals being used.
    uint32_t        default_signals;    ///< Number of signals being used.
    uint32_t        max_samples;        ///< Maximum number of the samples allowed by buffer capacity.
}} PaceMakerDevConfig;

/// @}}
