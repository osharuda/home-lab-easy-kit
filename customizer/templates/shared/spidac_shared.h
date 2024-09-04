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

 /* --------------------> END OF THE TEMPLATE HEADER <-------------------- */

/// \addtogroup group_spidac
/// @{{

typedef enum {{
    SPIDAC_SAMPLE_FORMAT_DAC7611 = 1,
    SPIDAC_SAMPLE_FORMAT_DAC8550 = 2,
    SPIDAC_SAMPLE_FORMAT_DAC8564 = 3
}} SPIDAC_SAMPLE_FORMATS;

typedef enum {{
    START = 0x10,
    START_PERIOD = 0x20,
    STOP = 0x30,
    DATA_START = 0x40,
    DATA = 0x50,
    SETDEFAULT = 0x60
}} SPIDAC_COMMAND;

typedef enum {{
    SAMPLING = 1,   // SPI is sending data to DACs
    WAITING  = 2,   // Waiting for a timer to sampling DACs
    STOPPED  = 3,    // Fully stopped,
    STOPPED_ABNORMAL  = 4 // Fully and abnormally stopped (sampling rate is too fast)
}} SPIDAC_STATUS;

typedef enum {{
    LSB = 0,
    MSB = 1
}} SPIDAC_FRAME_FORMAT;

#pragma pack(push, 1)

/// \struct SPIDACChannelSamplingInfo
/// \brief Structure that describes per channel sampling.
struct SPIDACChannelSamplingInfo {{
    uint16_t phase_increment;       /// Sample increment in numbers
    uint16_t start_phase;           /// Start phase in samples numbers
    uint16_t loaded_samples_number; /// Number of samples loaded
}};

/// \struct SPIDACStartInfo
/// \brief Structure to be passed in order to start SPIDAC device
struct SPIDACStartInfo {{
    uint16_t prescaler;            /// Timer prescaler
    uint16_t period;                /// Timer period
    struct SPIDACChannelSamplingInfo channel_info[]; /// Sampling information for each channel
}};

/// \struct SPIDACStatus
/// \brief Structure that describes status of the SPIDAC device
struct SPIDACStatus {{
    volatile uint8_t status;             /// Describes status of the device.
    uint8_t repeat_count;
    struct SPIDACStartInfo start_info;
}};
#pragma pack(pop)

/// @}}
