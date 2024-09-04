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
 *   \brief Generated include header of firmware part for SPIDAC device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define SPIDAC_DEVICE_ENABLED 1
/// \addtogroup group_spidac
/// @{{

/// \def SPIDAC_DEVICE_COUNT
/// \brief Number of SPIDAC devices being used
#define SPIDAC_DEVICE_COUNT {__SPIDAC_DEVICE_COUNT__}

/// \def SPIDAC_FW_DEV_DESCRIPTOR
/// \brief Defines array with configurations for SPIDAC virtual devices
#define SPIDAC_FW_DEV_DESCRIPTOR {{  {__SPIDAC_FW_DEV_DESCRIPTOR__} }}

/// @}}

{__SPIDAC_SHARED_HEADER__}

/// \addtogroup group_spidac
/// @{{

#define SPIDAC_CLOCKDIV_TO_VAL(v) ((uint16_t)(v) << 8)

/// \def SPIDAC_FW_BUFFERS
/// \brief Defines memory blocks used for SPIDAC circular buffers as data storage
#define SPIDAC_FW_BUFFERS {__SPIDAC_FW_BUFFERS__}

/// \def SPIDAC_FW_DEFAULT_VALUES
/// \brief Defines default value to be put into DAC after reset
#define SPIDAC_FW_DEFAULT_VALUES {__SPIDAC_FW_DEFAULT_VALUES__}

/// \def SPIDAC_FW_TX_DMA_IRQ_HANDLERS
/// \brief Defines SPIPROXY TX DMA irq handlers
#define SPIDAC_FW_TX_DMA_IRQ_HANDLERS {__SPIDAC_FW_TX_DMA_IRQ_HANDLERS__}

/// \def SPIDAC_FW_DMA_TX_PREINIT
/// \brief Defines list of dma_tx_preinit structures
#define SPIDAC_FW_DMA_TX_PREINIT {__SPIDAC_FW_DMA_TX_PREINIT__}

/// \def SPIDAC_FW_TIMER_IRQ_HANDLERS
/// \brief Defines SPIDAC TIMER irq handlers
#define SPIDAC_FW_TIMER_IRQ_HANDLERS {__SPIDAC_FW_TIMER_IRQ_HANDLERS__}

#define SPIDAC_NEED_LD {__SPIDAC_NEED_LD__}

#define SPIDAC_MULTI_CHANNEL {__SPIDAC_MULTI_CHANNEL__}

/// @}}




