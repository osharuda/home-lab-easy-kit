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




#ifdef STEP_MOTOR_FIRMWARE
#define MDESCR_SPECIFIER volatile
#elif !defined STEP_MOTOR_FIRMWARE
#define MDESCR_SPECIFIER const
#endif

/// \addtogroup group_step_motor_dev
/// @{

/// \defgroup group_step_motor_dev_microstep_tables Micro stepping
/// \brief Micro stepping details
/// @{
/// \page page_step_motor_dev_microstep_tables
/// \tableofcontents
///
/// \section sect_step_motor_dev_microstep_tables_01 Microstep notes
/// Microsteps allow stepper motor driver to perform a fraction of step by a step pulse. Minimal microstep for A4998 is
/// 1/16, for DRV8825 1/32.
///
/// This is a reason why stepper motor position is calculated in minimal steps possible - in units of 1/32 of the step.
/// Such an approach allow to control and track fractions of the step.
///
/// Microstep data is taken from A4998 and DRV8825 documentation and put here as #STEP_MOTOR_MICROSTEP_TABLE macro. You
/// may see there is an entry for #STEP_MOTOR_DRIVER_UNKNOWN. This entry is organized as unknown driver doesn't support
/// microsteps.
///

/// \typedef StepMotorMicrostepTable
/// \brief typedef for single microstep table row dedicated to particular step motor driver
typedef uint8_t StepMotorMicrostepTable[8];

/// \typedef StepMotorMicrostepTables
/// \brief typedef for whole microstep table
typedef StepMotorMicrostepTable StepMotorMicrostepTables[];

/// \def STEP_MOTOR_DRIVER_UNKNOWN
/// \brief Defines some unknown and unsupported stepper motor drive.
#define STEP_MOTOR_DRIVER_UNKNOWN   (uint8_t)(0)

/// \def STEP_MOTOR_DRIVER_A4998
/// \brief Defines A4998 stepper motor drive.
#define STEP_MOTOR_DRIVER_A4998     (uint8_t)(1)

/// \def STEP_MOTOR_DRIVER_DRV8825
/// \brief Defines DRV8825 stepper motor drive.
#define STEP_MOTOR_DRIVER_DRV8825   (uint8_t)(2)

/// \def STEP_MOTOR_FULL_STEP
/// \brief Defines full step. Corresponds to tag_StepMotorStatus#pos change by +(-) 32
#define STEP_MOTOR_FULL_STEP          (5)

/// \def STEP_MOTOR_FULL_STEP_DIV_2
/// \brief Defines 1/2 step. Corresponds to tag_StepMotorStatus#pos change by +(-) 16
#define STEP_MOTOR_FULL_STEP_DIV_2    (4)

/// \def STEP_MOTOR_FULL_STEP_DIV_4
/// \brief Defines 1/4 step. Corresponds to tag_StepMotorStatus#pos change by +(-) 8
#define STEP_MOTOR_FULL_STEP_DIV_4    (3)

/// \def STEP_MOTOR_FULL_STEP_DIV_8
/// \brief Defines 1/8 step. Corresponds to tag_StepMotorStatus#pos change by +(-) 4
#define STEP_MOTOR_FULL_STEP_DIV_8    (2)

/// \def STEP_MOTOR_FULL_STEP_DIV_16
/// \brief Defines 1/16 step. Corresponds to tag_StepMotorStatus#pos change by +(-) 2
#define STEP_MOTOR_FULL_STEP_DIV_16   (1)

/// \def STEP_MOTOR_FULL_STEP_DIV_32
/// \brief Defines 1/32 step. Corresponds to tag_StepMotorStatus#pos change by +(-) 1
#define STEP_MOTOR_FULL_STEP_DIV_32   (0)

/// \def STEP_MOTOR_BAD_STEP
/// \brief Indicates this configuration of microstep values is not supported by stepper motor driver
#define STEP_MOTOR_BAD_STEP           (0xFF)


/// \def STEP_MOTOR_MICROSTEP_TABLE
/// \brief Defines microstep table. First index corresponds to step motor driver type. Second index corresponds amount of bits to shift 1 left in order to get full step, which consist of 32 microsteps.
#define STEP_MOTOR_MICROSTEP_TABLE  {{STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP},/*unknown driver type always work in full step mode*/ \
                                    {STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP_DIV_2,STEP_MOTOR_FULL_STEP_DIV_4,STEP_MOTOR_FULL_STEP_DIV_8,STEP_MOTOR_BAD_STEP,STEP_MOTOR_BAD_STEP,STEP_MOTOR_BAD_STEP,STEP_MOTOR_FULL_STEP_DIV_16},/*A4998*/ \
                                    {STEP_MOTOR_FULL_STEP,STEP_MOTOR_FULL_STEP_DIV_2,STEP_MOTOR_FULL_STEP_DIV_4,STEP_MOTOR_FULL_STEP_DIV_8,STEP_MOTOR_FULL_STEP_DIV_16,STEP_MOTOR_FULL_STEP_DIV_32,STEP_MOTOR_FULL_STEP_DIV_32,STEP_MOTOR_FULL_STEP_DIV_32}} /*Drv8825*/

/// \def STEP_MOTOR_MICROSTEP_DELTA
/// \brief Calculates a delta value that should be used to increment/decrement tag_StepMotorStatus#pos value per one step
/// \param step_shift - number of bits to shift 1 left in order to get required fraction of a full step, which consist of 32 microsteps.
/// \return delta value to change tag_StepMotorStatus#pos
#define STEP_MOTOR_MICROSTEP_DELTA(step_shift)          (1 << (step_shift))

/// \def STEP_MOTOR_MICROSTEP_DIVIDER
/// \brief Returns microstep divider
/// \param value - number of bits to shift 1 left in order to get required fraction of a full step, which consist of 32 microsteps.
/// \return divider that corresponds passed value
#define STEP_MOTOR_MICROSTEP_DIVIDER(value)             (32 >> (value))

/// \def STEP_MOTOR_MICROSTEP_VALUE
/// \brief Returns number of bits to shift 1 left in order to get required fraction of a full step, which consist of 32 microsteps.
/// \param table - pointer to the microstep table #STEP_MOTOR_MICROSTEP_TABLE
/// \param m1 - M1 microstep value
/// \param m2 - M2 microstep value
/// \param m3 - M3 microstep value
/// \return number of bits to shift 1 left in order to get required fraction of a full step, which consist of 32 microsteps. or #STEP_MOTOR_BAD_STEP in the case of unsupported microstep value
#define STEP_MOTOR_MICROSTEP_VALUE(table, m1, m2, m3)   ((table)[(m1) | ((m2) << 1) | ((m3) << 2)])

