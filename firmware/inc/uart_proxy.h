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
 *   \brief USART proxy device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef UART_PROXY_DEVICE_ENABLED


/// \defgroup group_uart_proxy_dev UARTDev
/// \brief UART proxy support
/// @{
/// \page page_uart_proxy_dev
/// \tableofcontents
///
/// \section sect_uart_proxy_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

struct __attribute__ ((aligned)) UartProxyDevInstance {
    struct DeviceContext dev_ctx __attribute__ ((aligned));
    struct CircBuffer circ_buffer;
    USART_TypeDef*  uart_port;
    GPIO_TypeDef *  rx_port;
    GPIO_TypeDef *  tx_port;
    uint8_t*        dev_buffer;
    uint32_t		baud_rate;
	uint16_t        dev_buffer_len;
	uint16_t        rx_pin_mask;
	uint16_t        tx_pin_mask;
	IRQn_Type		irq_vector;
	uint8_t         dev_id;
};

void uart_proxy_init(void);

/// @}

#endif