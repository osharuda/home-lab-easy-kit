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
 *   \brief Generated include header of firmware part for SPIProxy device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define SPIPROXY_DEVICE_ENABLED 1
/// \addtogroup group_spiproxy
/// @{{

/// \def SPIPROXY_DEVICE_COUNT
/// \brief Number of SPIProxy devices being used
#define SPIPROXY_DEVICE_COUNT {__SPIPROXY_DEVICE_COUNT__}

/// \def SPIPROXY_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for SPIProxy virtual devices
#define SPIPROXY_FW_DEV_DESCRIPTOR {{  {__SPIPROXY_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__SPIPROXY_SHARED_HEADER__}

/// \addtogroup group_spiproxy
/// @{{

/// \def SPIPROXY_FW_IN_BUFFERS
/// \brief Defines memory blocks used for SPIProxy output circular buffers as data storage
#define SPIPROXY_FW_IN_BUFFERS {__SPIPROXY_FW_IN_BUFFERS__}

/// \def SPIPROXY_FW_OUT_BUFFERS
/// \brief Defines memory blocks used for SPIProxy input linear buffers as data storage
#define SPIPROXY_FW_OUT_BUFFERS {__SPIPROXY_FW_OUT_BUFFERS__}

/// \def SPI_FW_TX_DMA_IRQ_HANDLERS
/// \brief Defines SPIPROXY TX DMA irq handlers
#define SPI_FW_TX_DMA_IRQ_HANDLERS {__SPI_FW_TX_DMA_IRQ_HANDLERS__}

/// \def SPI_FW_RX_DMA_IRQ_HANDLERS
/// \brief Defines SPIPROXY RX DMA irq handlers
#define SPI_FW_RX_DMA_IRQ_HANDLERS {__SPI_FW_RX_DMA_IRQ_HANDLERS__}

/// \def SPI_FW_IRQ_HANDLERS
/// \brief Defines SPIPROXY (Non-DMA) irq handlers
#define SPI_FW_IRQ_HANDLERS {__SPI_FW_IRQ_HANDLERS__}

/// \def SPI_FW_DMA_TX_PREINIT
/// \brief Defines list of dma_tx_preinit structures
#define SPI_FW_DMA_TX_PREINIT {__SPI_FW_DMA_TX_PREINIT__}

/// \def SPI_FW_DMA_RX_PREINIT
/// \brief Defines list of dma_rx_preinit structures
#define SPI_FW_DMA_RX_PREINIT {__SPI_FW_DMA_RX_PREINIT__}

/// @}}