/// \def STEP_MOTOR_MICROSTEP_VALUE_TO_STATUS
/// \brief Converts parameter passed with #STEP_MOTOR_SET_MICROSTEP command into motor state flag defined by @ref group_step_motor_dev_configuration
/// \param msval - value passed with #STEP_MOTOR_SET_MICROSTEP command
/// \return flags set defined by @ref group_step_motor_dev_configuration
#define STEP_MOTOR_MICROSTEP_VALUE_TO_STATUS(msval)     ((uint32_t)(((msval) & 0b00000111) << STEP_MOTOR_M1_DEFAULT_OFFSET))

/// \def STEP_MOTOR_MICROSTEP_STATUS_TO_VALUE
/// \brief Converts motor state flags defined by @ref group_step_motor_dev_configuration into command parameter to be passed with #STEP_MOTOR_SET_MICROSTEP command.
/// \param status - flags defined by @ref group_step_motor_dev_configuration
/// \return parameter to be passed with #STEP_MOTOR_SET_MICROSTEP command.
#define STEP_MOTOR_MICROSTEP_STATUS_TO_VALUE(status)    (((status) >> STEP_MOTOR_M1_DEFAULT_OFFSET) & 0b00000111)

/// @}

/// \defgroup group_step_motor_dev_configuration Configuration/status flags
/// \brief Stepper motor configuration/status flags defines and explanation
/// @{
/// \page page_step_motor_dev_configuration
/// \tableofcontents
///
/// \section sect_step_motor_dev_configuration_01 Stepper motor configuration flags
/// It is important to state the configuration flags are used for several purposes:
/// 1. Define which stepper motor driver lines are used, which are not:
///    - Microstep lines: #STEP_MOTOR_M1_IN_USE, #STEP_MOTOR_M2_IN_USE, #STEP_MOTOR_M3_IN_USE.
///    - Power state lines: #STEP_MOTOR_ENABLE_IN_USE, #STEP_MOTOR_RESET_IN_USE, #STEP_MOTOR_SLEEP_IN_USE.
///    - Fault and end-stop lines: #STEP_MOTOR_FAULT_IN_USE, #STEP_MOTOR_CWENDSTOP_IN_USE, #STEP_MOTOR_CCWENDSTOP_IN_USE.
///      Also, logical levels for these lines: #STEP_MOTOR_FAULT_ACTIVE_HIGH, #STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH,
///      #STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH.
///    - Direction line: #STEP_MOTOR_DIR_IN_USE.
/// 2. Define default state of some stepper motor lines:
///    - Microstep defaults: #STEP_MOTOR_M1_DEFAULT, #STEP_MOTOR_M2_DEFAULT, #STEP_MOTOR_M3_DEFAULT.
///    - Default power state: #STEP_MOTOR_DISABLE_DEFAULT, #STEP_MOTOR_WAKEUP_DEFAULT.
///    - Default direction: #STEP_MOTOR_DIRECTION_CW.
/// 3. To configure behaviour in the case hardware/software end-stop or fault is triggered.
///    - To configure reaction on failure: #STEP_MOTOR_CONFIG_FAILURE_IGNORE, #STEP_MOTOR_CONFIG_FAILURE_ALL.
///    - To configure reaction on errors during stepper motor command execution: #STEP_MOTOR_CONFIG_ERROR_ALL.
///    - To configure reaction on end-stops: #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE,
///      #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL.
/// 4. To store stepper motor state flags:
///    - Stepper motor state flags: #STEP_MOTOR_ERROR, #STEP_MOTOR_DONE, #STEP_MOTOR_SUSPENDING.
///    - End-stops and failure flags: #STEP_MOTOR_FAILURE, #STEP_MOTOR_CW_ENDSTOP_TRIGGERED, #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED.
///

/// \def STEP_MOTOR_M1_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M1_IN_USE flag
#define STEP_MOTOR_M1_IN_USE_OFFSET              (0)
/// \def STEP_MOTOR_M1_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for M1 signal
#define STEP_MOTOR_M1_IN_USE                     ((uint32_t)1 << STEP_MOTOR_M1_IN_USE_OFFSET)

/// \def STEP_MOTOR_M2_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M2_IN_USE flag
#define STEP_MOTOR_M2_IN_USE_OFFSET              (1)
/// \def STEP_MOTOR_M2_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for M2 signal
#define STEP_MOTOR_M2_IN_USE                     ((uint32_t)1 << STEP_MOTOR_M2_IN_USE_OFFSET)

/// \def STEP_MOTOR_M3_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M3_IN_USE flag
#define STEP_MOTOR_M3_IN_USE_OFFSET              (2)
/// \def STEP_MOTOR_M3_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for M3 signal
#define STEP_MOTOR_M3_IN_USE                     ((uint32_t)1 << STEP_MOTOR_M3_IN_USE_OFFSET)

/// \def STEP_MOTOR_ENABLE_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_ENABLE_IN_USE flag
#define STEP_MOTOR_ENABLE_IN_USE_OFFSET          (3)
/// \def STEP_MOTOR_ENABLE_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for ENABLE signal
#define STEP_MOTOR_ENABLE_IN_USE                 ((uint32_t)1 << STEP_MOTOR_ENABLE_IN_USE_OFFSET)

/// \def STEP_MOTOR_RESET_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_RESET_IN_USE flag
#define STEP_MOTOR_RESET_IN_USE_OFFSET           (4)
/// \def STEP_MOTOR_RESET_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for RESET signal
#define STEP_MOTOR_RESET_IN_USE                  ((uint32_t)1 << STEP_MOTOR_RESET_IN_USE_OFFSET)

/// \def STEP_MOTOR_SLEEP_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_SLEEP_IN_USE flag
#define STEP_MOTOR_SLEEP_IN_USE_OFFSET           (5)
/// \def STEP_MOTOR_SLEEP_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for SLEEP signal
#define STEP_MOTOR_SLEEP_IN_USE                  ((uint32_t)1 << STEP_MOTOR_SLEEP_IN_USE_OFFSET)

/// \def STEP_MOTOR_FAULT_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_FAULT_IN_USE flag
#define STEP_MOTOR_FAULT_IN_USE_OFFSET           (6)
/// \def STEP_MOTOR_FAULT_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for FAULT signal
/// \note Fault line is not available in A4998 stepper motor driver
#define STEP_MOTOR_FAULT_IN_USE                  ((uint32_t)1 << STEP_MOTOR_FAULT_IN_USE_OFFSET)

/// \def STEP_MOTOR_CWENDSTOP_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CWENDSTOP_IN_USE flag
#define STEP_MOTOR_CWENDSTOP_IN_USE_OFFSET       (7)
/// \def STEP_MOTOR_CWENDSTOP_IN_USE
/// \brief This flag indicates hardware endstop for CW moves is used and connected to GPIO line
#define STEP_MOTOR_CWENDSTOP_IN_USE              ((uint32_t)1 << STEP_MOTOR_CWENDSTOP_IN_USE_OFFSET)

/// \def STEP_MOTOR_CCWENDSTOP_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CCWENDSTOP_IN_USE flag
#define STEP_MOTOR_CCWENDSTOP_IN_USE_OFFSET      (8)
/// \def STEP_MOTOR_CCWENDSTOP_IN_USE
/// \brief This flag indicates hardware endstop for CCW moves is used and connected to GPIO line
#define STEP_MOTOR_CCWENDSTOP_IN_USE             ((uint32_t)1 << STEP_MOTOR_CCWENDSTOP_IN_USE_OFFSET)

/// \def STEP_MOTOR_DIR_IN_USE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_DIR_IN_USE flag
#define STEP_MOTOR_DIR_IN_USE_OFFSET             (9)
/// \def STEP_MOTOR_DIR_IN_USE
/// \brief This flag indicates stepper motor driver uses GPIO line for DIRECTION
#define STEP_MOTOR_DIR_IN_USE                    ((uint32_t)1 << STEP_MOTOR_DIR_IN_USE_OFFSET)

/// \def STEP_MOTOR_FAULT_ACTIVE_HIGH_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_FAULT_ACTIVE_HIGH flag
#define STEP_MOTOR_FAULT_ACTIVE_HIGH_OFFSET      (10)
/// \def STEP_MOTOR_FAULT_ACTIVE_HIGH
/// \brief This flag says logical 1 (high level) indicates FAULT is active
#define STEP_MOTOR_FAULT_ACTIVE_HIGH             ((uint32_t)1 << STEP_MOTOR_FAULT_ACTIVE_HIGH_OFFSET)

/// \def STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH flag
#define STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH_OFFSET  (11)
/// \def STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH
/// \brief This flag says logical 1 (high level) indicates hardware CW endstop is triggered
#define STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH         ((uint32_t)1 << STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH_OFFSET)

/// \def STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH flag
#define STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH_OFFSET (12)
/// \def STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH
/// \brief This flag says logical 1 (high level) indicates hardware CCW endstop is triggered
#define STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH        ((uint32_t)1 << STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH_OFFSET)

/// \def STEP_MOTOR_M1_DEFAULT_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M1_DEFAULT flag
#define STEP_MOTOR_M1_DEFAULT_OFFSET             (13)
/// \def STEP_MOTOR_M1_DEFAULT
/// \brief This flag says M1 default value is 1, if not set M1 default value is 0
#define STEP_MOTOR_M1_DEFAULT                    ((uint32_t)1 << STEP_MOTOR_M1_DEFAULT_OFFSET)

/// \def STEP_MOTOR_M2_DEFAULT_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M2_DEFAULT flag
#define STEP_MOTOR_M2_DEFAULT_OFFSET             (14)
/// \def STEP_MOTOR_M2_DEFAULT
/// \brief This flag says M2 default value is 1, if not set M2 default value is 0
#define STEP_MOTOR_M2_DEFAULT                    ((uint32_t)1 << STEP_MOTOR_M2_DEFAULT_OFFSET)

/// \def STEP_MOTOR_M3_DEFAULT_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_M3_DEFAULT flag
#define STEP_MOTOR_M3_DEFAULT_OFFSET             (15)
/// \def STEP_MOTOR_M3_DEFAULT
/// \brief This flag says M3 default value is 1, if not set M3 default value is 0
#define STEP_MOTOR_M3_DEFAULT                    ((uint32_t)1 << STEP_MOTOR_M3_DEFAULT_OFFSET)

/// \def STEP_MOTOR_DIRECTION_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_DIRECTION_CW flag
#define STEP_MOTOR_DIRECTION_OFFSET              (16)
/// \def STEP_MOTOR_DIRECTION_CW
/// \brief This flag says motor direction is CW, if not set motor direction is CCW
#define STEP_MOTOR_DIRECTION_CW                  ((uint32_t)1 << STEP_MOTOR_DIRECTION_OFFSET)

/// \def STEP_MOTOR_DISABLE_DEFAULT_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_DISABLE_DEFAULT flag
#define STEP_MOTOR_DISABLE_DEFAULT_OFFSET        (17)
/// \def STEP_MOTOR_DISABLE_DEFAULT
/// \brief This flag says motor is DISABLED by default, if not set motor is ENABLED
#define STEP_MOTOR_DISABLE_DEFAULT               ((uint32_t)1 << STEP_MOTOR_DISABLE_DEFAULT_OFFSET)

/// \def STEP_MOTOR_WAKEUP_DEFAULT_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_WAKEUP_DEFAULT flag
#define STEP_MOTOR_WAKEUP_DEFAULT_OFFSET         (18)
/// \def STEP_MOTOR_WAKEUP_DEFAULT
/// \brief This flag says motor is WAKEUP by default, if not set motor is in SLEEP mode
#define STEP_MOTOR_WAKEUP_DEFAULT                ((uint32_t)1 << STEP_MOTOR_WAKEUP_DEFAULT_OFFSET)

/// \def STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_FAILURE_IGNORE flag
#define STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET     (19)
/// \def STEP_MOTOR_CONFIG_FAILURE_IGNORE
/// \brief This flag says motor failure should be ignored
#define STEP_MOTOR_CONFIG_FAILURE_IGNORE            ((uint32_t)1 << STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET)

/// \def STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE flag
#define STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE_OFFSET  (20)
/// \def STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE
/// \brief This flag says motor CW endstop (both, hardware or software) should be ignored
#define STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE         ((uint32_t)1 << STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE_OFFSET)

/// \def STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE flag
#define STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE_OFFSET (21)
/// \def STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE
/// \brief This flag says motor CCW endstop (both, hardware or software) should be ignored
#define STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE        ((uint32_t)1 << STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE_OFFSET)

/// \def STEP_MOTOR_CONFIG_FAILURE_ALL_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_FAILURE_ALL flag
#define STEP_MOTOR_CONFIG_FAILURE_ALL_OFFSET        (22)
/// \def STEP_MOTOR_CONFIG_FAILURE_ALL
/// \brief This flag says motor failure should suspend all motors for device
#define STEP_MOTOR_CONFIG_FAILURE_ALL               ((uint32_t)1 << STEP_MOTOR_CONFIG_FAILURE_ALL_OFFSET)

/// \def STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL flag
#define STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL_OFFSET     (23)
/// \def STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL
/// \brief This flag says motor CW endstop (both, software and hardware) should suspend all motors for device
#define STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL            ((uint32_t)1 << STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL_OFFSET)

/// \def STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL flag
#define STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL_OFFSET    (24)
/// \def STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL
/// \brief This flag says motor CCW endstop (both, software and hardware) should suspend all motors for device
#define STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL           ((uint32_t)1 << STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL_OFFSET)

/// \def STEP_MOTOR_CONFIG_ERROR_ALL_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CONFIG_ERROR_ALL flag
#define STEP_MOTOR_CONFIG_ERROR_ALL_OFFSET          (25)
/// \def STEP_MOTOR_CONFIG_ERROR_ALL
/// \brief This flag says ERROR returned by motor command execution should suspend all motors for device
/// \note If this flag is not set just the same motor is suspended. There is no way to ignore ERROR
#define STEP_MOTOR_CONFIG_ERROR_ALL                 ((uint32_t)1 << STEP_MOTOR_CONFIG_ERROR_ALL_OFFSET)

/// \def STEP_MOTOR_FAILURE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_FAILURE flag
#define STEP_MOTOR_FAILURE_OFFSET                   (26)
/// \def STEP_MOTOR_FAILURE
/// \brief This flag indicates motor expirienced a FAILURE
#define STEP_MOTOR_FAILURE                          ((uint32_t)1 << STEP_MOTOR_FAILURE_OFFSET)

/// \def STEP_MOTOR_CW_ENDSTOP_TRIGGERED_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CW_ENDSTOP_TRIGGERED flag
#define STEP_MOTOR_CW_ENDSTOP_TRIGGERED_OFFSET      (27)
/// \def STEP_MOTOR_CW_ENDSTOP_TRIGGERED
/// \brief This flag indicates motor CW endstop (both, hardware or software) is triggered
/// \note It may be cleared when motor moves opposite direction
#define STEP_MOTOR_CW_ENDSTOP_TRIGGERED             ((uint32_t)1 << STEP_MOTOR_CW_ENDSTOP_TRIGGERED_OFFSET)

/// \def STEP_MOTOR_CCW_ENDSTOP_TRIGGERED_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED flag
#define STEP_MOTOR_CCW_ENDSTOP_TRIGGERED_OFFSET     (28)
/// \def STEP_MOTOR_CCW_ENDSTOP_TRIGGERED
/// \brief This flag indicates motor CCW endstop (both, hardware or software) is triggered
/// \note It may be cleared when motor moves opposite direction
#define STEP_MOTOR_CCW_ENDSTOP_TRIGGERED            ((uint32_t)1 << STEP_MOTOR_CCW_ENDSTOP_TRIGGERED_OFFSET)

/// \def STEP_MOTOR_ERROR_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_ERROR flag
#define STEP_MOTOR_ERROR_OFFSET                     (29)
/// \def STEP_MOTOR_ERROR
/// \brief This flag indicates motor command execution returned an ERROR
#define STEP_MOTOR_ERROR                            ((uint32_t)1 << STEP_MOTOR_ERROR_OFFSET)

/// \def STEP_MOTOR_DONE_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_DONE flag
#define STEP_MOTOR_DONE_OFFSET                      (30)
/// \def STEP_MOTOR_DONE
/// \brief This flag indicates motor is suspended
#define STEP_MOTOR_DONE                             ((uint32_t)1 << STEP_MOTOR_DONE_OFFSET)

/// \def STEP_MOTOR_SUSPENDING_OFFSET
/// \brief Defines offset of the #STEP_MOTOR_SUSPENDING flag
#define STEP_MOTOR_SUSPENDING_OFFSET                (31)
/// \def STEP_MOTOR_SUSPENDING
/// \brief This flag indicates motor is suspending. It is used as temporary state to switch power motor lines into default state.
#define STEP_MOTOR_SUSPENDING                       ((uint32_t)1 << STEP_MOTOR_SUSPENDING_OFFSET)

/// \def STEP_MOTOR_CONFIG_MASK
/// \brief Mask for the motor configuration flags (STEP_MOTOR_CONFIG_XXX)
#define STEP_MOTOR_CONFIG_MASK                      (   STEP_MOTOR_CONFIG_FAILURE_IGNORE |      \
                                                        STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE |   \
                                                        STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE |  \
                                                        STEP_MOTOR_CONFIG_FAILURE_ALL |         \
                                                        STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL |      \
                                                        STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL |     \
                                                        STEP_MOTOR_CONFIG_ERROR_ALL)

/// \def STEP_MOTOR_CONFIG_BYTE_MASK
/// \brief This macro specifies motor configuration flags mask as they appear in #STEP_MOTOR_GENERAL_CONFIG command parameter
#define STEP_MOTOR_CONFIG_BYTE_MASK                 ((uint8_t)(STEP_MOTOR_CONFIG_MASK >> STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET))

/// \def STEP_MOTOR_CONFIG_BYTE_TO_FLAGS
/// \brief This macro converts parameter received with #STEP_MOTOR_GENERAL_CONFIG command into #STEP_MOTOR_CONFIG_MASK flags
#define STEP_MOTOR_CONFIG_BYTE_TO_FLAGS(byte)       ((((uint32_t)(byte)) & STEP_MOTOR_CONFIG_BYTE_MASK) << STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET)

/// \def STEP_MOTOR_CONFIG_TO_BYTE
/// \brief This macro converts #STEP_MOTOR_CONFIG_MASK flags into value to be passed with #STEP_MOTOR_GENERAL_CONFIG command
#define STEP_MOTOR_CONFIG_TO_BYTE(cfg)              ((uint8_t)(((cfg) >> STEP_MOTOR_CONFIG_FAILURE_IGNORE_OFFSET) & STEP_MOTOR_CONFIG_BYTE_MASK))

/// \def STEP_MOTOR_IGNORE_ENDSTOP_FLAG
/// \brief This macro calculates either #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE or #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE flag for active endstop.
/// \param dir - is #STEP_MOTOR_SET_DIR_CW for CW, #STEP_MOTOR_SET_DIR_CCW for CCW
#define STEP_MOTOR_IGNORE_ENDSTOP_FLAG(dir)         ((uint32_t)1 << (STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE_OFFSET-(dir)))

/// \def STEP_MOTOR_ALL_ENDSTOP_FLAG
/// \brief This macro calculates either #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL or #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL flag for active endstop.
/// \param dir - is #STEP_MOTOR_SET_DIR_CW for CW, #STEP_MOTOR_SET_DIR_CCW for CCW
#define STEP_MOTOR_ALL_ENDSTOP_FLAG(dir)            ((uint32_t)1 << (STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL_OFFSET-(dir)))

