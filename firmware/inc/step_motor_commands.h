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
 *   \brief Stepper Motor commands handlers header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef STEP_MOTOR_DEVICE_ENABLED


/// \addtogroup group_step_motor_dev_impl
/// @{

/// \def STEP_MOTOR_CMD_COUNT
/// \brief Defines length of the #g_step_motor_cmd_map array
#define STEP_MOTOR_CMD_COUNT    (32)

/// \def STEP_MOTOR_8BIT_COMMAND_LEN
/// \brief Defines length of the 8-bit command (without payload)
#define STEP_MOTOR_8BIT_COMMAND_LEN 1

/// \def STEP_MOTOR_16BIT_COMMAND_LEN
/// \brief Defines length of the 16-bit command (1 byte payload)
#define STEP_MOTOR_16BIT_COMMAND_LEN 2

/// \def STEP_MOTOR_24BIT_COMMAND_LEN
/// \brief Defines length of the 24-bit command (2 bytes payload)
#define STEP_MOTOR_24BIT_COMMAND_LEN 3

/// \def STEP_MOTOR_72BIT_COMMAND_LEN
/// \brief Defines length of the 72-bit command (8 bytes payload)
#define STEP_MOTOR_72BIT_COMMAND_LEN 9
extern const uint16_t g_step_motor_cmd_length_map[];

/// \def STEP_MOTOR_COMMAND_LENGTH
/// \brief Returns length of the stepper motor command
/// \param cmd - first command byte
#define STEP_MOTOR_COMMAND_LENGTH(cmd) g_step_motor_cmd_length_map[((cmd) & STEP_MOTOR_PARAM_MASK) >> 5]

/// \brief This function handles errors that occur during command execution
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command is being executed
void step_motor_handle_error(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \def STE_MOTOR_CMD_RESULT_OK
/// \brief Defines successful error code to be returned from #PFN_STEP_MOTOR_CMD_FUNC
#define STE_MOTOR_CMD_RESULT_OK               0

/// \def STE_MOTOR_CMD_RESULT_FAIL
/// \brief Defines failure error code to be returned from #PFN_STEP_MOTOR_CMD_FUNC
#define STE_MOTOR_CMD_RESULT_FAIL             1

/// \typedef PFN_STEP_MOTOR_CMD_FUNC
/// \brief Defines stepper motor handler to be called for each command passed to stepper motor buffer
/// \param dev - pointer to #tag_StepMotorDevice structure corresponding to selected stepper motor device
/// \param mindex - motor index
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
typedef uint8_t (*PFN_STEP_MOTOR_CMD_FUNC)(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Initializes g_step_motor_cmd_map command map. This map is used to access commands by index.
void step_motor_init_cmd_map(void);

/// \brief Corrects wait, taking into account the fact timer may cause errors.
/// \param wait - wait requested by command
/// \param corr_factor - correction factor. Limits correction maximum by (wait >> corr_factor) value
/// \param mcontext - pointer to the corresponding stepper motor #tag_StepMotorContext structure
/// \return corrected wait value
/// \details correction calculation is based on StepMotorContext::late_us value
uint64_t step_motor_correct_timing(uint64_t wait, uint8_t corr_factor, struct StepMotorContext* mcontext);

/// \brief Invalid command handler. It is invoked in the case invalid command is sent by a software
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
uint8_t step_motor_invalid_cmd(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Enable command handler. This command is sent by a software in order to enable step motor driver
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
uint8_t step_motor_general_enable(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Sleep command handler. This command is sent by a software in order to put step motor driver into sleep state
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
uint8_t step_motor_general_sleep(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Disable command handler. This command is sent by a software in order to disable motor driver
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
uint8_t step_motor_general_disable(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Wakeup command handler. This command is sent by a software in order to wake up step motor driver from sleep state
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
uint8_t step_motor_general_wakeup(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Reset command handler. This command is sent by a software in order to reset step motor driver
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
uint8_t step_motor_general_reset(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Wait command handler. This command is sent by a software in order to issue a wait. No actions are made to
///        step motor driver
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details Wait command requires a parameter that spcifies number of microseconds to wait
uint8_t step_motor_general_wait(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Set software limit for motor position during CW movement.
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command may fail if inappropriate software limit is passed
/// \details Software limits are ignored if corresponding direction equipped with hardware endstop line is used
uint8_t step_motor_set_cw_sft_limit(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Set software limit for motor position during CCW movement.
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command may fail if inappropriate software limit is passed
/// \details Software limits are ignored if corresponding direction equipped with hardware endstop line is used
uint8_t step_motor_set_ccw_sft_limit(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Configuration command handler. This command is sent by a software in order to configure step motor
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
///          Configuration parameter is required. It is described by STEP_MOTOR_CONFIG_XXX flags
uint8_t step_motor_general_config(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Clockwise direction command handler. This command is sent by a software in order to instruct step motor driver
///        to rotate in clockwise direction
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
/// \note If step motor rotates in wrong direction, check if coils are connected properly
uint8_t step_motor_set_dir_cw(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Counter clockwise direction command handler. This command is sent by a software in order to instruct step motor driver
///        to rotate in counter clockwise direction
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
/// \note If step motor rotates in wrong direction, check if coils are connected properly
uint8_t step_motor_set_dir_ccw(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Set microstep command handler. This command is sent by a software in order to instruct step motor driver
///        to use specific micro step options.
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details Microstep parameter is required. It is described by STEP_MOTOR_SET_MICROSTEP_MXXX flags
///          This command is executed immediately, timings and command state are not required
/// \note Read step motor driver documentation for more details regarding micro steps being supported
uint8_t step_motor_set_microstep(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Set step wait being command handler. This command is sent by a software in order to set duration that
///        separates subsequent step pulses. This parameter directly controls rotation speed of the motor.
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
/// \details This command is executed immediately, timings and command state are not required
/// \details This command requires parameter that specifies wait duration in microseconds
/// \note Read step motor and step motor driver documentation for supported values. Value is specified in micro seconds.
uint8_t step_motor_set_step_wait(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// \brief Move and Move non-stop commands handler. This commands is sent by a software in order to instruct step motor
///        to move by infinite or a fixed numbed of steps.
/// \param dev - device this command was sent to
/// \param mindex - target motor index this command was sent to
/// \param cmd - command to be executed
/// \return 0 - Success, nonzero indicates error
uint8_t step_motor_move(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd);

/// @}

#endif
