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
 *   \brief {DevName} device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef {DEVNAME}_DEVICE_ENABLED

/// \defgroup group_{devname} {DevName}
/// \brief {DevName} support
/// @{
/// \page page_{devname}
/// \tableofcontents
///
/// \section sect_{devname}_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \struct tag_{DevName}PrivData
/// \brief Structure that describes private {DevName} data
typedef struct tag_{DevName}PrivData {
    uint8_t priv_data;  ///< Some private data.
} {DevName}PrivData;
typedef volatile {DevName}PrivData* P{DevName}PrivData;


/// \struct tag_{DevName}Instance
/// \brief Structure that describes {DevName} virtual device
typedef struct __attribute__ ((aligned)) tag_{DevName}Instance {
        volatile DeviceContext      dev_ctx __attribute__ ((aligned));            ///< Virtual device context

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
        volatile CircBuffer         circ_buffer;        ///< Circular buffer control structure
#endif

        volatile {DevName}PrivData   privdata;          ///< Private data used by this {DevName} device

#if {DEVNAME}_DEVICE_BUFFER_TYPE != DEV_NO_BUFFER
        volatile uint8_t*           buffer;             ///< Internal buffer

        uint16_t                    buffer_size;        ///< Buffer size
#endif

        uint8_t                     dev_id;             ///< Device ID for {DevName} virtual device

} {DevName}Instance;

typedef volatile {DevName}Instance* P{DevName}Instance;

/// \brief Initializes all {DevName} virtual devices
void {devname}_init();

/// \brief #ON_COMMAND callback for all {DevName} devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void {devname}_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all {DevName} devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void {devname}_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif