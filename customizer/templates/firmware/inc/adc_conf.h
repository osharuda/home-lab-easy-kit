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
 *   \brief Generated include header of firmware part for ADC (Analog to Digital Convertor) device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define ADCDEV_DEVICE_ENABLED 1
/// \addtogroup group_adc_dev
/// @{{

/// \def ADCDEV_DEVICE_COUNT
/// \brief Number of ADCDev devices being used
#define ADCDEV_DEVICE_COUNT {__ADCDEV_DEVICE_COUNT__}

/// \def ADCDEV_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for ADCDev virtual devices
#define ADCDEV_FW_DEV_DESCRIPTOR {{  {__ADCDEV_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__ADC_SHARED_HEADER__}

/// \addtogroup group_adc_dev
/// @{{

/// \def ADCDEV_FW_BUFFERS
/// \brief Defines memory blocks used for ADCDev circular buffers as data storage
#define ADCDEV_FW_BUFFERS {__ADCDEV_FW_BUFFERS__}

/// \def ADCDEV_FW_MEASUREMENT_BUFFERS
/// \brief Defines memory blocks used to keep data (measurements) to be averaged later into samples
#define ADCDEV_FW_MEASUREMENT_BUFFERS {__ADCDEV_FW_MEASUREMENT_BUFFERS__}

/// \def ADCDEV_FW_SAMPLE_TIME_BUFFERS
/// \brief Defines memory blocks used to store sample time for each channel
#define ADCDEV_FW_SAMPLE_TIME_BUFFERS {__ADCDEV_FW_SAMPLE_TIME_BUFFERS__}

/// \def ADCDEV_FW_ACCUMULATOR_BUFFERS
/// \brief Defines memory used to accumulate measurements (sum) and divide later to get average value.
#define ADCDEV_FW_ACCUMULATOR_BUFFERS {__ADCDEV_FW_ACCUMULATOR_BUFFERS__}

/// \def ADCDEV_FW_CHANNELS
/// \brief Defines global arrays with channels being used by ADCDev virtual devices
#define ADCDEV_FW_CHANNELS {__ADCDEV_FW_CHANNELS__}

/// \def ADCDEV_FW_ADC_IRQ_HANDLERS
/// \brief Defines ADCDev ADC irq handlers
#define ADCDEV_FW_ADC_IRQ_HANDLERS {__ADCDEV_FW_ADC_IRQ_HANDLERS__}

/// \def ADCDEV_FW_TIMER_IRQ_HANDLERS
/// \brief Defines ADCDev TIMER irq handlers
#define ADCDEV_FW_TIMER_IRQ_HANDLERS {__ADCDEV_FW_TIMER_IRQ_HANDLERS__}

/// \def ADCDEV_FW_DMA_IRQ_HANDLERS
/// \brief Defines ADCDev DMA irq handlers
#define ADCDEV_FW_DMA_IRQ_HANDLERS {__ADCDEV_FW_DMA_IRQ_HANDLERS__}

/// @}}




