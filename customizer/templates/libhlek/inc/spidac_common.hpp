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
 *   \brief Generated include header of software part for SPIDAC device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>

#include <cstddef>

{__SPIDAC_SHARED_HEADER__}

/// \addtogroup group_spidac
/// @{{

/// \struct tag_SPIDACChannelDescriptor
/// \brief Describes SPIDAC channel
typedef struct tag_SPIDACChannelDescriptor {{
    const char*     name;           ///< Name of the channel
    double          min_value;      ///< Minimum value
    double          max_value;      ///< Maximum value
    double          default_value;  ///< Default value
}} SPIDACChannelDescriptor;

/// \struct tag_SPIDACConfig
/// \brief Describes SPIDAC configuration
typedef struct tag_SPIDACConfig {{
    const char*     dev_name;           ///< Name of the SPIDAC virtual device as given in JSON configuration file
    size_t          dev_buffer_len;     ///< Length of the SPIDAC internal buffer
    size_t          frames_per_sample;  ///< Amount of sample per frame
    size_t          frame_size;         ///< Size of the SPI frame, in bytes
    uint8_t         dev_id;             ///< Device ID for SPIDAC virtual device
    SPIDAC_FRAME_FORMAT frame_format;   ///< Frame format
    size_t          channel_count;      ///< Number of the channels
    size_t          max_sample_count;   ///< Maximum number of samples per channel
    size_t          bits_per_sample;    ///< Number of bits per one sample
    uint32_t        timer_freq;         ///< Timer clock frequency
    const SPIDACChannelDescriptor* channels;  ///< Channels description
}} SPIDACConfig;

/// @}}