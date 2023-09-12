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
 *   \brief PaceMakerDev device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef PACEMAKERDEV_DEVICE_ENABLED

/// \defgroup group_pacemakerdev PaceMakerDev
/// \brief PaceMakerDev support
/// @{
/// \page page_pacemakerdev
/// \tableofcontents
///
/// \section sect_pacemakerdev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \struct tag_PaceMakerDevPrivData
/// \brief Structure that describes private PaceMakerDev data
typedef struct tag_PaceMakerDevPrivData {
    uint8_t priv_data;  ///< Some private data.
} PaceMakerDevPrivData;
typedef volatile PaceMakerDevPrivData* PPaceMakerDevPrivData;


/// \struct tag_PaceMakerDevInstance
/// \brief Structure that describes PaceMakerDev virtual device
typedef struct __attribute__ ((aligned)) tag_PaceMakerDevInstance {
        volatile DeviceContext      dev_ctx __attribute__ ((aligned));            ///< Virtual device context

#if PACEMAKERDEV_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
        volatile CircBuffer         circ_buffer;        ///< Circular buffer control structure
#endif

        volatile PaceMakerDevPrivData   privdata;          ///< Private data used by this PaceMakerDev device

#if PACEMAKERDEV_DEVICE_BUFFER_TYPE != DEV_NO_BUFFER
        volatile uint8_t*           buffer;             ///< Internal buffer

        uint16_t                    buffer_size;        ///< Buffer size
#endif

        uint8_t                     dev_id;             ///< Device ID for PaceMakerDev virtual device

} PaceMakerDevInstance;

typedef volatile PaceMakerDevInstance* PPaceMakerDevInstance;

/// \brief Initializes all PaceMakerDev virtual devices
void pacemakerdev_init();

/// \brief #ON_COMMAND callback for all PaceMakerDev devices
/// \param cmd_byte - command byte received from software. Corresponds to tag_CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void pacemakerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all PaceMakerDev devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void pacemakerdev_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif