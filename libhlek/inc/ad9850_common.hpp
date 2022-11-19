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
 *   \brief Generated include header of software part for AD9850Dev device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>



/// \addtogroup group_ad9850dev
/// @{

/// \def AD9850DEV_RESET
/// \brief Instructs to reset AD9850 prior to command execution
#define AD9850DEV_RESET                 128

/// \def AD9850DEV_RESERVED_1
/// \brief Defines AD9850Dev command specific flag 1
#define AD9850DEV_RESERVED_1             64

/// \def AD9850DEV_RESERVED_0
/// \brief Defines AD9850Dev command specific flag 0
#define AD9850DEV_RESERVED_0             32

#pragma pack(push, 1)

/// \struct tag_AD9850Command
/// \brief Structure that describes status of the AD9850Dev
/// \note Actual phase is 2*Pi / AD9850Command#phase
/// \note Actual frequency is F_clk * frequency_word / 4294967295.
typedef struct tag_AD9850Command {
    union {
        struct {
            /// \brief Phase to set. Note that phase may be set with accuracy of 11.25 degree
            uint8_t phase: 5;

            /// \brief If 1 power down mode is activated otherwise normal operation.
            uint8_t power_down: 1;

            /// \brief Documentation doesn't state much about this bits. It looks like it is used for the
            /// testing on the factory. This value shouldn't be equal 1 or 2. Therefor it is set to
            /// 0 by virtual device forcibly.
            uint8_t control: 2;

            uint8_t : 0;
        };
        /// \brief W0 may be used to access a structure as a single byte.
        uint8_t W0;
    };

    /// \brief Bits 31-24 of the frequency 32-bit word.
    uint8_t freq_b31_b24;

    /// \brief Bits 23-16 of the frequency 32-bit word.
    uint8_t freq_b23_b16;

    /// \brief Bits 15-8 of the frequency 32-bit word.
    uint8_t freq_b15_b8;

    /// \brief Bits 7-0 of the frequency 32-bit word.
    uint8_t freq_b7_b0;

} AD9850Command;

#pragma pack(pop)
/// @}


/// \addtogroup group_ad9850dev
/// @{

/// \struct AD9850Config
/// \brief AD9850 configuration structure.
typedef struct tag_AD9850Config{
    uint8_t         dev_id;             ///< Device ID for AD9850Dev virtual device
    const char*     dev_name;           ///< Name of the AD9850Dev virtual device as given in JSON configuration file
    uint32_t        clock_frequency;    ///< Default clock frequency (HZ)
} AD9850Config;

/// @}