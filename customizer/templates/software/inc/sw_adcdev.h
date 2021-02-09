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

#define ADCDEV_DEVICE_ENABLED 1

{__ADC_SHARED_HEADER__}

/// \addtogroup group_adc_dev
/// @{{

/// \struct tag_ADCInput
/// \brief Describes ADCDev input
typedef struct tag_ADCInput {{
    const char* in_name;    ///< Input name as specified in JSON configuration file
    const char* adc_input;  ///< Channel name as specified in JSON configuration file (CMSIS channel name)
}} ADCInput;

/// \struct tag_ADCDevInstance
/// \brief Describes ADCDev configuration
typedef struct tag_ADCDevInstance {{
	uint8_t         dev_id;             ///< Device ID for ADCDev virtual device
	const char*     dev_name;           ///< Name of the ADCDev virtual device as given in JSON configuration file
	uint16_t        dev_buffer_len;     ///< Length of the ADCDev internal circular buffer
	uint16_t        input_count;        ///< Number of channels configured to be sampled by ADCDev virtual device
	uint32_t        timer_freq;         ///< Timer clock frequency
	double          vref;               ///< Default Vref+ voltage
	uint16_t        adc_maxval;         ///< Maximum value ADC may return
	const ADCInput* inputs;
}} ADCDevInstance;

/// \def ADCDEV_DEVICE_COUNT
/// \brief Number of ADCDev devices being used
#define ADCDEV_DEVICE_COUNT {__ADCDEV_DEVICE_COUNT__}

/// \def ADCDEV_SW_CHANNELS
/// \brief Defines global arrays with channels being used by ADCDev virtual devices
#define ADCDEV_SW_CHANNELS {__ADCDEV_SW_CHANNELS__}

/// \def ADCDEV_SW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for ADCDev virtual devices
#define ADCDEV_SW_DEV_DESCRIPTOR {{  {__ADCDEV_SW_DEV_DESCRIPTOR__} }}

/// @}}