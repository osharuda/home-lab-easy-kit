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
 *   \brief SPIProxy device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef SPIPROXY_DEVICE_ENABLED

/// \defgroup group_spiproxy SPIProxy
/// \brief SPIProxy support
/// @{
/// \page page_spiproxy
/// \tableofcontents
///
/// \section sect_spiproxy_01 SPIProxy allows you to control SPI devices
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \struct SPIProxyPrivData
/// \brief Structure that holds private SPIProxy state.
struct __attribute__ ((aligned)) SPIProxyPrivData {
    struct SPIProxyStatus*   status;      ///< Pointer to the status header which is allocated before input_buffer. This
                                          ///< status is read first when data is read from SPIProxy.

    uint8_t* in_data_buffer;              ///< Pointer to the input buffer to store data.

    DMA_InitTypeDef*  dma_rx_preinit;     ///< Preinitialized DMA_InitTypeDef structure for RX DMA channel. Not used in interrupt mode.

    DMA_InitTypeDef*  dma_tx_preinit;     ///< Preinitialized DMA_InitTypeDef structure for TX DMA channel. Not used in interrupt mode.

    uint16_t          recv_frames_mask;   ///< Optimization: Special value used to avoid extra "if" statement when receiving data in uni-direction mode.

    uint16_t          send_frame_counter; ///< Counter of the send frames (per single transaction). Used to detect the end of the transaction.

    uint16_t          recv_frame_counter; ///< Counter of the received frames (per single transaction). Used to detect the end of the transaction.

    uint16_t          frame_number;       ///< Number of frames for a given transaction.

    uint16_t          transmit_len;       ///< Size of the last transmit in bytes
};

/// \struct SPIProxyInstance
/// \brief Structure that describes SPIProxy virtual device
struct __attribute__ ((aligned)) SPIProxyInstance {
        struct DeviceContext       dev_ctx __attribute__ ((aligned)); ///< Virtual device context

        struct SPIProxyPrivData    privdata;                  ///< Private data used by this SPIProxy device

        uint8_t*                   out_buffer;                ///< Output buffer for receive from slave

        uint8_t*                   in_status_and_data_buffer; ///< Input buffer allocated to store status and input data.
                                                              ///< Use SPIProxyPrivData::status and SPIProxyPrivData::in_data_buffer
                                                              ///< to access status and input data correspondingly.

        SPI_TypeDef*               spi;                       ///< SPI peripheral device.

        DMA_Channel_TypeDef*       tx_dma_channel;            ///< DMA channel being used for transmit

        DMA_Channel_TypeDef*       rx_dma_channel;            ///< DMA channel being used for receive

        GPIO_TypeDef*              miso_port;                 ///< MISO (RX) pin port

        GPIO_TypeDef*              mosi_port;                 ///< MOSI (TX) pin port

        GPIO_TypeDef*              sck_port;                  ///< SCK pin port

        GPIO_TypeDef*              nss_port;                  ///< NSS pin port

        uint32_t                   dma_rx_it;                 ///< RX DMA channel interrupt flag

        uint32_t                   dma_tx_it;                 ///< TX DMA channel interrupt flag

        uint16_t                   buffer_size;               ///< Buffer size (input and output buffers have the same size)

        IRQn_Type                  tx_dma_complete_irqn;     ///< DMA TX transfer complete irqn

        IRQn_Type                  rx_dma_complete_irqn;     ///< DMA RX transfer complete irqn

        IRQn_Type                  spi_interrupt_irqn;       ///< DMA RX transfer complete irqn

        uint8_t                    baud_rate_control;        ///< Baud rate control value

        uint8_t                    frame_size;               ///< Frame format: 0 - 8 bits, 1 - 16 bits.

        uint8_t                    remap;                    ///< Non-zero if remapping is required

        uint8_t                    miso_pin;                 ///< MISO (RX) pin

        uint8_t                    mosi_pin;                 ///< MOSI (TX) pin

        uint8_t                    sck_pin;                  ///< SCK pin

        uint8_t                    nss_pin;                  ///< NSS pin

        uint8_t                    clock_polarity;          ///< Clock polarity

        uint8_t                    clock_phase;             ///< Clock Phase

        uint8_t                    frame_format;            ///< Frame format

        uint8_t                    is_bidirectional;        ///< true if bidirectional, otherwise false.

        uint8_t                    use_dma;                 ///< true if DMA should be used, otherwise false.

        uint8_t                    dev_id;                  ///< Device ID for SPIProxy virtual device
};

/// \brief Initializes all SPIProxy virtual devices
void spiproxy_init();

/// \brief #ON_COMMAND callback for all SPIProxy devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Result of the operation as communication status.
uint8_t spiproxy_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all SPIProxy devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
/// \return Result of the operation as communication status.
uint8_t spiproxy_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif