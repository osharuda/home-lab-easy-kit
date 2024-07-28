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
 *   \brief Generated include header of software part for Stepper Motor device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <cstddef>
#include <stdint.h>

{__STEP_MOTOR_SHARED_HEADER__}

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

/// \struct StepMotorDescriptor
/// \brief Describes stepper motor default configuration
struct __attribute__ ((aligned)) StepMotorDescriptor {{
    uint32_t      config_flags;                 ///< flags used to describe default step motor behaviour. Corresponds to tag_StepMotorStatus#motor_state. See @ref group_step_motor_dev_configuration
    uint16_t      buffer_size;                  ///< stepper motor command buffer size in bytes.
    uint64_t      default_speed;                ///< stepper motor default speed. tag_StepMotorDescriptor#default_speed is a number of microseconds between step pulses. It doesn't take into account microstepping.
    uint8_t       motor_driver;                 ///< stepper motor driver type. One of the STEP_MOTOR_DRIVER_XXX values. See @ref group_step_motor_dev_microstep_tables
    int64_t       cw_sft_limit;                 ///< Default software limit for stepper motor position during CW moves. Ignored if hardware end-stop is used
    int64_t       ccw_sft_limit;                ///< Default software limit for stepper motor position during CCW moves. Ignored if hardware end-stop is used
    const char*   motor_name;                       ///< Name of the motor given in JSON configuration file. (available in software part only)
    uint16_t      steps_per_revolution;             ///< Number of steps per revolution for this stepper motor. (available in software part only)
}};
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

/// \struct StepMotorConfig
/// \brief This structure is being used by firmware and software as storage of all information needed.
struct __attribute__ ((aligned)) StepMotorConfig {{
	const char*	     dev_name;      ///< Stepper motor device name. (available in firmware part only). Do not change this field.
    const struct StepMotorDescriptor** motor_descriptor; ///< Array of the pointers to #tag_StepMotorDescriptor for each stepper motor controlled by the device. Do not change this field.
    uint8_t          motor_count;   ///< Number of stepper motors controled by this device. Do not change this field.
    uint8_t          dev_id;        ///< Device ID for the stepper motor device. Do not change this field.
}};
/// @}}

