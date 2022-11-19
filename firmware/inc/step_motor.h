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
#include "i2c_bus.h"


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

extern const StepMotorMicrostepTables g_step_motor_microstep_tables;

/// \brief This function initializes all configured stepper motor devices
void step_motor_init(void);

/// \brief Step motor timer function
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param now - 64-bit timestamp that represents current time
/// \param is_irq_handler - non-zero if function is called in context of IRQ handler. Zero if function is called in other
///        than IRQ handler context. This parameter is used to detect when sequence of timer events is started. For the very
///        first time this function is being called from #step_motor_dev_execute() function. All subsequent calls are made in
///        IRQ handler context.
void step_motor_timer_event(volatile PStepMotorDevice dev, uint64_t now, uint8_t is_irq_handler);

/// \brief Starts execution of stepper motor device commands
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
void step_motor_dev_start(volatile PStepMotorDevice dev);

/// \brief Stops execution of stepper motor device commands
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \details this function doest full reset, for details see #step_motor_dev_reset()
void step_motor_dev_stop(volatile PStepMotorDevice dev);

/// \brief Reset stepper motor device
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param full_reset - non-zero instructs to stop and clear all command buffers and switch stepper motors to their
///        default state (full reset). Zero instructs to switch motors into default state, but all commands in buffers are
///        intact, current state is also preserved. (partial reset). In other way partial reset behaves as pause.
void step_motor_dev_reset(volatile PStepMotorDevice dev, uint8_t full_reset);

/// \brief Helper function that wraps initialization of the stepper motor driver line.
/// \param mdescr - pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param linenum - line index (see @ref group_step_motor_dev_motor_lines)
void step_motor_init_motor_line(volatile StepMotorDescriptor* mdescr, uint8_t linenum);

/// \brief Initialize step motor GPIO lines and external interruppts to default state.
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
void step_motor_init_gpio_and_exti(volatile PStepMotorDevice dev);

/// \brief Set motor GPIO lines to default state (including hardware end-stops)
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - index of the motor to initialize
void step_motor_set_default(volatile PStepMotorDevice dev, uint8_t mindex);

/// \brief Helper function that sets stepper motor line to specified value
/// \param mdescr - pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param linenum - line index (see @ref group_step_motor_dev_motor_lines)
/// \param value - value to set (0 or non-zero)
void step_motor_set_line(volatile StepMotorDescriptor* mdescr, uint8_t linenum, BitAction value);

/// \brief Helper function that sets stepper motor device tag_StepMotorDevStatus#status
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mask - mask for the flags to be set
/// \param flags - values to be set
/// \warning This function disables interrupts with #DISABLE_IRQ macro. #DISABLE_IRQ may not be used recursively, thus don't
///          use this function when interrupts are disabled.
void step_motor_set_dev_status(volatile PStepMotorDevice dev, uint8_t mask, uint8_t flags);

/// \brief Helper function that updates tag_StepMotorContext#pos_change_by_step
/// \param mdescr - pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #tag_StepMotorStatus structure
/// \param mcontext - pointer to the corresponding stepper motor #tag_StepMotorContext structure
/// \return 0 if success, non-zero indicates an error due to incorrect micro stepping value
uint8_t step_motor_update_pos_change_by_step(volatile StepMotorDescriptor* mdescr,
                                             volatile PStepMotorStatus mstatus,
                                             volatile PStepMotorContext mcontext);

/// \brief Helper function that suspends stepper motor and switch it's #STEP_MOTOR_LINE_ENABLE and #STEP_MOTOR_LINE_SLEEP
///        lines into their default state
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mdescr - pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #tag_StepMotorStatus structure
/// \param error - non-zero if motor is suspending because of error, otherwise zero
/// \details This function is called in context of #step_motor_timer_event(), it may become inline in future
/// \details May suspend other motors if stepper motor is configured to affect other motors.
void step_motor_suspend_motor(volatile PStepMotorDevice dev,
                              volatile StepMotorDescriptor* mdescr,
                              volatile PStepMotorStatus mstatus,
                              uint8_t error);

/// \brief Helper function that resumes stepper motor and switch it's #STEP_MOTOR_LINE_ENABLE and #STEP_MOTOR_LINE_SLEEP
///        lines into their preserved state
/// \param mdescr - pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor configuration
/// \param mstatus - pointer to the corresponding stepper motor #tag_StepMotorStatus structure
void step_motor_resume_motor(volatile StepMotorDescriptor* mdescr, volatile PStepMotorStatus mstatus);

