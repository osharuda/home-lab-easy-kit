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
 *   \brief Generated include header of firmware part for Stepper Motor device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define STEP_MOTOR_DEVICE_ENABLED     1
#define STEP_MOTOR_FIRMWARE

#include "circbuffer.h"
#include "i2c_bus.h"

/// \addtogroup group_step_motor_dev
/// @{{

/// \defgroup group_step_motor_dev_motor_lines GPIO lines
/// \brief Stepper motor driver GPIO lines being used by firmware
/// @{{
/// \page page_step_motor_dev_motor_lines
/// \tableofcontents
///
/// \section sect_step_motor_dev_motor_lines_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \def STEP_MOTOR_LINE_STEP
/// \brief Defines index for STEP signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_STEP          ((uint8_t)0)

/// \def STEP_MOTOR_LINE_DIR
/// \brief Defines index for DIRECTION signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_DIR           ((uint8_t)1)

/// \def STEP_MOTOR_LINE_M1
/// \brief Defines index for M1 signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_M1            ((uint8_t)2)

/// \def STEP_MOTOR_LINE_M2
/// \brief Defines index for M2 signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_M2            ((uint8_t)3)

/// \def STEP_MOTOR_LINE_M3
/// \brief Defines index for M3 signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_M3            ((uint8_t)4)

/// \def STEP_MOTOR_LINE_ENABLE
/// \brief Defines index for ENABLE signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_ENABLE        ((uint8_t)5)

/// \def STEP_MOTOR_LINE_RESET
/// \brief Defines index for RESET signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_RESET         ((uint8_t)6)

/// \def STEP_MOTOR_LINE_SLEEP
/// \brief Defines index for SLEEP signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_SLEEP         ((uint8_t)7)

/// \def STEP_MOTOR_LINE_FAULT
/// \brief Defines index for FAULT signal line difinition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_FAULT         ((uint8_t)8)

/// \def STEP_MOTOR_LINE_CWENDSTOP
/// \brief Defines index for CW hardware endstop signal line definition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_CWENDSTOP     ((uint8_t)9)

/// \def STEP_MOTOR_LINE_CCWENDSTOP
/// \brief Defines index for CCW hardware endstop signal line definition in array of #tag_StepMotorLine
#define STEP_MOTOR_LINE_CCWENDSTOP    ((uint8_t)10)

/// \def STEP_MOTOR_LINE_COUNT
/// \brief Defines number of lines to be described for stepper motor drive
#define STEP_MOTOR_LINE_COUNT         (STEP_MOTOR_LINE_CCWENDSTOP+1)

/// \struct tag_StepMotorLine
/// \brief Describes GPIO line required to communicate with step motor driver
typedef struct tag_StepMotorLine {{
    GPIO_TypeDef* port;     ///< port used for this gpio line (see GPIO_TypeDef in CMSIS). 0 indicates unused line.
    uint8_t      pin;       ///< pin number used for this gpio line (see GPIO_TypeDef in CMSIS).
}} StepMotorLine, *PStepMotorLine;
/// @}}
/// @}}

/// \addtogroup group_step_motor_dev_impl
/// @{{

/// \def STEP_MOTOR_CORRECTION_FACTOR
/// \brief Defines default correction factor to be used with #step_motor_correct_timing()
#define STEP_MOTOR_CORRECTION_FACTOR 1

/// \struct tag_StepMotorDevPrivData
/// \brief This structure is being used by firmware to store stepper motor device specific data (for all motors, but
///        for single device).
typedef struct __attribute__ ((aligned)) tag_StepMotorDevPrivData {{
    volatile uint64_t last_event_timestamp __attribute__ ((aligned)); ///< Last stepper motor device timer timestamp
}} StepMotorDevPrivData;

typedef volatile StepMotorDevPrivData* PStepMotorDevPrivData;

#define STEP_MOTOR_CMDSTATUS_INIT       (uint8_t)(0)
#define STEP_MOTOR_CMDSTATUS_WAIT       (uint8_t)(1)
#define STEP_MOTOR_CMDSTATUS_STEP       (uint8_t)(2)
#define STEP_MOTOR_CMDSTATUS_STEPWAIT   (uint8_t)(3)
#define STEP_MOTOR_CMDSTATUS_DONE       (uint8_t)(0xFF)

/// \struct tag_StepMotorCmd
/// \brief This structure is being used by firmware to store a command sent by software to specific stepper motor
typedef __attribute__ ((aligned (8))) struct tag_StepMotorCmd {{
    uint64_t param; ///< Parameter passed with the command

    uint64_t wait;  ///< Period of time to wait. Once wait is elapsed command (or other command) execution may be
                    ///  continued.

    uint8_t cmd;    ///< Stepper motor command (command byte)

    uint8_t state;  ///< Value indicating state of the command execution. This value is specific to each stepper motor
                    ///  command handler.
}} StepMotorCmd, *PStepMotorCmd;

/// \struct tag_StepMotorContext
/// \brief This structure is being used by firmware to store a motor specific data
typedef __attribute__ ((aligned (8))) struct tag_StepMotorContext {{
    volatile uint64_t                late_us;                   ///< 64-bit value that specifies number of microseconds
                                                                ///  command execution is late. This value is used to
                                                                ///  correct further timer events.

    volatile uint64_t                steps_beyond_endstop;      ///< Precalculated value for move command that equals a
                                                                ///  number of steps to be made before corresponding
                                                                ///  software limit is triggered. Used for both non-stop
                                                                ///  moves and moves with parameters.

    volatile uint64_t                step_wait;                 ///< Number of microseconds that separate two subsequent
                                                                ///  step pulses. This value is set by #STEP_MOTOR_SET
                                                                ///  command (#STEP_MOTOR_SET_STEP_WAIT)

    volatile int8_t                  pos_change_by_step;        ///< Increment value to be added to motor position with
                                                                ///  each step pulse (may be negative)

    volatile uint8_t                 step_counter_decrement;    ///< Used to count steps for move command. For non-stop
                                                                ///  moves is 0, for moves with specified number of
                                                                ///  steps is 1.

    volatile uint32_t                move_sw_endstop_flag;      ///< For software limits defines flag to be used when
                                                                ///  software limit will trigger. May be either
                                                                ///  #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or
                                                                ///  #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED.

    __attribute__ ((aligned (8))) volatile StepMotorCmd current_cmd; ///< #tag_StepMotorCmd structure that describes currently
                                                                     ///  executed command

    volatile CircBuffer              circ_buffer;               ///< Circular buffer to store commands. Note actual
                                                                ///  buffer memory pointer is stored in
                                                                ///  tag_StepMotorDescriptor#buffer. This is just
                                                                ///  circular buffer control structure.
}} StepMotorContext, *PStepMotorContext;

/// @}}
/// @}}

{__STEP_MOTOR_SHARED_HEADER__}

/// \addtogroup group_step_motor_dev_configuration
/// @{{

/// \def STEP_MOTOR_ENDSTOP_TO_LINE
/// \brief Converts either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED flag into corresponding line in tag_StepMotorDescriptor#lines
/// \param endstop_trig - either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED
/// \return line for the endstop from tag_StepMotorDescriptor#lines (either #STEP_MOTOR_LINE_CWENDSTOP or #STEP_MOTOR_LINE_CCWENDSTOP)
/// \warning Use this macro with absolute care, because endstop_trig must be equal either STEP_MOTOR_CW_ENDSTOP_TRIGGERED or STEP_MOTOR_CCW_ENDSTOP_TRIGGERED
#define STEP_MOTOR_ENDSTOP_TO_LINE(endstop_trig)         (uint8_t)(STEP_MOTOR_LINE_CWENDSTOP + (uint8_t)((endstop_trig) >> STEP_MOTOR_CCW_ENDSTOP_TRIGGERED_OFFSET))

/// \def STEP_MOTOR_IS_USED_ENDSTOP
/// \brief Converts either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED flag into corresponding line in tag_StepMotorDescriptor#lines
/// \param config - stepper motor configuration flags
/// \param endstop_trig - either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED
/// \return line for the endstop from tag_StepMotorDescriptor#lines (either #STEP_MOTOR_LINE_CWENDSTOP or #STEP_MOTOR_LINE_CCWENDSTOP)
/// \warning Use this macro with absolute care, because endstop_trig must be equal either STEP_MOTOR_CW_ENDSTOP_TRIGGERED or STEP_MOTOR_CCW_ENDSTOP_TRIGGERED
#define STEP_MOTOR_IS_USED_ENDSTOP(config, endstop_trig) ((config) & ((endstop_trig) >> (STEP_MOTOR_CW_ENDSTOP_TRIGGERED_OFFSET - STEP_MOTOR_CWENDSTOP_IN_USE_OFFSET)))

/// @}}

{__STEP_MOTORS_BUFFERS__}

{__STEP_MOTORS_DEV_STATUSES__}

{__STEP_MOTORS_MOTOR_COUNTS__}

{__STEP_MOTOR_MOTOR_DESCRIPTORS__}

{__STEP_MOTOR_MOTOR_DESCRIPTORS_ARRAYS__}

{__STEP_MOTOR_MOTOR_CONTEXT_ARRAYS__}

{__STEP_MOTOR_MOTOR_STATUS_ARRAYS__}

{__STEP_MOTOR_DEVICE_DESCRIPTORS__}

