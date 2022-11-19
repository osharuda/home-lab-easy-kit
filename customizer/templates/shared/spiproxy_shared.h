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

 /* --------------------> END OF THE TEMPLATE HEADER <-------------------- */

/*
#define DEV_NO_BUFFER           0
#define DEV_LINIAR_BUFFER       1
#define DEV_CIRCULAR_BUFFER     2
#define SPIPROXY_DEVICE_BUFFER_TYPE DEV_CIRCULAR_BUFFER
*/

/// \addtogroup group_spiproxy
/// @{{

/// \struct tag_SPIProxyStatus
/// \brief Structure that describes private SPIProxy status
#pragma pack(push, 1)
typedef struct tag_SPIProxyStatus {{
    uint8_t running : 1;            ///< Nonzero if sending data.
    uint8_t tx_ovf : 1;             ///< Nonzero if transmit buffer is overflow.
    uint8_t rx_ovf : 1;             ///< Nonzero if receive buffer is overflow.
    uint8_t : 0;
}} SPIProxyStatus;
typedef volatile SPIProxyStatus* PSPIProxyStatus;
#pragma pack(pop)
/// @}}
