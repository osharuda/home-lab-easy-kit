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

#include "i2c_bus.h"

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