/// \def STEP_MOTOR_DIRECTION
/// \brief This macro takes motor configuration flags and return direction
/// \param motor_state - state of the motor described by @ref group_step_motor_dev_configuration
/// \return #STEP_MOTOR_SET_DIR_CW for CW, #STEP_MOTOR_SET_DIR_CCW for CCW
#define STEP_MOTOR_DIRECTION(motor_state)               (((motor_state) >> STEP_MOTOR_DIRECTION_OFFSET) & 1)

/// \def STEP_MOTOR_DIRECTION_TO_ACTIVE_ENDSTOP
/// \brief This macro takes direction and returns either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED flag that corresponds to direction
/// \param dir - is #STEP_MOTOR_SET_DIR_CW for CW, #STEP_MOTOR_SET_DIR_CCW for CCW
/// \return #STEP_MOTOR_CW_ENDSTOP_TRIGGERED for CW, #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED for CCW direction
#define STEP_MOTOR_DIRECTION_TO_ACTIVE_ENDSTOP(dir)      (STEP_MOTOR_CCW_ENDSTOP_TRIGGERED >> (dir))

/// \def STEP_MOTOR_DIRECTION_TO_INACTIVE_ENDSTOP
/// \brief This macro takes direction and returns either #STEP_MOTOR_CW_ENDSTOP_TRIGGERED or #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED flag that corresponds to opposite direction (inactive endstop)
/// \param dir - is #STEP_MOTOR_SET_DIR_CW for CW, #STEP_MOTOR_SET_DIR_CCW for CCW
/// \return #STEP_MOTOR_CCW_ENDSTOP_TRIGGERED for CW, #STEP_MOTOR_CW_ENDSTOP_TRIGGERED for CCW direction
#define STEP_MOTOR_DIRECTION_TO_INACTIVE_ENDSTOP(dir)    (STEP_MOTOR_CW_ENDSTOP_TRIGGERED << (dir))
/// @}





/// \defgroup group_step_motor_dev_motor_command Motor commands
/// \brief Description for commands being send by software to firmware to control stepper motors
/// @{
/// \page page_step_motor_dev_motor_command
/// \tableofcontents
///
/// \section sect_step_motor_dev_motor_command_01 Stepper motor device specific flags in communication protocol command byte
/// Every device may use custom device specific flags in communication protocol command byte to allow software to pass
/// an additional parameter when it writes data to the device. Stepper motor device utilizes the following flags from communication protocol command byte:
/// #STEP_MOTOR_START, #STEP_MOTOR_STOP .
/// See @ref group_communication_command for details.
///
/// \section sect_step_motor_dev_motor_command_02 Motor command byte structure
/// <table>
/// <caption id="multi_row">Motor command byte structure</caption>
/// <tr><th>Offset<th>Value<th>Group<th>Purpose
/// <tr><td>7<td>128<td>#STEP_MOTOR_SELECT</td><td>If this bit is specified, the rest of the bits specify motor index to be selected. All subsequent motor commands will be put in specified motor's buffer.</td>
/// <tr><td>6<td>64<td rowspan=2>#STEP_MOTOR_PARAM_MASK</td><td rowspan=2>These two bits describe size of argument attached to this motor command. Possible values are #STEP_MOTOR_PARAM_NONE, #STEP_MOTOR_PARAM_8, #STEP_MOTOR_PARAM_16, #STEP_MOTOR_PARAM_64</td>
/// <tr><td>5<td>32</td>
/// <tr><td>4<td>16<td rowspan=2>#STEP_MOTOR_CMD_MASK</td><td rowspan=2>These two bits describe motor command being sent. Possible values are #STEP_MOTOR_GENERAL, #STEP_MOTOR_SET, #STEP_MOTOR_MOVE, #STEP_MOTOR_MOVE_NON_STOP</td>
/// <tr><td>3<td>8</td>
/// <tr><td>2<td>4<td rowspan=3>#STEP_MOTOR_ARG_MASK</td><td rowspan=3>These three bits is a placeholder for short arguments or subcommands</td>
/// <tr><td>1<td>2</td>
/// <tr><td>0<td>1</td>
///
/// \section sect_step_motor_dev_motor_command_03 Motor selection flag
/// Software may mix motor commands for different motors. To make firmware able to distinguish them software should pass special single byte motor command
/// that will specify selected motor index. This motor selection command must have #STEP_MOTOR_SELECT set and the rest bits (0-6) must specify required motor index. Default motor index is 0. This command will not be enqueued into any motor buffer.
///
/// \section sect_step_motor_dev_motor_command_04 Parameter selection flags
/// Some motor commands may have arguments. These arguments may be passed as a part of the motor command byte or may follow the command byte.
/// In order to weaken memory requirements these flags allow to control amount of parameter bytes being attached to the motor command.
/// For example, if command requires 64 bit value, but actual value is 2, than it can be packed into command byte, thus no extra bytes will be sent (and will not occupy memory in motor buffer).
/// It is possible to pack values in motor command byte (must fit #STEP_MOTOR_ARG_MASK) or pass 1,2 or 8 bytes as parameter.
/// This options correspond to  #STEP_MOTOR_PARAM_NONE, #STEP_MOTOR_PARAM_8, #STEP_MOTOR_PARAM_16, #STEP_MOTOR_PARAM_64.
///
/// \section sect_step_motor_dev_motor_command_05 Motor command flags
/// Stepper motor device declares 4 types of commands. #STEP_MOTOR_GENERAL for general commands like reset or issue a wait. #STEP_MOTOR_SET to set some parameters like micro stepping options.
/// #STEP_MOTOR_MOVE to issue some number of step pulses. #STEP_MOTOR_MOVE_NON_STOP to issue steps pulses non stop (without explicit limitation parameter).
/// The last two commands are very familiar, but they are separated as two different commands. The reason is that parameter passing may be very confusing otherwise.
///
/// \section sect_step_motor_dev_motor_command_06 Motor command subtype/argument flags
/// These bits (defined by #STEP_MOTOR_ARG_MASK mask) are multipurpose. From one hand they may be used as command argument, from other hand
/// they may be used as command subtype. For example #STEP_MOTOR_MOVE may use them as argument (if number of step pulses fit #STEP_MOTOR_ARG_MASK).
/// Another example is #STEP_MOTOR_SET motor command with #STEP_MOTOR_SET_DIR_CCW subtype. It uses it to specify command subtype.
///
/// \section sect_step_motor_dev_motor_command_07 General command
/// General command is multipurpose. It uses @ref sect_step_motor_dev_motor_command_06 as command subtype. Arguments (if required) should always go as separate bytes.
/// See #STEP_MOTOR_GENERAL for more details.
///
/// \section sect_step_motor_dev_motor_command_08 Set command
/// Set commands is used to set multiple values. It uses @ref sect_step_motor_dev_motor_command_06 as command subtype.  Arguments (if required) should always go as separate bytes.
/// See #STEP_MOTOR_SET for more details.
///
/// \section sect_step_motor_dev_motor_command_09 Move command
/// Move command is used to cause actual stepper motor rotation. The argument is 64 bit value that specify number of pulses to send.
/// It uses @ref sect_step_motor_dev_motor_command_06 as argument, so small rotation may use this motor command argument bits to pass small values.
/// See #STEP_MOTOR_MOVE for more details.
///
/// \section sect_step_motor_dev_motor_command_10 Move non stop command
/// Move non stop command is used to cause actual stepper motor rotation. It doesn't use any argument, so @ref sect_step_motor_dev_motor_command_06 is ignored.
/// See #STEP_MOTOR_MOVE_NON_STOP for more details.
///


/// \def  STEP_MOTOR_NONE
/// \brief Indicates no action, just append motor commands to the motor's buffers
#define STEP_MOTOR_NONE              ((uint8_t)(0))

/// \def  STEP_MOTOR_START
/// \brief Indicates start action. This action will start execution of the commands available in buffers.
/// \brief If device is in #STEP_MOTOR_DEV_STATUS_RUN state all ongoing commands are stopped and execution begins from
/// \brief next command in buffer.
#define STEP_MOTOR_START             COMM_CMDBYTE_DEV_SPECIFIC_4

/// \def  STEP_MOTOR_STOP
/// \brief Indicates stop action. This action will stop and reset all motors. Motor buffers will be cleared.
/// \brief All stepper motor driver lines will set to default values.
#define STEP_MOTOR_STOP              COMM_CMDBYTE_DEV_SPECIFIC_5



//---------------------------- Motor command structure ----------------------------
//  Command structure for the motor
// bit number:  7  6  5  4  3  2  1  0
//             MS P1 P0 C1 C0 A2 A1 A0
//              |  |  |  |  |  |  |  +-----> A0, A1 and A2 are place for arguments for
//              |  |  |  |  |  |  +--------> those commands that doesn't require additional data.
//              |  |  |  |  |  +-----------> For 'set' and 'general' commands it represent subcommand
//              |  |  |  |  +--------------> C0 and C1 used to describe commands
//              |  |  |  +----------------->
//              |  |  +--------------------> P0 and P1 : specify type of the payload
//              |  +-----------------------> (one of StepMotorCmdParam_XX structures)
//              +--------------------------> Motor select: bits 6..0 represent index
//                                           of the motor to be selected

/// \def STEP_MOTOR_SELECT
/// \brief This motor command flag is described in @ref sect_step_motor_dev_motor_command_03 . Used to select motor for subsequent commands.
#define STEP_MOTOR_SELECT            (uint8_t)(0b10000000)

/// \def STEP_MOTOR_PARAM_MASK
/// \brief Defines location of @ref sect_step_motor_dev_motor_command_04 in motor command byte
#define STEP_MOTOR_PARAM_MASK        (uint8_t)(0b01100000)

/// \def STEP_MOTOR_PARAM_NONE
/// \brief Defines location of @ref sect_step_motor_dev_motor_command_04 in motor command byte
#define STEP_MOTOR_PARAM_NONE        (uint8_t)(0b00000000)

/// \def STEP_MOTOR_PARAM_8
/// \brief Specifies that motor command is followed by argument represented by 1 byte.
#define STEP_MOTOR_PARAM_8           (uint8_t)(0b00100000)

/// \def STEP_MOTOR_PARAM_16
/// \brief Specifies that motor command is followed by argument represented by 2 bytes.
#define STEP_MOTOR_PARAM_16          (uint8_t)(0b01000000)

/// \def STEP_MOTOR_PARAM_64
/// \brief Specifies that motor command is followed by argument represented by 8 bytes.
#define STEP_MOTOR_PARAM_64          (uint8_t)(0b01100000)

/// \def STEP_MOTOR_ARG_MASK
/// \brief Defines location of @ref sect_step_motor_dev_motor_command_06 in motor command byte
#define STEP_MOTOR_ARG_MASK          (uint8_t)(0b00000111)

//---------------------------- Motor commands ----------------------------
/// \def STEP_MOTOR_CMD_MASK
/// \brief This mask defines location of the @ref sect_step_motor_dev_motor_command_05 . Use it to get actual motor command type.
#define STEP_MOTOR_CMD_MASK          (uint8_t)(0b00011000)

/// \def STEP_MOTOR_GENERAL
/// \brief This macro defines general motor command. The following general motor command sub-types are available:
/// #STEP_MOTOR_GENERAL_ENABLE, #STEP_MOTOR_GENERAL_SLEEP, #STEP_MOTOR_GENERAL_DISABLE, #STEP_MOTOR_GENERAL_WAKEUP,
/// #STEP_MOTOR_GENERAL_RESET, #STEP_MOTOR_GENERAL_WAIT, #STEP_MOTOR_GENERAL_CONFIG
#define STEP_MOTOR_GENERAL           (uint8_t)(0b00000000)

/// \def STEP_MOTOR_SET
/// \brief This macro defines set motor command. The following values may be set by this command:
/// #STEP_MOTOR_SET_MICROSTEP, #STEP_MOTOR_SET_STEP_WAIT, #STEP_MOTOR_SET_CW_SFT_LIMIT, #STEP_MOTOR_SET_CCW_SFT_LIMIT.
/// Setting #STEP_MOTOR_SET_DIR_CW or #STEP_MOTOR_SET_DIR_CCW values changes stepper motor rotation in corresponding direction.
#define STEP_MOTOR_SET               (uint8_t)(0b00001000)

/// \def STEP_MOTOR_MOVE
/// \brief This command requires a parameter that specify number of step pulses to be issued. Also it uses value passed with #STEP_MOTOR_SET_STEP_WAIT
/// in order to form required waits to achieve specified speed. This command may be interrupted if hardware endstop or software limit
/// is reached and it is configured to not be ignored by #STEP_MOTOR_GENERAL_CONFIG.
#define STEP_MOTOR_MOVE              (uint8_t)(0b00010000)

/// \def STEP_MOTOR_MOVE_NON_STOP
/// \brief This command doesn't require parameter and cause step pulses to be issued without numbered limit. Also it uses value passed with #STEP_MOTOR_SET_STEP_WAIT
/// in order to form required waits to achieve specified speed. This command may be interrupted if hardware endstop or software limit
/// is reached and it is configured to not be ignored by #STEP_MOTOR_GENERAL_CONFIG.
#define STEP_MOTOR_MOVE_NON_STOP     (uint8_t)(0b00011000)

/// \def STEP_MOTOR_LIMITED_MOVE
/// \brief This macro takes motor command byte and returns zero if #STEP_MOTOR_MOVE_NON_STOP is used, and nonzero if #STEP_MOTOR_MOVE is used.
/// \warning Make sure motor command is either #STEP_MOTOR_MOVE or #STEP_MOTOR_MOVE_NON_STOP. Otherwise this macro behaviour is undefined.
#define STEP_MOTOR_LIMITED_MOVE(cmd) (((cmd) & STEP_MOTOR_CMD_MASK) ^ (STEP_MOTOR_MOVE_NON_STOP))

/// \def STEP_MOTOR_GENERAL_ENABLE
/// \brief Defines #STEP_MOTOR_GENERAL command that enables stepper motor driver. If physical line is not used this motor command is ignored.
#define STEP_MOTOR_GENERAL_ENABLE    (uint8_t)(0b00000000)

/// \def STEP_MOTOR_GENERAL_SLEEP
/// \brief Defines #STEP_MOTOR_GENERAL command that put stepper motor driver into sleep mode. If physical line is not used this motor command is ignored.
#define STEP_MOTOR_GENERAL_SLEEP     (uint8_t)(0b00000001)

/// \def STEP_MOTOR_GENERAL_DISABLE
/// \brief Defines #STEP_MOTOR_GENERAL command that disables stepper motor driver. If physical line is not used this motor command is ignored.
#define STEP_MOTOR_GENERAL_DISABLE   (uint8_t)(0b00000010)

/// \def STEP_MOTOR_GENERAL_WAKEUP
/// \brief Defines #STEP_MOTOR_GENERAL command that returns stepper motor drive from sleep mode. If physical line is not used this motor command is ignored.
#define STEP_MOTOR_GENERAL_WAKEUP    (uint8_t)(0b00000011)

/// \def STEP_MOTOR_GENERAL_RESET
/// \brief Defines #STEP_MOTOR_GENERAL command that causes reset pulse on RESET line of the stepper motor driver. If physical line is not used this motor command is ignored.
#define STEP_MOTOR_GENERAL_RESET     (uint8_t)(0b00000100)

/// \def STEP_MOTOR_GENERAL_WAIT
/// \brief Defines #STEP_MOTOR_GENERAL command that causes simple delay. Requires 64 bit argument that represents amount of microseconds the wait should last.
#define STEP_MOTOR_GENERAL_WAIT      (uint8_t)(0b00000101)

/// \def STEP_MOTOR_GENERAL_CONFIG
/// \brief Defines #STEP_MOTOR_GENERAL command that set behaviour configuration of the stepper motor drive. Requires 8 bit argument that represent a bit mask of the following flags:
/// #STEP_MOTOR_CONFIG_FAILURE_IGNORE, #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_FAILURE_ALL, #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_ERROR_ALL converted to a single byte with #STEP_MOTOR_CONFIG_TO_BYTE macro.
/// Use #STEP_MOTOR_CONFIG_BYTE_TO_FLAGS macro to convert this byte back to actual configuration/status flag values.
/// See @ref group_step_motor_dev_configuration for more details.
#define STEP_MOTOR_GENERAL_CONFIG    (uint8_t)(0b00000110)

/// \def STEP_MOTOR_SET_DIR_CCW
/// \brief Defines #STEP_MOTOR_SET command that set stepper motor rotation to counter-clock-wise (CCW).
/// This command affects physical DIRECTION line (if used) of the stepper motor driver, tag_StepMotorStatus#pos calculation, hardware endstops or software limits processing during stepper motor rotation.
/// This command doesn't require arguments.
#define STEP_MOTOR_SET_DIR_CCW       (uint8_t)(0b00000000)

/// \def STEP_MOTOR_SET_DIR_CW
/// \brief Defines #STEP_MOTOR_SET command that set stepper motor rotation to clock-wise (CW).
/// This command affects physical DIRECTION line (if used) of the stepper motor driver, tag_StepMotorStatus#pos calculation, hardware endstops or software limits processing during stepper motor rotation.
/// This command doesn't require arguments.
#define STEP_MOTOR_SET_DIR_CW        (uint8_t)(0b00000001)

/// \def STEP_MOTOR_SET_MICROSTEP
/// \brief Defines #STEP_MOTOR_SET command that set micro stepping options for the stepper motor drive. Requires 8 bit argument that may be a bitwise combination of the following flags:
/// #STEP_MOTOR_SET_MICROSTEP_M1, #STEP_MOTOR_SET_MICROSTEP_M2, #STEP_MOTOR_SET_MICROSTEP_M3.
/// This command affects physical physical micro stepping lines (M1, M2 and M3) of the stepper motor driver (if used), tag_StepMotorStatus#pos calculation, hardware endstops or software limits processing during stepper motor rotation.
#define STEP_MOTOR_SET_MICROSTEP     (uint8_t)(0b00000010)

/// \def STEP_MOTOR_SET_STEP_WAIT
/// \brief Defines #STEP_MOTOR_SET command that set wait between step pulses that cause stepper motor rotations. Requires 64-bit argument, which represents a number of microseconds that should separate subsequent step pulses.
/// This value may not be less than #STEP_MOTOR_MIN_STEP_WAIT.
#define STEP_MOTOR_SET_STEP_WAIT     (uint8_t)(0b00000011)

/// \def STEP_MOTOR_MIN_STEP_WAIT
/// \brief Defines a limit for the #STEP_MOTOR_SET_STEP_WAIT argument. Step pulses may not follow each other with lesser wait (in microseconds).
#define STEP_MOTOR_MIN_STEP_WAIT    100

/// \def STEP_MOTOR_SET_CW_SFT_LIMIT
/// \brief Defines #STEP_MOTOR_SET command that set clock-wise (CW) software limit of the tag_StepMotorStatus#pos.
#define STEP_MOTOR_SET_CW_SFT_LIMIT  (uint8_t)(0b00000100)

/// \def STEP_MOTOR_SET_CCW_SFT_LIMIT
/// \brief Defines #STEP_MOTOR_SET command that set counter-clock-wise (CCW) software limit of the tag_StepMotorStatus#pos.
#define STEP_MOTOR_SET_CCW_SFT_LIMIT (uint8_t)(0b00000101)

/// \def STEP_MOTOR_SET_MICROSTEP_M1
/// \brief Defines bit that corresponds M1 line for the #STEP_MOTOR_SET_MICROSTEP argument
#define STEP_MOTOR_SET_MICROSTEP_M1  (uint8_t)(0b00000001)

