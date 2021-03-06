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
 *   \brief ADC (Analog to Digital Convertor) device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef ADCDEV_DEVICE_ENABLED

/// \defgroup group_adc_dev ADCDev
/// \brief Analogue to Digital Converter support
/// @{
/// \page page_adc_dev
/// \tableofcontents
///
/// \section sect_adc_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

#pragma pack(push, 1)

/// \typedef ADCStartSamplingFunc
/// \brief Defines function to be used in order to start sampling
typedef void (*ADCStartSamplingFunc)(volatile void*);

/// \struct tag_ADCDevFwPrivData
/// \brief Structure that describes private ADCDev values
typedef struct tag_ADCDevFwPrivData {
    uint8_t started;                               ///< Indicates if sampling is started; non-zero sampling is started,
                                                   ///  0 - sampling is not started.

    uint8_t stop;                                  ///< Non-zero value indicates stop is requested by software.

    uint8_t unstoppable;                           ///< Non-zero indicates that tag_ADCDevCommand#sample_count should be
                                                   ///  ignored sampling must go on indefinitely.

    uint8_t current_channel;                       ///< Currently sampled channel in interrupt mode

    volatile uint16_t* dest_buffer;                ///< Buffer to store values sampled in interrupt mode

    uint16_t samples_left;                         ///< Amount of samples left to sample

    ADCStartSamplingFunc start_sampling_func;      ///< Function pointer that starts ADC sampling. Possible values are:
                                                   ///  - #adc_start_int_mode() to start conversion now in interrupt mode.
                                                   ///  - #adc_start_timer() if conversion must be started by timer.
                                                   ///  - #adc_start_dma_mode() to start conversion now in DMA mode.

    uint16_t prescaller;                           ///< Timer prescaller value. If this value and tag_ADCDevFwPrivData#period are zero
                                                   ///  conversions will follow each other without delay.

    uint16_t period;                               ///< Timer period value. If this value and tag_ADCDevFwPrivData#prescaller are zero
                                                   ///  conversions will follow each other without delay.
} ADCDevFwPrivData;


/// \struct tag_ADCDevFwChannel
/// \brief Structure that describes ADC channel
typedef struct tag_ADCDevFwChannel {
        uint8_t channel;        ///< Channel number (ADC_Channel_XXX values from CMSIS library)
        uint8_t sample_time;    ///< Sampling time (ADC_SampleTime_XXX values from CMSIS library)
        GPIO_TypeDef* port;     ///< Port being used by corresponding GPIO pin
        uint16_t      pin;      ///< GPIO pin (bitmask)
} ADCDevFwChannel;

/// \struct tag_ADCDevFwInstance
/// \brief Structure that describes ADCDev virtual device
typedef struct tag_ADCDevFwInstance {
        uint8_t                     dev_id;             ///< Device ID for ADCDev virtual device

        uint8_t                     input_count;        ///< Number of ADC channels used

        ADC_TypeDef*                adc;                ///< ADC being used

        uint32_t                    adc_dr_address;     ///< ADC data register address

        DMA_Channel_TypeDef*        dma_channel;        ///< DMA channel being used (0 in interrupt mode)

        DMA_TypeDef *               dma;                ///< DMA being used  (0 in interrupt mode)

        uint32_t                    dma_it;             ///< DMA transfer complete interrupt being used (DMA1_IT_XXX from CMSIS library)

        IRQn_Type                   timer_irqn;         ///< Timer interrupt number

        IRQn_Type                   scan_complete_irqn; ///< Either DMA transfer complete or ADC complete interrupt number

        uint16_t                    buffer_size;        ///< Circular buffer size

        volatile uint8_t*           buffer;             ///< Memory block used for circular buffer as storage

        uint16_t                    sample_block_size;  ///< Amount of bytes used for sample buffer

        TIM_TypeDef*                timer;              ///< Timer being used

        volatile ADCDevFwChannel*   channels;           ///< Pointer to #tag_ADCDevFwChannel channel description array

        volatile DeviceContext      dev_ctx;            ///< Virtual device context

        volatile CircBuffer         circ_buffer;        ///< Circular buffer control structure

        volatile ADCDevFwPrivData   privdata;           ///< Private data used by this ADCDev device
} ADCDevFwInstance;

#pragma pack(pop)

/// \def ADC_DMA_MODE
/// \brief This macro is used to check if running in DMA mode
/// \param dev - pointer to #ADCDevFwInstance structure
#define ADC_DMA_MODE(dev) ((dev)->dma!=0)

/// \def ADC_INT_MODE
/// \brief This macro is used to check if running in interrupt mode
/// \param dev - pointer to #ADCDevFwInstance structure
#define ADC_INT_MODE(dev) ((dev)->dma==0)

/// \brief Initializes all ADCDev virtual devices
void init_adc();

/// \brief #ON_COMMAND callback for all ADCDev devices
/// \param cmd_byte - command byte received from software. Corresponds to tag_CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void adc_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all ADCDev devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void adc_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif