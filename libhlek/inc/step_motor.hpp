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
 *   \brief StepMotorDev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include "ekit_device.hpp"
#include "step_motor_common.hpp"

/// \defgroup group_step_motor_dev StepMotorDev
/// @{
/// \page page_step_motor_dev
/// \tableofcontents
///
/// \section sect_step_motor_dev_01 Work with StepMotorDev.
///
/// StepMotorDev implements the following features:
/// - Enable, Sleep, Step and other lines control over A4998 and DRV8825 drivers.
/// - Providing information about number and properties of configured stepper motors.
/// - Reading status of StepMotorDev virtual device, stepper motors, end-stops.
/// - Estimation of time required to execute a set of commands.
/// - Programming stepper motors movements by feeding virtual device with command that control the motor.
/// - Estimation of internal virtual device circular buffer level.
///
/// Use StepMotorDev with care - this functionality controls stepper motors that may consume significant current and produce
/// a lot of heat. Read stepper motor documentation carefully to understand what you are doing. Also, it is wise to not leave
/// devices with stepper motors unattended.
///
/// Here is a small instructions how to use StepMotorDev from the software:
/// 1. Create #StepMotorDev object
/// 2. If required you may get stepper motor information with StepMotorDev#get_motor_count() and StepMotorDev#get_motor_info() calls.
/// 3. Use these methods in order to program stepper motors behaviour:
///    - StepMotorDev#enable()
///    - StepMotorDev#sleep()
///    - StepMotorDev#configure()
///    - StepMotorDev#set_software_endstop()
///    - StepMotorDev#reset()
///    - StepMotorDev#wait()
///    - StepMotorDev#dir()
///    - StepMotorDev#speed()
///    - StepMotorDev#microstep()
///    - StepMotorDev#move()
///
///   All these calls accumulate commands inside internal data structures (StepMotorDev#motors_data) until call to the
///   StepMotorDev#feed() is made.
///
///   Note, all these methods return 64-bit value which represent amount of microseconds required to execute this command.
///   Some returned values may be equal zero (StepMotorDev#enable()) some not. Using this value will allow to estimate
///   when motors will finish executing their commands. Do not rely on this value too much, because it's value may be
///   inexact for many reasons. However, this value should be reasonable enough to feed internal circular buffer with more
///   commands in time.
///
/// 4. Use StepMotorDev#start() to instruct virtual device to execute commands.
/// 5. Use StepMotorDev#stop() to terminate execution of the programed actions.
/// 6. Use StepMotorDev#feed() to send new stepper motor commands to the circular buffer.
/// 7. Periodically call StepMotorDev#status() in order to track stepper motor statuses, end-stop statuses, etc.
///

/// \struct StepMotorDevMotorData
/// \brief Structure to accumulate stepper motor commands until StepMotorDev#feed() was not called.
struct StepMotorDevMotorData{
	std::vector<uint8_t> buffer;    ///< Buffer with commands to send to the firmware
	uint64_t speed;                 ///< Stepper motor speed due to the last commands in StepMotorDevMotorData#buffer.
	uint8_t microstep;              ///< Stepper motor microstep value due to the last commands in StepMotorDevMotorData#buffer.
};

/// \class StepMotorDev
/// \brief Implementation of stepper motor virtual device support.
class StepMotorDev final : public EKitVirtualDevice{

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	/// \brief Data accumulated by the StepMotorDev class to send to the firmware with StepMotorDev#feed().
	std::vector<StepMotorDevMotorData> motors_data;

	/// \brief Clears StepMotorDev state.
    void clear();

    /// \brief Enqueue stepper motor command parameter.
    /// \param mbuffer - reference to the buffer to enqueue parameter.
    /// \param param - 64 bit parameter value.
    /// \param len - parameter length in bytes.
    void enque_param(std::vector<uint8_t>& mbuffer, uint64_t param, size_t len);

    /// \brief Enqueue stepper motor command.
    /// \param mindex - zero based motor index.
    /// \param cmd - command byte. See @ref sect_step_motor_dev_motor_command_02.
    /// \param subcmd - subcommand. See @ref sect_step_motor_dev_motor_command_02.
    /// \param param - stepper motor parameter. See @ref sect_step_motor_dev_motor_command_02.
    void enque_cmd(size_t mindex, uint8_t cmd, uint8_t subcmd, uint64_t param);

    /// \brief Returns microstep divider for the stepper motor.
    /// \param mindex - zero based motor index.
    /// \return microstep divider.
    uint8_t microstep_divider(size_t mindex);

    /// \brief Converts double value in seconds into number of microseconds.
    /// \param v - Value in seconds.
    /// \return Number of microseconds.
    /// \details Throws an exception if value can be expressed by 64-bit value.
    uint64_t double_to_us(double v) const;

	public:

    /// \brief Pointer to the #tag_StepMotorConfig structure that describes StepMotorDev virtual device represented by this class.
    const StepMotorConfig* config;

    /// \brief No default constructor
    StepMotorDev() = delete;

    /// \brief Copy construction is forbidden
    StepMotorDev(const StepMotorDev&) = delete;

