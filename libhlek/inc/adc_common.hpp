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

/// \def ADCDEV_CONFIGURE
/// \brief Instructs to configure ADCDev (using /ref ADCDevConfig structure). Ongoing sampling will be stopped.
#define ADCDEV_CONFIGURE             128

/// \def ADCDEV_CLEAR
/// \brief Instructs to clear circular buffer. Ongoing sampling will NOT be stopped.
#define ADCDEV_CLEAR                 64

/// \def ADCDEV_STOP
/// \brief Stops ongoing sampling. To start ADCDEV all flags should be cleared.
#define ADCDEV_STOP                  32

/// \def ADCDEV_START
/// \brief Starts sampling. If ADC is currently sampling returns COMM_STATUS_FAIL.
/// \note This is equivalent to clear all flags.
#define ADCDEV_START                 0


/// \def ADCDEV_STATUS_STARTED
/// \brief Specifies "started" device state. This state indicates device is sampling data. If cleared device is not sampling
///        at the moment.
#define ADCDEV_STATUS_STARTED       ((uint16_t)(1))

/// \def ADCDEV_STATUS_UNSTOPPABLE
/// \brief Device is sampling continuously, until it is not explicitly stopped.
#define ADCDEV_STATUS_UNSTOPPABLE   ((uint16_t)(1 << 1))

/// \def ADCDEV_STATUS_TOO_FAST
/// \brief This is error flag that says device detected it's timer is not capable to handle specified data flow rate.
///        If this flag is set device will be stopped and data flow rate should be decreased.
#define ADCDEV_STATUS_TOO_FAST      ((uint16_t)(1 << 2))

/// \def ADCDEV_STATUS_SAMPLING
/// \brief This flag is reserved to calculate if timer interrupt overlaps with ADC/DMA interrupts.
/// \note This flag may change random for software part. Software should ignore this flag.
#define ADCDEV_STATUS_SAMPLING      ((uint16_t)(1 << 3))

#pragma pack(push, 1)
    /// \struct tag_ADCDevCommand
    /// \brief This structure describes command payload that is used to start sampling by ADCDev
    typedef struct tag_ADCDevCommand {
        uint16_t sample_count;      ///< Number of samples to be sampled. Ignored if #ADCDEV_UNSTOPPABLE is specified.
    } ADCDevCommand;

    /// \struct tag_ADCDevConfig
    /// \brief This structure describes ADCDev configuration.
    typedef struct tag_ADCDevConfig {
        uint16_t timer_prescaller;  ///< Timer prescaller value. If this value and tag_ADCDevCommand#timer_period are zero
        ///  conversions will follow each other without delay.

        uint16_t timer_period;      ///< Timer period value. If this value and tag_ADCDevCommand#timer_prescaller are zero
        ///  conversions will follow each other without delay.

        uint16_t measurements_per_sample; ///< Number of measurements per sample. Must be in range [1, n] where n is number of
                                          ///< measurements to average, as specified in json configuration file ("measurements_per_sample").

        uint8_t  channel_sampling[];   ///< Sampling time per channel (may be omitted by software, in this case default value is used).
    } ADCDevConfig;

#pragma pack(pop)
/// @}



/// \addtogroup group_adc_dev
/// @{

#define ADC_SampleTime_1Cycles5         0
#define ADC_SampleTime_7Cycles5         1
#define ADC_SampleTime_13Cycles5        2
#define ADC_SampleTime_28Cycles5        3
#define ADC_SampleTime_41Cycles5        4
#define ADC_SampleTime_55Cycles5        5
#define ADC_SampleTime_71Cycles5        6
#define ADC_SampleTime_239Cycles5       7

/// \struct tag_ADCInput
/// \brief Describes ADCDev input
typedef struct tag_ADCInput {
    const char*   in_name;                ///< Input name as specified in JSON configuration file
    const char*   adc_input;              ///< Channel name as specified in JSON configuration file (CMSIS channel name)
    const uint8_t default_sampling_time;  ///< Default sampling time
} ADCInput;

/// \struct tag_ADCConfig
/// \brief ADC configuration structure.
typedef struct tag_ADCConfig{
    uint8_t         dev_id;             ///< Device ID for ADCDev virtual device
    const char*     dev_name;           ///< Name of the ADCDev virtual device as given in JSON configuration file
    uint16_t        dev_buffer_len;     ///< Length of the ADCDev internal circular buffer
    uint16_t        input_count;        ///< Number of channels configured to be sampled by ADCDev virtual device
    uint16_t        measurements_per_sample; ///< Maximum number of measurements per sample ([1..n], where n is maximum number of measurements per sample)
    uint32_t        timer_freq;         ///< Timer clock frequency
    uint16_t        adc_maxval;         ///< Maximum value ADC may return
    const ADCInput* inputs;
} ADCConfig;

/// @}