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
 *   \brief Stepper Motor device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef STEP_MOTOR_DEVICE_ENABLED

#include "circbuffer.h"
#include "step_motor_conf.h"

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
/// \brief Defines index for STEP signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_STEP          ((uint8_t)0)

/// \def STEP_MOTOR_LINE_DIR
/// \brief Defines index for DIRECTION signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_DIR           ((uint8_t)1)

/// \def STEP_MOTOR_LINE_M1
/// \brief Defines index for M1 signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_M1            ((uint8_t)2)

/// \def STEP_MOTOR_LINE_M2
/// \brief Defines index for M2 signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_M2            ((uint8_t)3)

/// \def STEP_MOTOR_LINE_M3
/// \brief Defines index for M3 signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_M3            ((uint8_t)4)

/// \def STEP_MOTOR_LINE_ENABLE
/// \brief Defines index for ENABLE signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_ENABLE        ((uint8_t)5)

/// \def STEP_MOTOR_LINE_RESET
/// \brief Defines index for RESET signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_RESET         ((uint8_t)6)

/// \def STEP_MOTOR_LINE_SLEEP
/// \brief Defines index for SLEEP signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_SLEEP         ((uint8_t)7)

/// \def STEP_MOTOR_LINE_FAULT
/// \brief Defines index for FAULT signal line difinition in array of #StepMotorLine
#define STEP_MOTOR_LINE_FAULT         ((uint8_t)8)

/// \def STEP_MOTOR_LINE_CWENDSTOP
/// \brief Defines index for CW hardware endstop signal line definition in array of #StepMotorLine
#define STEP_MOTOR_LINE_CWENDSTOP     ((uint8_t)9)

/// \def STEP_MOTOR_LINE_CCWENDSTOP
/// \brief Defines index for CCW hardware endstop signal line definition in array of #StepMotorLine
#define STEP_MOTOR_LINE_CCWENDSTOP    ((uint8_t)10)

/// \def STEP_MOTOR_LINE_COUNT
/// \brief Defines number of lines to be described for stepper motor drive
#define STEP_MOTOR_LINE_COUNT         (STEP_MOTOR_LINE_CCWENDSTOP+1)

/// \struct StepMotorLine
/// \brief Describes GPIO line required to communicate with step motor driver
struct StepMotorLine {
        GPIO_TypeDef* port;     ///< port used for this gpio line (see GPIO_TypeDef in CMSIS). 0 indicates unused line.
        uint8_t      pin;       ///< pin number used for this gpio line (see GPIO_TypeDef in CMSIS).
};
/// @}}
/// @}}


/// \defgroup group_step_motor_dev StepMotorDev
/// @{
/// \page page_step_motor_dev
/// \tableofcontents
///
/// \section sect_step_motor_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \defgroup group_step_motor_dev_impl Implementation
/// \brief Implementation details
/// @{
/// \page page_step_motor_dev_impl
/// \tableofcontents
///
/// \section sect_step_motor_dev_impl_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

/// \addtogroup group_step_motor_dev_impl
/// @{{

/// \def STEP_MOTOR_CORRECTION_FACTOR
/// \brief Defines default correction factor to be used with #step_motor_correct_timing()
#define STEP_MOTOR_CORRECTION_FACTOR 1

/// \struct StepMotorDevPrivData
/// \brief This structure is being used by firmware to store stepper motor device specific data (for all motors, but
///        for single device).
struct __attribute__ ((aligned)) StepMotorDevPrivData {
    volatile __attribute__ ((aligned)) uint64_t last_event_timestamp; ///< Last stepper motor device timer timestamp

    ///< Actual status used by device. It is being copyied to the dev->status by sync request from the software. If
    ///< Corresponding request is not made, read data will be invalid.
    struct StepMotorDevStatus* internal_status;
};


#define STEP_MOTOR_CMDSTATUS_INIT       (uint8_t)(0)
#define STEP_MOTOR_CMDSTATUS_WAIT       (uint8_t)(1)
#define STEP_MOTOR_CMDSTATUS_STEP       (uint8_t)(2)
#define STEP_MOTOR_CMDSTATUS_STEPWAIT   (uint8_t)(3)
#define STEP_MOTOR_CMDSTATUS_DONE       (uint8_t)(0xFF)

/// \struct StepMotorCmd
/// \brief This structure is being used by firmware to store a command sent by software to specific stepper motor
struct __attribute__ ((aligned (8))) StepMotorCmd {
        uint64_t param; ///< Parameter passed with the command

        uint64_t wait;  ///< Period of time to wait. Once wait is elapsed command (or other command) execution may be
        ///  continued.

        uint8_t cmd;    ///< Stepper motor command (command byte)

        uint8_t state;  ///< Value indicating state of the command execution. This value is specific to each stepper motor
        ///  command handler.
};

/// \struct StepMotorContext
/// \brief This structure is being used by firmware to store a motor specific data
struct __attribute__ ((aligned (8))) StepMotorContext {
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

        struct __attribute__ ((aligned (8))) StepMotorCmd current_cmd; ///< #StepMotorCmd structure that describes currently
        ///  executed command

        struct CircBuffer       circ_buffer;               ///< Circular buffer to store commands. Note actual
        ///  buffer memory pointer is stored in
        ///  StepMotorDescriptor#buffer. This is just
        ///  circular buffer control structure.
};

/// @}}


/// \defgroup group_step_motor_dev_description Motor descriptor
/// \brief Stepper motor description structure
/// @{
/// \page page_step_motor_dev_description
/// \tableofcontents
///
/// \section sect_step_motor_dev_description_01 Stepper motor descriptor
/// Stepper motor descriptor is a structure used to provide firmware or software information about stepper motor.
/// Both software and firmware use #StepMotorDescriptor structure for this purpose, however conditional compilation is
/// used and this structure is not the same for software and firmware.
///

/// \struct StepMotorDescriptor
/// \brief Describes stepper motor default configuration
struct __attribute__ ((aligned)) StepMotorDescriptor {
    uint32_t      config_flags;                 ///< flags used to describe default step motor behaviour. Corresponds to StepMotorStatus#motor_state. See @ref group_step_motor_dev_configuration
    uint16_t      buffer_size;                  ///< stepper motor command buffer size in bytes.
    uint64_t      default_speed;                ///< stepper motor default speed. StepMotorDescriptor#default_speed is a number of microseconds between step pulses. It doesn't take into account microstepping.
    uint8_t       motor_driver;                 ///< stepper motor driver type. One of the STEP_MOTOR_DRIVER_XXX values. See @ref group_step_motor_dev_microstep_tables
    int64_t       cw_sft_limit;                 ///< Default software limit for stepper motor position during CW moves. Ignored if hardware end-stop is used
    int64_t       ccw_sft_limit;                ///< Default software limit for stepper motor position during CCW moves. Ignored if hardware end-stop is used
    uint8_t*      buffer;                       ///< Stepper motor buffer (available in firmware part only)
    struct StepMotorLine lines[STEP_MOTOR_LINE_COUNT]; ///< Descriptor of lines connected to the stepper motor driver (available in firmware part only). See @ref group_step_motor_dev_motor_lines
    uint16_t      fault_exticr;                 ///< Fault EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
    uint16_t      cw_endstop_exticr;            ///< Hardware CW endstop EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
    uint16_t      ccw_endstop_exticr;           ///< Hardware CW endstop EXTI control register value; see AFIO_EXTICRXXX constants in CMSIS. (available in firmware part only)
};
/// @}

/// \defgroup group_step_motor_dev_device_description Device descriptor
/// \brief Stepper motor device description structure
/// @{
/// \page page_step_motor_dev_device_description
/// \tableofcontents
///
/// \section sect_step_motor_dev_device_description_01 Description
/// Customizer generates description for all stepper motor devices in #StepMotorDevice structures.
/// Some fields are used in runtime, some fields should remain constant. Be very careful changing them.
///

/// \struct StepMotorDevice
/// \brief This structure is being used by firmware and software as storage of all information needed.
/// \note This structure describes different field set for firmware and software. Some fields are common, some unique to firmware or software.
struct __attribute__ ((aligned)) StepMotorDevice {
    struct   DeviceContext         dev_ctx   __attribute__ ((aligned)); ///< Device context structure (available in firmware part only)
    struct   StepMotorDevPrivData  priv_data __attribute__ ((aligned)); ///< Private data unique for each stepper motor device.  (available in firmware part only).
    TIM_TypeDef*                   timer;         ///< Timer being used by device (available in firmware part only). Do not change this field.
    struct   StepMotorContext*     motor_context; ///< Array of the #StepMotorContext structures, one per each stepper motor controlled by the device.  (available in firmware part only).
    struct   StepMotorDevStatus*   status;        ///< Pointer to the #StepMotorDevStatus. It is used as bufer to read information by software. Firmware code should make changes to this structure with interrupts disabled.  (available in firmware part only)
    uint16_t                       status_size;   ///< Size of the StepMotorDevice#status structure in bytes. (available in firmware part only). Do not change this field.
    IRQn_Type                      timer_irqn;    ///< Timer interrupt number being used by device (available in firmware part only). Do not change this field.
    struct   StepMotorDescriptor** motor_descriptor; ///< Array of the pointers to #StepMotorDescriptor for each stepper motor controlled by the device. Do not change this field.
    uint8_t                        motor_count;   ///< Number of stepper motors controled by this device. Do not change this field.
    uint8_t                        dev_id;        ///< Device ID for the stepper motor device. Do not change this field.
};

extern const StepMotorMicrostepTables g_step_motor_microstep_tables;

/// \brief This function initializes all configured stepper motor devices
void step_motor_init(void);

/// \brief Step motor timer function
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param now - 64-bit timestamp that represents current time
/// \param is_irq_handler - non-zero if function is called in context of IRQ handler. Zero if function is called in other
///        than IRQ handler context. This parameter is used to detect when sequence of timer events is started. For the very
///        first time this function is being called from #step_motor_dev_execute() function. All subsequent calls are made in
///        IRQ handler context.
void step_motor_timer_event(struct StepMotorDevice* dev, uint64_t now, uint8_t is_irq_handler);

/// \brief Starts execution of stepper motor device commands
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
void step_motor_dev_start(struct StepMotorDevice* dev);

/// \brief Stops execution of stepper motor device commands
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \details this function doest full reset, for details see #step_motor_dev_reset()
void step_motor_dev_stop(struct StepMotorDevice* dev);

/// \brief Reset stepper motor device
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param full_reset - non-zero instructs to stop and clear all command buffers and switch stepper motors to their
///        default state (full reset). Zero instructs to switch motors into default state, but all commands in buffers are
///        intact, current state is also preserved. (partial reset). In other way partial reset behaves as pause.
void step_motor_dev_reset(struct StepMotorDevice* dev, uint8_t full_reset);

/// \brief Helper function that wraps initialization of the stepper motor driver line.
/// \param mdescr - pointer to #StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param linenum - line index (see @ref group_step_motor_dev_motor_lines)
void step_motor_init_motor_line(struct StepMotorDescriptor* mdescr, uint8_t linenum);

/// \brief Initialize step motor GPIO lines and external interruppts to default state.
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
void step_motor_init_gpio_and_exti(struct StepMotorDevice* dev);

/// \brief Set motor GPIO lines to default state (including hardware end-stops)
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - index of the motor to initialize
void step_motor_set_default(struct StepMotorDevice* dev, uint8_t mindex);

/// \brief Helper function that sets stepper motor line to specified value
/// \param mdescr - pointer to #StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param linenum - line index (see @ref group_step_motor_dev_motor_lines)
/// \param value - value to set (0 or non-zero)
void step_motor_set_line(struct StepMotorDescriptor* mdescr, uint8_t linenum, BitAction value);

/// \brief Helper function that sets stepper motor device StepMotorDevStatus#status
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mask - mask for the flags to be set
/// \param flags - values to be set
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void step_motor_set_dev_status(struct StepMotorDevice* dev, uint8_t mask, uint8_t flags);

/// \brief Helper function that updates StepMotorContext#pos_change_by_step
/// \param mdescr - pointer to #StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #StepMotorStatus structure
/// \param mcontext - pointer to the corresponding stepper motor #StepMotorContext structure
/// \return 0 if success, non-zero indicates an error due to incorrect micro stepping value
uint8_t step_motor_update_pos_change_by_step(struct StepMotorDescriptor* mdescr,
                                             struct StepMotorStatus* mstatus,
                                             struct StepMotorContext* mcontext);

/// \brief Helper function that suspends stepper motor and switch it's #STEP_MOTOR_LINE_ENABLE and #STEP_MOTOR_LINE_SLEEP
///        lines into their default state
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mdescr - pointer to #StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #StepMotorStatus structure
/// \param error - non-zero if motor is suspending because of error, otherwise zero
/// \details This function is called in context of #step_motor_timer_event(), it may become inline in future
/// \details May suspend other motors if stepper motor is configured to affect other motors.
void step_motor_suspend_motor(struct StepMotorDevice* dev,
                              struct StepMotorDescriptor* mdescr,
                              struct StepMotorStatus* mstatus,
                              uint8_t error);

/// \brief Helper function that resumes stepper motor and switch it's #STEP_MOTOR_LINE_ENABLE and #STEP_MOTOR_LINE_SLEEP
///        lines into their preserved state
/// \param mdescr - pointer to #StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #StepMotorStatus structure
void step_motor_resume_motor(struct StepMotorDescriptor* mdescr, struct StepMotorStatus* mstatus);

/// \brief Helper function that implements similar logic for handling hardware end-stops, software position limits and
///        faults.
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mstatus - pointer to the corresponding stepper motor #StepMotorStatus structure
/// \param ignore_flag - specifies flag within StepMotorStatus#motor_state that indicates to ignore selected event.
///        Must be one of the following: #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE,
///        #STEP_MOTOR_CONFIG_FAILURE_IGNORE
/// \param all_flag - specifies flag within StepMotorStatus#motor_state that indicates this event to affect all motors.
///        Must be one of the following: #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL,
///        #STEP_MOTOR_CONFIG_FAILURE_ALL
/// \return non-zero if motor (or motors) was suspended, otherwise zero
uint8_t step_motor_handle_alarm(struct StepMotorDevice* dev, struct StepMotorStatus* mstatus, uint32_t ignore_flag, uint32_t all_flag);

/// \brief #ON_COMMAND callback for all stepper motor devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
/// \return Result of the operation as communication status.
uint8_t step_motor_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief #ON_SYNC callback for all stepper motor devices
/// \param cmd_byte - command byte received from software. Corresponds to CommCommandHeader#command_byte
/// \param length - total length of the received data during the i2c transmittion.
/// \return Result of the operation as communication status.
uint8_t step_motor_dev_sync(uint8_t cmd_byte, uint16_t length);