/// \def STEP_MOTOR_SET_MICROSTEP_M2
/// \brief Defines bit that corresponds M2 line for the #STEP_MOTOR_SET_MICROSTEP argument
#define STEP_MOTOR_SET_MICROSTEP_M2  (uint8_t)(0b00000010)

/// \def STEP_MOTOR_SET_MICROSTEP_M3
/// \brief Defines bit that corresponds M3 line for the #STEP_MOTOR_SET_MICROSTEP argument
#define STEP_MOTOR_SET_MICROSTEP_M3  (uint8_t)(0b00000100)

/// @}


/// \defgroup group_step_motor_dev_statuses Statuses
/// \brief Stepper motor device (and motor) status structures
/// @{
/// \page page_step_motor_dev_statuses
/// \tableofcontents
///
/// \section sect_step_motor_dev_statuses_01 Status structures
/// Stepper motor status information is separated on two parts:
/// 1. StepMotorDev virtual device status. #tag_StepMotorDevStatus structure is used to describe virtual device status.
/// 2. Stepper motors statuses. Stepper motor status is described by #tag_StepMotorStatus
///    structure. Note, that #tag_StepMotorDevStatus contains an array of #tag_StepMotorStatus, which is obvious, stepper
///    motor status is a part of whole StepMotorDev device status.
///

/// \def STEP_MOTOR_DEV_STATUS_STATE_MASK
/// \brief Defines location of device status value in tag_StepMotorDevStatus#status
#define STEP_MOTOR_DEV_STATUS_STATE_MASK    (uint8_t)(0b00000001)

/// \def STEP_MOTOR_DEV_STATUS_IDLE
/// \brief Defines idle stepper motor device state. Motor(s) ENABLE and SLEEP lines are in their default state.
#define STEP_MOTOR_DEV_STATUS_IDLE          (uint8_t)(0b00000000)

/// \def STEP_MOTOR_DEV_STATUS_RUN
/// \brief Defines running stepper motor device state. This state indicates device is controlling motor(s) and all line(s) are set according to executed commands.
#define STEP_MOTOR_DEV_STATUS_RUN           (uint8_t)(0b00000001)

/// \def STEP_MOTOR_DEV_STATUS_ERROR
/// \brief Defines error stepper motor device state. This state indicates state is equivalent to #STEP_MOTOR_DEV_STATUS_IDLE, but one or more motors had errors. Motor(s) ENABLE and SLEEP lines are in their default state.
#define STEP_MOTOR_DEV_STATUS_ERROR         (uint8_t)(0b00000010)

/// \struct tag_StepMotorStatus
/// \brief Describes current motor status
/// \warning Firmware code should make changes to this structure with interrupts disabled.
typedef struct __attribute__ ((aligned)) tag_StepMotorStatus {
    int64_t  pos;               ///< Stepper motor position. CW moves increase (and CCW moves decrease) position (by 32 / microstep divider) value. See @ref group_step_motor_dev_microstep_tables
    int64_t  cw_sft_limit;      ///< Current software limit for stepper motor position during CW moves. Ignored if hardware end-stop is used
    int64_t  ccw_sft_limit;     ///< Current software limit for stepper motor position during CCW moves. Ignored if hardware end-stop is used
    uint32_t motor_state;       ///< flags used to describe current motor status. Corresponds to tag_StepMotorDescriptor#config_flags. See @ref group_step_motor_dev_configuration
    uint16_t bytes_remain;      ///< Number of unread bytes in motor command buffer.
} StepMotorStatus, *PStepMotorStatus;

/// \struct tag_StepMotorDevStatus
/// \brief This structure describes current stepper motor device state. It also includes statuses of all stepper motors belonging to the device.
/// \warning Firmware code should make changes to this structure with interrupts disabled.
typedef struct __attribute__ ((aligned)) tag_StepMotorDevStatus {
    uint8_t status; ///< Stepper motor device status. One of the following values: #STEP_MOTOR_DEV_STATUS_IDLE, #STEP_MOTOR_DEV_STATUS_RUN, #STEP_MOTOR_DEV_STATUS_ERROR
    StepMotorStatus mstatus[] __attribute__ ((aligned)); ///< Array of #tag_StepMotorStatus structures containing each motor status. Indexed as motor index.
} StepMotorDevStatus, *PStepMotorDevStatus;

/// @}


/// @}



/// \defgroup group_step_motor_dev_description Motor descriptor
/// \brief Stepper motor description structure
/// @{
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
typedef struct __attribute__ ((aligned)) tag_StepMotorDescriptor {
    uint32_t      config_flags;                 ///< flags used to describe default step motor behaviour. Corresponds to tag_StepMotorStatus#motor_state. See @ref group_step_motor_dev_configuration
    uint16_t      buffer_size;                  ///< stepper motor command buffer size in bytes.
    uint64_t      default_speed;                ///< stepper motor default speed. tag_StepMotorDescriptor#default_speed is a number of microseconds between step pulses. It doesn't take into account microstepping.
    uint8_t       motor_driver;                 ///< stepper motor driver type. One of the STEP_MOTOR_DRIVER_XXX values. See @ref group_step_motor_dev_microstep_tables
    int64_t       cw_sft_limit;                 ///< Default software limit for stepper motor position during CW moves. Ignored if hardware end-stop is used
    int64_t       ccw_sft_limit;                ///< Default software limit for stepper motor position during CCW moves. Ignored if hardware end-stop is used
    const char*   motor_name;                       ///< Name of the motor given in JSON configuration file. (available in software part only)
    uint16_t      steps_per_revolution;             ///< Number of steps per revolution for this stepper motor. (available in software part only)
} StepMotorDescriptor;
/// @}


/// \defgroup group_step_motor_dev_device_description Device descriptor
/// \brief Stepper motor device description structure
/// @{
/// \page page_step_motor_dev_device_description
/// \tableofcontents
///
/// \section sect_step_motor_dev_device_description_01 Description
/// Customizer generates description for all stepper motor devices in #tag_StepMotorDevice structures.
/// Some fields are used in runtime, some fields should remain constant. Be very careful changing them.
///

/// \struct tag_StepMotorConfig
/// \brief This structure is being used by firmware and software as storage of all information needed.
typedef struct __attribute__ ((aligned)) tag_StepMotorConfig {
	const char*	     dev_name;      ///< Stepper motor device name. (available in firmware part only). Do not change this field.
    const StepMotorDescriptor** motor_descriptor; ///< Array of the pointers to #tag_StepMotorDescriptor for each stepper motor controlled by the device. Do not change this field.
    uint8_t          motor_count;   ///< Number of stepper motors controled by this device. Do not change this field.
    uint8_t          dev_id;        ///< Device ID for the stepper motor device. Do not change this field.
} StepMotorConfig;
/// @}

