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

/// \brief Typedef for auto-generated gpio initializing function
typedef void(*PFN_PACEMAKER_INIT_GPIO)();

/// \brief Typedef for auto-generated gpio set function
/// \param Pointer to the bitmask with pin set information (might be different types, depends on the context)
typedef void(*PFN_PACEMAKER_SET_GPIO)(uint32_t);

/// \struct tag_PaceMakerDevPrivData
/// \brief Structure that describes private PaceMakerDev data
typedef struct tag_PaceMakerDevPrivData {
    volatile PaceMakerStatus*     status;                 ///< Status of the virtual device.
    volatile PaceMakerTransition* transitions;            ///< Data buffer.
    volatile uint32_t             main_cycle_number;      ///< Number of main cycles remains to the finish. If zero infinite cycling is used.
    volatile uint16_t             main_cycle_prescaller;  ///< Main time cycle prescaller.
    volatile uint16_t             main_cycle_counter;     ///< Main timer cycle counter.
    volatile uint32_t             max_trans_number;       ///< Maximum number of transitions.
    volatile uint32_t             trans_number;           ///< Current number of transitions.
} PaceMakerDevPrivData;
typedef volatile PaceMakerDevPrivData* PPaceMakerDevPrivData;


/// \struct tag_PaceMakerDevInstance
/// \brief Structure that describes PaceMakerDev virtual device
typedef struct __attribute__ ((aligned)) tag_PaceMakerDevInstance {
        volatile DeviceContext          dev_ctx  __attribute__ ((aligned)); ///< Virtual device context

        volatile PaceMakerDevPrivData   privdata;              ///< Private data used by this PaceMakerDev device

        PFN_PACEMAKER_INIT_GPIO         pfn_init_gpio;         ///< GPIO initializer function pointer
        
	    PFN_PACEMAKER_SET_GPIO          pfn_set_gpio;          ///< GPIO setter function pointer

        volatile uint8_t*               buffer;                ///< Internal buffer

        TIM_TypeDef*                    main_timer;            ///< Main timer to generate pulses

        TIM_TypeDef*                    internal_timer;        ///< Internal timer to generate pulses

        uint32_t                        default_pin_state;     ///< Default pin state

        IRQn_Type                       main_timer_irqn;       ///< Main timer interrupt number

        IRQn_Type                       internal_timer_irqn;   ///< Internal timer interrupt number

        uint16_t                        buffer_size;           ///< Buffer size

        uint8_t                         dev_id;                ///< Device ID for PaceMakerDev virtual device
} PaceMakerDevInstance;

typedef volatile PaceMakerDevInstance* PPaceMakerDevInstance;

/// \brief Initializes all PaceMakerDev virtual devices
void pacemakerdev_init();

/// \brief #ON_COMMAND callback for all PaceMakerDev devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void pacemakerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_READDONE callback for all PaceMakerDev devices
/// \param device_id - Device ID of the virtual device which data was read
/// \param length - amount of bytes read.
void pacemakerdev_read_done(uint8_t device_id, uint16_t length);

/// \brief Declaration for initialization gpio functions
PACEMAKERDEV_FW_INIT_GPIO_HEADERS

/// \brief Declaration for set gpio functions
PACEMAKERDEV_FW_SET_GPIO_HEADERS

/// @}
#endif