#define STEP_MOTOR_FW_TIMER_IRQ_HANDLERS {__STEP_MOTOR_FW_TIMER_IRQ_HANDLERS__}

#define STEP_MOTOR_DEVICE_COUNT {__STEP_MOTOR_DEVICE_COUNT__}
#define STEP_MOTOR_DEVICES {{ {__STEP_MOTOR_DEVICES__} }}

/// \defgroup group_step_motor_dev_description Motor descriptor
/// \brief Stepper motor description structure
/// @{{
/// \page page_step_motor_dev_description
/// \tableofcontents
///
/// \section sect_step_motor_dev_description_01 Stepper motor descriptor
/// Stepper motor descriptor is a structure used to provide firmware or software information about stepper motor.
/// Both software and firmware use #tag_StepMotorDescriptor structure for this purpose, however conditional compilation is
/// used and this structure is not the same for software and firmware.
///

/// \struct tag_StepMotorDescriptor
/// \brief Describes stepper motor default configuration
typedef struct __attribute__ ((aligned)) tag_StepMotorDescriptor {{
        uint32_t      config_flags;                 ///< flags used to describe default step motor behaviour. Corresponds to tag_StepMotorStatus#motor_state. See @ref group_step_motor_dev_configuration
        uint16_t      buffer_size;                  ///< stepper motor command buffer size in bytes.
        uint64_t      default_speed;                ///< stepper motor default speed. tag_StepMotorDescriptor#default_speed is a number of microseconds between step pulses. It doesn't take into account microstepping.
        uint8_t       motor_driver;                 ///< stepper motor driver type. One of the STEP_MOTOR_DRIVER_XXX values. See @ref group_step_motor_dev_microstep_tables
        int64_t       cw_sft_limit;                 ///< Default software limit for stepper motor position during CW moves. Ignored if hardware end-stop is used
        int64_t       ccw_sft_limit;                ///< Default software limit for stepper motor position during CCW moves. Ignored if hardware end-stop is used
        uint8_t*      buffer;                       ///< Stepper motor buffer (available in firmware part only)
        StepMotorLine lines[STEP_MOTOR_LINE_COUNT]; ///< Descriptor of lines connected to the stepper motor driver (available in firmware part only). See @ref group_step_motor_dev_motor_lines
        uint16_t      fault_exticr;                 ///< Fault EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
        uint16_t      cw_endstop_exticr;            ///< Hardware CW endstop EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
        uint16_t      ccw_endstop_exticr;           ///< Hardware CW endstop EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
}} StepMotorDescriptor;
/// @}}

/// \defgroup group_step_motor_dev_device_description Device descriptor
/// \brief Stepper motor device description structure
/// @{{
/// \page page_step_motor_dev_device_description
/// \tableofcontents
///
/// \section sect_step_motor_dev_device_description_01 Description
/// Customizer generates description for all stepper motor devices in #tag_StepMotorDevice structures.
/// Some fields are used in runtime, some fields should remain constant. Be very careful changing them.
///

/// \struct tag_StepMotorDevice
/// \brief This structure is being used by firmware and software as storage of all information needed.
/// \note This structure describes different field set for firmware and software. Some fields are common, some unique to firmware or software.
typedef struct __attribute__ ((aligned)) tag_StepMotorDevice {{
    volatile DeviceContext         dev_ctx   __attribute__ ((aligned)); ///< Device context structure (available in firmware part only)
    volatile StepMotorDevPrivData  priv_data __attribute__ ((aligned)); ///< Private data unique for each stepper motor device.  (available in firmware part only).
    TIM_TypeDef*                   timer;         ///< Timer being used by device (available in firmware part only). Do not change this field.
    volatile PStepMotorContext     motor_context; ///< Array of the #tag_StepMotorContext structures, one per each stepper motor controlled by the device.  (available in firmware part only).
    volatile PStepMotorDevStatus   status;        ///< Pointer to the #tag_StepMotorDevStatus. It is used as bufer to read information by software. Firmware code should make changes to this structure with interrupts disabled.  (available in firmware part only)
    uint16_t                       status_size;   ///< Size of the tag_StepMotorDevice#status structure in bytes. (available in firmware part only). Do not change this field.
    IRQn_Type                      timer_irqn;    ///< Timer interrupt number being used by device (available in firmware part only). Do not change this field.
    volatile StepMotorDescriptor** motor_descriptor; ///< Array of the pointers to #tag_StepMotorDescriptor for each stepper motor controlled by the device. Do not change this field.
    uint8_t                        motor_count;   ///< Number of stepper motors controled by this device. Do not change this field.
    uint8_t                        dev_id;        ///< Device ID for the stepper motor device. Do not change this field.
}} StepMotorDevice;

typedef volatile StepMotorDevice* PStepMotorDevice;
