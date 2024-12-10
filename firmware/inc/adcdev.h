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

#include "circbuffer.h"

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

/// \typedef ADCStartSamplingFunc
/// \brief Defines function to be used in order to start sampling
typedef void (*ADCStartSamplingFunc)(volatile void*, volatile void*);
typedef void (*ADCResetFunc)(volatile void*, volatile void*);

#define ADC_CR2_FLAG_SWSTART       ((uint32_t)0x00400000)
#define ADC_CR2_FLAG_EXT_TRIG      ((uint32_t)0x00100000)
#define ADC_CR2_FLAG_DMA           ((uint32_t)0x00000100)
#define ADC_CR2_FLAG_CONT          ((uint32_t)0x00000002)
#define ADC_CR2_FLAG_ADON          ((uint32_t)0x00000001)

#define ADC_CR1_FLAG_SCAN          ((uint32_t)0x00000100)

/// \struct ADCDevFwPrivData
/// \brief Structure that describes private ADCDev values
struct __attribute__ ((aligned)) ADCDevFwPrivData {
    ADCStartSamplingFunc adc_continue_sampling_ptr;  ///< Function pointer that schedules (continues) next ADC sampling.

    ADCResetFunc adc_hw_reset_ptr;                ///< Pointer to the function that reset adc hardware

    volatile uint16_t* current_measurement;       ///< Pointer to the current measurement

    volatile uint16_t* end_measurement;           ///< Pointer to the next after last measurements.

    uint32_t interrupt_priority;                  ///< Interrupt priority. One of the IRQ_PRIORITY_ADC_XXX values.

    uint32_t measurement_count;                   ///< Number of measurements that should be made for all channels per one sample.

    uint16_t status;                              ///< ADCDev status

    uint16_t comm_status;                         ///< Copy of the status for synchronization with I2C communication.

    uint16_t measurement_per_sample;              ///< Number of measurements per sample

    uint16_t samples_left;                        ///< Amount of samples left to sample

    uint16_t prescaller;                          ///< Timer prescaller value. If this value and tag_ADCDevFwPrivData#period are zero
                                                  ///  conversions will follow each other without delay.

    uint16_t period;                              ///< Timer period value. If this value and tag_ADCDevFwPrivData#prescaller are zero
                                                  ///  conversions will follow each other without delay.
};


/// \struct ADCDevFwChannel
/// \brief Structure that describes ADC channel
struct ADCDevFwChannel {
        GPIO_TypeDef* port;           ///< Port being used by corresponding GPIO pin
        uint16_t      pin;            ///< GPIO pin (bitmask)
        uint8_t       channel;        ///< Channel number (ADC_Channel_XXX values from CMSIS library)
        uint8_t       sample_time;    ///< Sampling time (ADC_SampleTime_XXX values from CMSIS library)
} ADCDevFwChannel;

/// \struct ADCDevFwInstance
/// \brief Structure that describes ADCDev virtual device
struct __attribute__ ((aligned)) ADCDevFwInstance {
        struct DeviceContext      dev_ctx __attribute__ ((aligned)); ///< Virtual device context

        struct CircBuffer         circ_buffer;        ///< Circular buffer control structure

        struct ADCDevFwPrivData   privdata;           ///< Private data used by this ADCDev device

        struct ADCDevFwChannel*   channels;           ///< Pointer to #tag_ADCDevFwChannel channel description array

        struct TimerData  timer_data;      ///< Timer preinit data.

        uint16_t*                 measurement_buffer; ///< Buffer for values to be measured (and averaged later).

        uint8_t*                  sample_time_buffer; ///< Sample time buffer, used to keep current sample time settings.

        uint32_t*                 accumulator_buffer; ///< Accumulator buffer.

        uint8_t*                  buffer;             ///< Memory block used for circular buffer as storage

        ADC_TypeDef*              adc;                ///< ADC being used

        uint32_t                  adc_dr_address;     ///< ADC data register address

        DMA_Channel_TypeDef*      dma_channel;        ///< DMA channel being used (0 in interrupt mode)

        DMA_TypeDef *             dma;                ///< DMA being used  (0 in interrupt mode)

        uint32_t                  dma_it;             ///< DMA transfer complete interrupt being used (DMA1_IT_XXX from CMSIS library)

        uint16_t                  buffer_size;        ///< Circular buffer size

        uint16_t                  sample_block_size;  ///< Amount of bytes used for sample buffer

        uint16_t                  max_measurement_per_sample; ///< Maximum number of measurements per sample

        IRQn_Type                 scan_complete_irqn; ///< Either DMA transfer complete or ADC complete interrupt number

        uint8_t                   dev_id;             ///< Device ID for ADCDev virtual device

        uint8_t                   input_count;        ///< Number of ADC channels used
};

/// \def ADC_DMA_MODE
/// \brief This macro is used to check if running in DMA mode
/// \param dev - pointer to #ADCDevFwInstance structure
#define ADC_DMA_MODE(dev) ((dev)->dma!=0)

/// \def ADC_INT_MODE
/// \brief This macro is used to check if running in interrupt mode
/// \param dev - pointer to #ADCDevFwInstance structure
#define ADC_INT_MODE(dev) ((dev)->dma==0)

/// \def ADC_RESOLUTION_BITS
/// \brief This macro provides mask with meaningful bits for the sampled value
#define ADC_RESOLUTION_BITS 0x0FFF

/// \brief Initializes all ADCDev virtual devices
void adc_init();

/// \brief #ON_COMMAND callback for all ADCDev devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Communication status to be applied after command execution.
uint8_t adc_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all ADCDev devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
/// \return Communication status to be applied after read completion.
uint8_t adc_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif