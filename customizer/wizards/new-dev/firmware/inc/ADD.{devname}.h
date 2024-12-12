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

#include "{devname}_conf.h"

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

/// \struct {DevName}PrivData
/// \brief Structure that describes private {DevName} data

struct __attribute__ ((aligned)) {DevName}PrivData {
    struct {DevName}Status status     __attribute__ ((aligned)); ///< Private, actual status of the device
    struct {DevName}Settings settings __attribute__ ((aligned)); ///< Device settings
    uint64_t priv_data                __attribute__ ((aligned)); ///< Some Device data.
} {DevName}PrivData;


/// \struct {DevName}Instance
/// \brief Structure that describes {DevName} virtual device
struct __attribute__ ((aligned)) {DevName}Instance {
        struct DeviceContext        dev_ctx __attribute__ ((aligned));     ///< Virtual device context

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
        struct CircBuffer           circ_buffer __attribute__ ((aligned)); ///< Circular buffer control structure
#endif
        struct {DevName}Status      status __attribute__ ((aligned));     ///< Public status available to software,
                                                                          ///< a copy of the privdata->status made during {devname}_sync() call.
        struct {DevName}PrivData    privdata __attribute__ ((aligned));   ///< Private data used by this {DevName} device

#if {DEVNAME}_DEVICE_BUFFER_TYPE != DEV_NO_BUFFER
        uint8_t*                    buffer;             ///< Internal buffer
        uint16_t                    buffer_size;        ///< Buffer size
#endif
        uint8_t                     dev_id;             ///< Device ID for {DevName} virtual device
};

/// \brief Initializes all {DevName} virtual devices
void {devname}_init();

/// \brief #ON_COMMAND callback for all {DevName} devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Updated device status (later synchronously copied to g_comm_status)
uint8_t {devname}_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all {DevName} devices
/// \param device_id - device id of the device whose data was read
/// \param length - length of the read (transmitted) data.
/// \return Updated device status (later synchronously copied to g_comm_status)
uint8_t {devname}_read_done(uint8_t device_id, uint16_t length);

/// \brief Synchronizes {DevName} status before reading it by software
/// \param device_id - device id of the device whose data was read
/// \param length - length of the read (transmitted) data. In this case it is total number of bytes, those which belong to
///        incomplete CommCommandHeader. Obviously this value may not be >= then sizeof(CommCommandHeader).
/// \return Updated device status (later synchronously copied to g_comm_status)
uint8_t {devname}_sync(uint8_t cmd_byte, uint16_t length);

/// @}
#endif