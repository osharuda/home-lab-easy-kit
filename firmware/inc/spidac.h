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
 *   \brief SPIDAC device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef SPIDAC_DEVICE_ENABLED

/// \defgroup group_spidac SPIDAC
/// \brief SPIDAC support
/// @{
/// \page page_spidac
/// \tableofcontents
///
/// \section sect_spidac_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///


/// \struct tag_SPIDACPrivData
/// \brief Structure that describes private SPIDAC data
/// \note The whole device buffer consist of the three parts:
///       [SPIDACStatus status (8 bytes)][Default sample][Sample buffer]
///       status is pointed to by SPIDACPrivData::status
///       default sample is pointed to by SPIDACPrivData::default_sample_base (size is SPIDACPrivData::sample_size)
///       sample buffer is pointed to by SPIDACPrivData::sample_buffer_base (current size is SPIDACPrivData::sample_buffer_size,
///                     maxiumum sample buffer size is SPIDACPrivData::max_sample_buffer_size)
///
/// \note When default sample and sample buffer are accessed (for signal generation) SPIDACPrivData::sample_ptr,
///       SPIDACPrivData::sample_ptr_start and SPIDACPrivData::sample_ptr_end members must be used.
typedef struct tag_SPIDACPrivData {
    DMA_InitTypeDef   dma_tx_preinit;
    PSPIDACStatus     status;
    volatile uint32_t* ld_port_BSRR;          ///< Optimization for ld pin access from IRQ handler
    volatile uint32_t* ld_port_BRR;           ///< Optimization for ld pin access from IRQ handler
    volatile uint8_t* default_sample_base;    ///< Default sample (value) buffer base
    volatile uint8_t* sample_buffer_base;     ///< Sample buffer base
    volatile uint8_t* sample_ptr;             ///< Current sample pointer
    volatile uint8_t* sample_ptr_start;       ///< Sample buffer or default sample buffer end
    volatile uint8_t* sample_ptr_end;         ///< Sample buffer or default sample buffer end
    volatile uint32_t dummy_register;         ///< Optimization for ld pin access from IRQ handler
    volatile uint32_t dma_ccr_enabled;        ///< Cached value of the DMAChannel->CCR register when enabled (for optimization)
    volatile uint32_t dma_ccr_disabled;       ///< Cached value of the DMAChannel->CCR register when disabled (for optimization)
    uint16_t          max_sample_buffer_size; ///< Maximum sample buffer size
    uint16_t          sample_buffer_size;     ///< Current sample buffer size (actual samples)
    uint16_t          spi_cr1_enabled;        ///< Cached value of the SPI->CR1 register when enabled (for optimization)
    uint16_t          spi_cr1_disabled;       ///< Cached value of the SPI->CR1 register when disabled (for optimization)
    uint16_t          sample_size;            ///< Sample size in bytes
    uint8_t           phase_overflow_status;  ///< The status that will be applied once phase is overflown


} SPIDACPrivData;
typedef volatile SPIDACPrivData* PSPIDACPrivData;


/// \struct tag_SPIDACInstance
/// \brief Structure that describes SPIDAC virtual device
typedef struct __attribute__ ((aligned)) tag_SPIDACInstance {
    volatile DeviceContext     dev_ctx __attribute__ ((aligned)); ///< Virtual device context

    volatile SPIDACPrivData    priv_data;                  ///< Private data used by this SPIProxy device

    volatile uint8_t*          buffer;                    ///< Buffer

    volatile uint8_t*          default_values;            ///< Default values to be put after reset

    SPI_TypeDef*               spi;                       ///< SPI peripheral device.

    TIM_TypeDef*               timer;                     ///< Timer to be used.

    DMA_Channel_TypeDef*       tx_dma_channel;            ///< DMA channel being used for transmit

    GPIO_TypeDef*              mosi_port;                 ///< MOSI (TX) pin port

    GPIO_TypeDef*              sck_port;                  ///< SCK pin port

    GPIO_TypeDef*              nss_port;                  ///< NSS pin port

    GPIO_TypeDef*              ld_port;                  ///< LD line port (0 if not used)

    DMA_TypeDef*               dma;                      ///< DMA controller to be used

    uint32_t                   dma_tx_it;                ///< TX DMA channel interrupt flag

    uint32_t                   ld_bit_mask;              ///< Mask of the LD pin

    uint16_t                   buffer_size;              ///< Buffer size (for samples and for status)

    IRQn_Type                  tx_dma_complete_irqn;     ///< DMA TX transfer complete irqn

    IRQn_Type                  timer_irqn;               ///< Timer interrupt number

    uint8_t                    baud_rate_control;        ///< Baud rate control value

    uint8_t                    frame_size;               ///< Frame format: 0 - 8 bits, 1 - 16 bits.

    uint8_t                    remap;                    ///< Non-zero if remapping is required

    uint8_t                    mosi_pin;                 ///< MOSI (TX) pin

    uint8_t                    sck_pin;                  ///< SCK pin

    uint8_t                    nss_pin;                  ///< NSS pin

    uint8_t                    ld_rise;                 ///< LD line behaviour: 0 - load is triggered by fall, 1 by rise of the signal

    uint8_t                    clock_polarity;          ///< Clock polarity

    uint8_t                    clock_phase;             ///< Clock Phase

    uint8_t                    frames_per_sample;       ///< Amount of frames per sample

    uint8_t                    dev_id;                  ///< Device ID for SPIProxy virtual device
} SPIDACInstance;
typedef volatile SPIDACInstance* PSPIDACInstance;

/// \brief Initializes all SPIDAC virtual devices
void spidac_init();

/// \brief #ON_COMMAND callback for all SPIDAC devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void spidac_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all SPIDAC devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void spidac_read_done(uint8_t device_id, uint16_t length);

/// \brief Switches device to \ref STARTING mode and initializes timer, which actually starts sampling.
/// \param dev - device instance.
/// \return communication status to be passed to \ref comm_done().
uint8_t spidac_start(PSPIDACInstance dev);

/// \brief Switches device to \ref STOPPING state informing it to stop processing.
/// \param dev - device instance.
/// \return communication status to be passed to \ref comm_done().
uint8_t spidac_stop(PSPIDACInstance dev);

/// \brief Writes data to device sampling buffer
/// \param dev - device instance.
/// \param data - pointer to the data.
/// \param length - length of the data in bytes.
/// \return communication status to be passed to \ref comm_done().
uint8_t spidac_data(PSPIDACInstance dev, uint8_t* data, uint16_t length);

/// \brief Shutdowns sampling process by disabling peripherals and sets device to \ref SHUTDOWN state.
/// \param dev - device instance.
void spidac_shutdown(PSPIDACInstance dev);

/// @}
#endif