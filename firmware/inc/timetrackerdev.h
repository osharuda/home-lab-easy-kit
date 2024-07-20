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
 *   \brief TimeTrackerDev device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef TIMETRACKERDEV_DEVICE_ENABLED

/// \defgroup group_timetrackerdev TimeTrackerDev
/// \brief TimeTrackerDev support
/// @{
/// \page page_timetrackerdev
/// \tableofcontents
///
/// \section sect_timetrackerdev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \struct tag_TimeTrackerDevPrivData
/// \brief Structure that describes private TimeTrackerDev data
typedef struct tag_TimeTrackerDevPrivData {
    volatile TimeTrackerStatus status;
    uint8_t priv_data;  ///< Some private data.
} TimeTrackerDevPrivData;
typedef volatile TimeTrackerDevPrivData* PTimeTrackerDevPrivData;


/// \struct tag_TimeTrackerDevInstance
/// \brief Structure that describes TimeTrackerDev virtual device
typedef struct __attribute__ ((aligned)) tag_TimeTrackerDevInstance {
        volatile DeviceContext      dev_ctx __attribute__ ((aligned));  ///< Virtual device context
        volatile struct CircBuffer         circ_buffer __attribute__ ((aligned));                        ///< Circular buffer control structure
        volatile TimeTrackerDevPrivData   privdata __attribute__ ((aligned));                     ///< Private data used by this TimeTrackerDev device
        volatile GPIO_descr         interrup_line;                      ///< Line dedicated for interrupts
        volatile GPIO_descr         near_full_line;                     ///< Line dedicated for buffer nearly full warning
        volatile uint8_t*           buffer;                             ///< Internal buffer
        uint16_t                    buffer_size;                        ///< Buffer size
        uint16_t                    intrrupt_exci_cr;                   ///< Interrupt EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
        uint8_t                     dev_id;                             ///< Device ID for TimeTrackerDev virtual device
        uint8_t                     trig_on_rise;                       ///< 1 if trigger on signal rise is required, otherwise 0
        uint8_t                     trig_on_fall;                       ///< 1 if trigger on signal fall is required, otherwise 0
} TimeTrackerDevInstance;

typedef volatile TimeTrackerDevInstance* PTimeTrackerDevInstance;

/// \brief Initializes all TimeTrackerDev virtual devices
void timetrackerdev_init();

/// \brief #ON_COMMAND callback for all TimeTrackerDev devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Communication status to be applied after command execution.
uint8_t timetrackerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all TimeTrackerDev devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
/// \return Communication status to be applied after read completion.
uint8_t timetrackerdev_read_done(uint8_t device_id, uint16_t length);

/// @}
#endif