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
 *   \brief Generated include header of software part for UART proxy device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>

/// \addtogroup group_uart_proxy_dev
/// @{





/// \struct tag_UARTProxyConfig
/// \brief UARTProxy configuration structure.
typedef struct tag_UARTProxyConfig{
    uint8_t         dev_id;         ///< Virtual device id.
    uint16_t        dev_buffer_len; ///< Receive buffer length.
    const char*     dev_name;       ///< Device name.
    uint32_t		baud_rate;      ///< Communication speed (baud rate).
} UARTProxyConfig;

/// @}