/// \brief Prepares motor for the movement
/// \param dev_index - stepper motor device index
/// \param mindex - stepper motor index
/// \param cmd - command this motor should be prepared for (must be either #STEP_MOTOR_MOVE or #STEP_MOTOR_MOVE_NON_STOP)
/// \return non-zero if command should stop's it's execution, otherwise zero
/// \details Preparation includes configuration of hardware end-stops or software limits.
uint8_t step_motor_prepare_for_move(uint8_t dev_index, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Stepper motor device fault handler to be called by @ref group_exti_hub_group
/// \param clock - timestamp corresponding the moment when IRQ handler was called
/// \param ctx - context. For stepper motor devices context is 32 bit value used to encode device and motor indexes.
///        For details see: #STEP_MOTOR_EXTI_DEV_INDEX and #STEP_MOTOR_EXTI_MINDEX
void step_motor_fault_handler(uint64_t clock, volatile void* ctx);

/// \brief Stepper motor device clockwise end-stop handler to be called by @ref group_exti_hub_group
/// \param clock - timestamp corresponding the moment when IRQ handler was called
/// \param ctx - context. For stepper motor devices context is 32 bit value used to encode device and motor indexes.
///        For details see: #STEP_MOTOR_EXTI_DEV_INDEX and #STEP_MOTOR_EXTI_MINDEX
void step_motor_cw_end_stop_handler(uint64_t clock, volatile void* ctx);

/// \brief Stepper motor device counter clockwise end-stop handler to be called by @ref group_exti_hub_group
/// \param clock - timestamp corresponding the moment when IRQ handler was called
/// \param ctx - context. For stepper motor devices context is 32 bit value used to encode device and motor indexes.
///        For details see: #STEP_MOTOR_EXTI_DEV_INDEX and #STEP_MOTOR_EXTI_MINDEX
void step_motor_ccw_end_stop_handler(uint64_t clock, volatile void* ctx);

/// \def MOTOR_DEVICE
/// \brief This macro converts device index into pointer to #StepMotorDevice structure corresponding to selected
///        stepper motor device
/// \param dev_index - device index
/// \return pointer to #StepMotorDevice structure
#define MOTOR_DEVICE(dev_index)     ((struct StepMotorDevice*)(g_step_motor_devs[(dev_index)]))

/// \def MOTOR_DEV_STATUS
/// \brief This macro returns pointer to #StepMotorDevStatus structure corresponding to selected stepper motor
///        device status
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \return pointer to #StepMotorDevStatus structure
#define MOTOR_DEV_STATUS(dev)       ((struct StepMotorDevStatus*)((dev)->priv_data.internal_status))

/// \def MOTOR_DEV_PRIV_DATA
/// \brief This macro returns pointer to #StepMotorDevPrivData structure corresponding to selected stepper motor
///        device private data
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \return pointer to #StepMotorDevPrivData structure
#define MOTOR_DEV_PRIV_DATA(dev)    ((struct StepMotorDevPrivData*)(&((dev)->priv_data)))

/// \def MOTOR_DESCR
/// \brief This macro returns pointer to #StepMotorDescriptor structure corresponding to selected stepper motor
///        description
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #StepMotorDescriptor structure
#define MOTOR_DESCR(dev, mindex)    ((struct StepMotorDescriptor*)(*((dev)->motor_descriptor + (mindex))))

/// \def MOTOR_STATUS
/// \brief This macro returns pointer to #StepMotorStatus structure corresponding to selected stepper motor status
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #StepMotorStatus structure
#define MOTOR_STATUS(dev, mindex)   ((struct StepMotorStatus*)((dev)->priv_data.internal_status->mstatus + (mindex)))

/// \def MOTOR_CONTEXT
/// \brief This macro returns pointer to #StepMotorContext structure corresponding to selected stepper motor context
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #StepMotorContext structure
#define MOTOR_CONTEXT(dev, mindex)  ((struct StepMotorContext*)((dev)->motor_context + (mindex)))

/// \def MOTOR_CMD
/// \brief This macro returns pointer to #StepMotorCmd structure corresponding to selected stepper motor current
///        command (StepMotorCmd#current_cmd)
/// \param dev - pointer to #StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #StepMotorCmd structure
#define MOTOR_CMD(dev, mindex)      ((struct StepMotorCmd*)(&((dev)->motor_context[(mindex)].current_cmd)))

/// \def STEP_MOTOR_EXTI_PARAM
/// \brief This macro is used to encode device index and motor index into 32 bit value to be used as context for
///        @ref group_exti_hub_group
/// \param dev_index - device index
/// \param mindex - motor index
/// \return encoded 32 bit context
#define STEP_MOTOR_EXTI_PARAM(dev_index, mindex) (volatile void*)((uint32_t)(((dev_index) << 8) | (mindex)))

/// \def STEP_MOTOR_EXTI_DEV_INDEX
/// \brief This macro returns device index from 32 bit value used as context in @ref group_exti_hub_group calls for stepper
///        motor devices
/// \param param - 32 bit context value
/// \return device index
#define STEP_MOTOR_EXTI_DEV_INDEX(param) (uint8_t)((((uint32_t)param) >> 8) & 0xFF)

/// \def STEP_MOTOR_EXTI_MINDEX
/// \brief This macro returns motor index from 32 bit value used as context in @ref group_exti_hub_group calls for stepper
///        motor devices
/// \param param - 32 bit context value
/// \return motor index
#define STEP_MOTOR_EXTI_MINDEX(param)    (uint8_t)(((uint32_t)param) & 0xFF)


/// @}
/// @}
#endif

