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


// Device command options

/// \addtogroup group_adc_dev
/// @{{

/// \def ADCDEV_START
/// \brief Defines ADCDev "Start" command. This command is sent via device specific
///        part of the command byte
#define ADCDEV_START                  (1 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def ADCDEV_STOP
/// \brief Defines ADCDev "Stop" command. This command is sent via device specific
///        part of the command byte
#define ADCDEV_STOP                   (2 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def ADCDEV_RESET
/// \brief Defines ADCDev "Reset" command. This command is sent via device specific
///        part of the command byte
/// \note Device must be in stopped state to execute this command.
#define ADCDEV_RESET                  (3 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def ADCDEV_CONFIGURE
/// \brief Defines ADCDev "Configure" command. This command is sent via device specific
///        part of the command byte
/// \note Device must be in stopped state to execute this command.
#define ADCDEV_CONFIGURE              (4 << COMM_CMDBYTE_SPECIFIC_OFFSET)


/// \def ADCDEV_STATUS_STARTED
/// \brief Specifies "started" device state. This state indicates device is sampling data. If cleared device is not sampling
///        at the moment.
#define ADCDEV_STATUS_STARTED       ((uint16_t)(1))

/// \def ADCDEV_STATUS_UNSTOPPABLE
/// \brief Device is sampling continuously, until it is not explicitly stopped or buffer overflow.
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
    /// \struct ADCDevCommand
    /// \brief This structure describes command payload that is used to start sampling by ADCDev
    struct ADCDevCommand {{
        uint16_t sample_count;      ///< Number of samples to be sampled. Ignored if #ADCDEV_UNSTOPPABLE is specified.
    }};

    /// \struct ADCDevConfig
    /// \brief This structure describes ADCDev configuration.
    struct ADCDevConfig {{
        uint16_t timer_prescaller;  ///< Timer prescaller value. If this value and tag_ADCDevCommand#timer_period are zero
        ///  conversions will follow each other without delay.

        uint16_t timer_period;      ///< Timer period value. If this value and tag_ADCDevCommand#timer_prescaller are zero
        ///  conversions will follow each other without delay.

        uint16_t measurements_per_sample; ///< Number of measurements per sample. Must be in range [1, n] where n is number of
                                          ///< measurements to average, as specified in json configuration file ("measurements_per_sample").

        uint8_t  channel_sampling[];   ///< Sampling time per channel (may be omitted by software, in this case default value is used).
    }};

#pragma pack(pop)
/// @}}