    /// \brief Assignment is forbidden
    StepMotorDev& operator=(const StepMotorDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	StepMotorDev(std::shared_ptr<EKitBus>& ebus, const StepMotorConfig* config);

	/// \brief Destructor (virtual)
	~StepMotorDev() override;

	/// \brief Returns stepper motors information
	/// \return vector of const pointers to #tag_StepMotorDescriptor structures describing motors.
	std::vector<const StepMotorDescriptor*> get_motor_info() const;

	/// \brief Returns number of motors.
	/// \return Number of motors.
	size_t get_motor_count() const;

	/// \brief Enqueues enable command (#STEP_MOTOR_GENERAL_ENABLE and #STEP_MOTOR_GENERAL_DISABLE)
	/// \param mindex - zero based motor index.
	/// \param on - true to enable motor, false to disable motor.
	/// \return Number of microseconds required to execute this command.
	uint64_t enable(size_t mindex, bool on);

	/// \brief Enqueues sleep command (#STEP_MOTOR_GENERAL_SLEEP and #STEP_MOTOR_GENERAL_WAKEUP)
    /// \param mindex - zero based motor index.
	/// \param sleep - true to put in sleep mode, false to return from sleep mode.
    /// \return Number of microseconds required to execute this command.
	uint64_t sleep(size_t mindex, bool sleep);

	/// \brief Enqueues general config command (#STEP_MOTOR_GENERAL_CONFIG)
    /// \param mindex - zero based motor index.
	/// \param flags - bitmask consisting of the following flags: #STEP_MOTOR_CONFIG_FAILURE_IGNORE,
	///                #STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE,
	///                #STEP_MOTOR_CONFIG_FAILURE_ALL, #STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL, #STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL,
	///                #STEP_MOTOR_CONFIG_ERROR_ALL
    /// \return Number of microseconds required to execute this command.
	uint64_t configure(size_t mindex, uint32_t flags);

	/// \brief Enqueues set command for software limit (end-stop) value.
    /// \param mindex - zero based motor index.
	/// \param cw - true for clock-wise direction, false for counter-clock-wise direction.
	/// \param limit - new software limit value. This value is a position. This value corresponds to tag_StepMotorStatus#pos.
    /// \return Number of microseconds required to execute this command.
	uint64_t set_software_endstop(size_t mindex, bool cw, int64_t limit);

	/// \brief Enqueues a reset command for stepper motor driver (#STEP_MOTOR_GENERAL_RESET).
    /// \param mindex - zero based motor index.
    /// \return Number of microseconds required to execute this command.
    /// \warning  Do not confuse it with stop. Reset command will pulse reset line if configured. It won't stop command
    ///           execution
	uint64_t reset(size_t mindex);

	/// \brief Enqueues a wait command for stepper motor driver (#STEP_MOTOR_GENERAL_WAIT).
    /// \param mindex - zero based motor index.
	/// \param val_sec
    /// \return Number of microseconds required to execute this command.
	uint64_t wait(size_t mindex, double val_sec);

	/// \brief Enqueues stepper motor direction change command (#STEP_MOTOR_SET_DIR_CW and #STEP_MOTOR_SET_DIR_CCW).
    /// \param mindex - zero based motor index.
	/// \param cw - true for clock-wise direction, false for counter clock wise deirection.
    /// \return Number of microseconds required to execute this command.
	uint64_t dir(size_t mindex, bool cw);

	/// \brief Enqueues stepper motor rotation speed change.
    /// \param mindex - zero based motor index.
	/// \param value - value to be set
	/// \param rpm - true - value is revolutions per minute, false - value is delay (in seconds) between STEP pulses.
    /// \return Number of microseconds required to execute this command.
	uint64_t speed(size_t mindex, double value, bool rpm);

	/// \brief Enqueues microstep set command (#STEP_MOTOR_SET_MICROSTEP).
    /// \param mindex - zero based motor index.
	/// \param m1 - value for m1.
	/// \param m2 - value for m2.
	/// \param m3 - value for m3.
    /// \return Number of microseconds required to execute this command.
	uint64_t microstep(size_t mindex, bool m1, bool m2, bool m3);

	/// \brief Enqueues move command (unlimited move, #STEP_MOTOR_MOVE_NON_STOP)
    /// \param mindex - zero based motor index.
    /// \return Number of microseconds required to execute this command.
	uint64_t move(size_t mindex);

	/// \brief Enqueues move command (#STEP_MOTOR_MOVE)
    /// \param mindex - zero based motor index.
	/// \param n_steps - number of steps to move.
    /// \return Number of microseconds required to execute this command.
	uint64_t move(size_t mindex, uint64_t n_steps);

	/// \brief Returns status of the StepMotorDev virtual device and motors.
	/// \param mstatus - std::vector of the #tag_StepMotorStatus structures.
    /// \return Stepper motor device status. Corresponds to tag_StepMotorDevStatus#status.
	uint8_t status(std::vector<StepMotorStatus>& mstatus);

	/// \brief Starts execution of stepper motor commands.
	void start();

	/// \brief Stops execution of stepper motor commands.
	void stop();

	/// \brief Sends accumulated commands into virtual device for execution.
	void feed();
};

/// @}
