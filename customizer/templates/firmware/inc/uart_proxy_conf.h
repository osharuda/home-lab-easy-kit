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
 *   \brief Generated include header of firmware part for UART proxy device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define UART_PROXY_DEVICE_ENABLED 1

/// \addtogroup group_uart_proxy_dev
/// @{{

{__UART_PROTO_SHARED_HEADER__}

#define UART_PROXY_DEVICE_NUMBER {__UART_PROXY_DEVICE_NUMBER__}
{__UART_PROXY_BUFFER_LENGTHS__}

#define DEFINE_UART_PROXY_BUFFERS \
{__UART_PROXY_BUFFERS__}

#define UART_PROXY_DESCRIPTOR {{ {__UART_FW_PROXY_DEV_DESCRIPTOR__} }}

#define UART_PROXY_ISR_ROUTINES    \
    {__UART_PROXY_ISR_HANDLERS__}


/// @}}