/// \brief Helper function that implements similar logic for handling hardware end-stops, software position limits and
///        faults.
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mstatus - pointer to the corresponding stepper motor #tag_StepMotorStatus structure
/// \param ignore_flag - specifies flag within tag_StepMotorStatus#motor_state that indicates to ignore selected event.
///        Must be one of the following: #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE,
///        #STEP_MOTOR_CONFIG_FAILURE_IGNORE
/// \param all_flag - specifies flag within tag_StepMotorStatus#motor_state that indicates this event to affect all motors.
///        Must be one of the following: #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL,
///        #STEP_MOTOR_CONFIG_FAILURE_ALL
/// \return non-zero if motor (or motors) was suspended, otherwise zero
uint8_t step_motor_handle_alarm(volatile PStepMotorDevice dev, volatile PStepMotorStatus mstatus, uint32_t ignore_flag, uint32_t all_flag);

/// \brief #ON_COMMAND callback for all stepper motor devices
/// \param cmd_byte - command byte received from software. Corresponds to tag_CommCommandHeader#command_byte
/// \param data - pointer to data received
/// \param length - length of the received data.
void step_motor_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);

/// \brief Prepares motor for the movement
/// \param dev_index - stepper motor device index
/// \param mindex - stepper motor index
/// \param cmd - command this motor should be prepared for (must be either #STEP_MOTOR_MOVE or #STEP_MOTOR_MOVE_NON_STOP)
/// \return non-zero if command should stop's it's execution, otherwise zero
/// \details Preparation includes configuration of hardware end-stops or software limits.
uint8_t step_motor_prepare_for_move(uint8_t dev_index, uint8_t mindex, StepMotorCmd* cmd);

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
/// \brief This macro converts device index into pointer to #tag_StepMotorDevice structure corresponding to selected
///        stepper motor device
/// \param dev_index - device index
/// \return pointer to #tag_StepMotorDevice structure
#define MOTOR_DEVICE(dev_index)     ((volatile PStepMotorDevice)(g_step_motor_devs[(dev_index)]))

/// \def MOTOR_DEV_STATUS
/// \brief This macro returns pointer to #tag_StepMotorDevStatus structure corresponding to selected stepper motor
///        device status
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \return pointer to #tag_StepMotorDevStatus structure
#define MOTOR_DEV_STATUS(dev)       ((volatile PStepMotorDevStatus)((dev)->status))

/// \def MOTOR_DEV_PRIV_DATA
/// \brief This macro returns pointer to #tag_StepMotorDevPrivData structure corresponding to selected stepper motor
///        device private data
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \return pointer to #tag_StepMotorDevPrivData structure
#define MOTOR_DEV_PRIV_DATA(dev)    ((volatile PStepMotorDevPrivData)(&((dev)->priv_data)))

/// \def MOTOR_DESCR
/// \brief This macro returns pointer to #tag_StepMotorDescriptor structure corresponding to selected stepper motor
///        description
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #tag_StepMotorDescriptor structure
#define MOTOR_DESCR(dev, mindex)    ((volatile StepMotorDescriptor*)(*((dev)->motor_descriptor + (mindex))))

/// \def MOTOR_STATUS
/// \brief This macro returns pointer to #tag_StepMotorStatus structure corresponding to selected stepper motor status
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #tag_StepMotorStatus structure
#define MOTOR_STATUS(dev, mindex)   ((volatile PStepMotorStatus)((dev)->status->mstatus + (mindex)))

/// \def MOTOR_CONTEXT
/// \brief This macro returns pointer to #tag_StepMotorContext structure corresponding to selected stepper motor context
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #tag_StepMotorContext structure
#define MOTOR_CONTEXT(dev, mindex)  ((volatile PStepMotorContext)((dev)->motor_context + (mindex)))

/// \def MOTOR_CMD
/// \brief This macro returns pointer to #tag_StepMotorCmd structure corresponding to selected stepper motor current
///        command (tag_StepMotorCmd#current_cmd)
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \return pointer to #tag_StepMotorCmd structure
#define MOTOR_CMD(dev, mindex)      ((volatile PStepMotorCmd)(&((dev)->motor_context[(mindex)].current_cmd)))

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

