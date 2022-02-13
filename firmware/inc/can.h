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
 *   \brief Can device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef CAN_DEVICE_ENABLED

#include <stm32f10x_can.h>
#include "fw_can.h"

/// \defgroup group_can Can
/// \brief Can support
/// @{
/// \page page_can
/// \tableofcontents
///
/// \section sect_can_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \struct tag_CanPrivData
/// \brief Structure that describes private Can data
typedef struct tag_CanPrivData {
    CanStatus status;

    volatile CAN_FilterInitTypeDef can_filters[CAN_MAX_FILTER_COUNT];   ///< CAN filters to be applied
} CanPrivData;

/// \struct tag_CanInstance
/// \brief Structure that describes Can virtual device
typedef struct __attribute__ ((aligned)) tag_CanInstance {
        volatile DeviceContext dev_ctx __attribute__ ((aligned)); ///< Virtual device context

        volatile CircBuffer         circ_buffer;    ///< Circular buffer control structure

        volatile CanPrivData        privdata;       ///< Private data used by this Can device

        volatile uint8_t*           buffer;         ///< Internal buffer

        CAN_TypeDef*                can;            ///< Can module to be used

        GPIO_TypeDef*               canrx_port;     ///< CAN RX pin port

        GPIO_TypeDef*               cantx_port;     ///< CAN TX pin port

        uint16_t                    buffer_size;    ///< Circular buffer size

        uint16_t                    can_prescaller; ///< CAN prescaller

        IRQn_Type                   irqn_tx;        ///< CAN TX interrupt IRQn value

        IRQn_Type                   irqn_rx0;       ///< CAN RX0 interrupt IRQn value

        IRQn_Type                   irqn_rx1;       ///< CAN RX1 interrupt IRQn value

        IRQn_Type                   irqn_sce;       ///< CAN SCE interrupt IRQn value

        uint8_t                     can_bs1;        ///< CAN segment 1 length

        uint8_t                     can_sample_point;   ///< CAN sample point length

        uint8_t                     can_bs2;        ///< CAN segment 2 length

        uint8_t                     can_remap;      ///< Non-zero if remap is required

        uint8_t                     canrx_pin;      ///< CAN RX pin number

        uint8_t                     cantx_pin;      ///< CAN TX pin number

        uint8_t                     dev_id;         ///< Device ID for Can virtual device
} CanInstance;

/// \brief Initializes all Can virtual devices
void can_init();

/// \brief #ON_COMMAND callback for all Can devices
/// \param cmd_byte - command byte received from software. Corresponds to tag_CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void can_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all Can devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void can_read_done(uint8_t device_id, uint16_t length);


/// \brief #ON_POLLING callback for all Can devices
/// \param device_id - Device ID of the virtual device which data was read
void can_polling(uint8_t device_id);

/// @}
#endif