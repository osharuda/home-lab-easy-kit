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
 *   \brief Generated include header of software part for ADC (Analog to Digital Convertor) device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>




// Device command options

/// \addtogroup group_adc_dev
/// @{

/// \def ADCDEV_UNSTOPPABLE
/// \brief Defines ADCDev command specific flag that instructs ADCDev to sample indefinitely, in this case tag_ADCDevCommand#sample_count
///        is ignored
#define ADCDEV_UNSTOPPABLE             128

/// \def ADCDEV_RESET_DATA
/// \brief Defines ADCDev command specific flag instruct to reset all accumulated data.
#define ADCDEV_RESET_DATA              64

/// \def ADCDEV_RESERVED
/// \brief Reserved, currently is not used
#define ADCDEV_RESERVED                32

#pragma pack(push, 1)
/// \struct tag_ADCDevCommand
/// \brief This structure describes command payload that is used to start sampling by ADCDev
typedef struct tag_ADCDevCommand {
    uint16_t sample_count;      ///< Number of samples to be sampled. Ignored if #ADCDEV_UNSTOPPABLE is specified.

    uint16_t timer_prescaller;  ///< Timer prescaller value. If this value and tag_ADCDevCommand#timer_period are zero
                                ///  conversions will follow each other without delay.

    uint16_t timer_period;      ///< Timer period value. If this value and tag_ADCDevCommand#timer_prescaller are zero
                                ///  conversions will follow each other without delay.
} ADCDevCommand;
#pragma pack(pop)
/// @}



/// \addtogroup group_adc_dev
/// @{

/// \struct tag_ADCInput
/// \brief Describes ADCDev input
typedef struct tag_ADCInput {
    const char* in_name;    ///< Input name as specified in JSON configuration file
    const char* adc_input;  ///< Channel name as specified in JSON configuration file (CMSIS channel name)
} ADCInput;

/// \struct tag_ADCConfig
/// \brief ADC configuration structure.
typedef struct tag_ADCConfig{
    uint8_t         dev_id;             ///< Device ID for ADCDev virtual device
    const char*     dev_name;           ///< Name of the ADCDev virtual device as given in JSON configuration file
    uint16_t        dev_buffer_len;     ///< Length of the ADCDev internal circular buffer
    uint16_t        input_count;        ///< Number of channels configured to be sampled by ADCDev virtual device
    uint32_t        timer_freq;         ///< Timer clock frequency
    double          vref;               ///< Default Vref+ voltage
    uint16_t        adc_maxval;         ///< Maximum value ADC may return
    const ADCInput* inputs;
} ADCConfig;

/// @}