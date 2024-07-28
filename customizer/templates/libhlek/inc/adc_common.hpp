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
#include "i2c_proto.h"

{__ADC_SHARED_HEADER__}

/// \addtogroup group_adc_dev
/// @{{

#define ADC_SampleTime_1Cycles5         0
#define ADC_SampleTime_7Cycles5         1
#define ADC_SampleTime_13Cycles5        2
#define ADC_SampleTime_28Cycles5        3
#define ADC_SampleTime_41Cycles5        4
#define ADC_SampleTime_55Cycles5        5
#define ADC_SampleTime_71Cycles5        6
#define ADC_SampleTime_239Cycles5       7

/// \struct ADCInput
/// \brief Describes ADCDev input
struct ADCInput {{
    const char*   in_name;                ///< Input name as specified in JSON configuration file
    const char*   adc_input;              ///< Channel name as specified in JSON configuration file (CMSIS channel name)
    const uint8_t default_sampling_time;  ///< Default sampling time
}};

/// \struct ADCConfig
/// \brief ADC configuration structure.
struct ADCConfig {{
    uint8_t         dev_id;             ///< Device ID for ADCDev virtual device
    const char*     dev_name;           ///< Name of the ADCDev virtual device as given in JSON configuration file
    uint16_t        dev_buffer_len;     ///< Length of the ADCDev internal circular buffer
    uint16_t        input_count;        ///< Number of channels configured to be sampled by ADCDev virtual device
    uint16_t        measurements_per_sample; ///< Maximum number of measurements per sample ([1..n], where n is maximum number of measurements per sample)
    uint32_t        timer_freq;         ///< Timer clock frequency
    uint16_t        adc_maxval;         ///< Maximum value ADC may return
    const ADCInput* inputs;
}};

/// @}}