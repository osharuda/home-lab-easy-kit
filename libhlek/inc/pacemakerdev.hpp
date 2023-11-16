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
 *   \brief PaceMakerDev device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "pacemakerdev_common.hpp"
#include <iostream> /* TEST CODE */
#include <iomanip> /* TEST CODE */

/// \defgroup group_pacemakerdev PaceMakerDevDev
/// \brief PaceMakerDev support
/// @{
/// \page page_pacemakerdev
/// \tableofcontents
///
/// \section sect_pacemakerdev_01 Work with PaceMakerDevDev
///
/// PaceMakerDevDev functionality provides the following features:
/// - features list ...
///
/// Basic logic of PaceMakerDevDev functionality work is shown on the following schema:
/// \image html PaceMakerDevDev_schema.png
/// \image latex PaceMakerDevDev_schema.eps
///

/// \struct PaceMakerSignalTransit
/// \brief Describes signal transit
struct PaceMakerSignalTransit{
    uint32_t signal;    // Signals to set
    double next_delay;  // Delay before the next signal transit
};
typedef PaceMakerSignalTransit* PPaceMakerSignalTransit;
typedef std::list<PaceMakerSignalTransit> PaceMakerSignals;

/// \class PaceMakerDev
/// \brief PaceMakerDev implementation. Use this class in order to control PaceMakerDev virtual devices.
class PaceMakerDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

    static constexpr double max_main_freq = 800000.0L;
    static constexpr double max_internal_delay = 60.0L;
    static constexpr double min_internal_delay = 1 / 800000.0L;

    uint32_t all_signals;
    uint32_t current_signal;
    PaceMakerSignals signals;


	public:

	/// \brief Pointer to the #tag_PaceMakerDevConfig structure that describes PaceMakerDev virtual device represented by this class.
	const PaceMakerDevConfig* config;

    /// \brief No default constructor
    PaceMakerDev() = delete;

    /// \brief Copy construction is forbidden
    PaceMakerDev(const PaceMakerDev&) = delete;

    /// \brief Assignment is forbidden
    PaceMakerDev& operator=(const PaceMakerDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	PaceMakerDev(std::shared_ptr<EKitBus>& ebus, const PaceMakerDevConfig* config);

    /// \brief Destructor (virtual)
	~PaceMakerDev() override;

    /// \brief Starts signal generation
    /// \param frequency - frequency of main cycle repetition
    /// \param repeat_count - Number of times to repeat main cycle
    void start(double frequency, size_t repeat_count);

    /// \brief Stops signals generation
    void stop();

    /// \brief Resets device to it's default state.
    void reset();

    /// \brief Sends data to the device buffer.
    void set_data();

    /// \brief Reset signal composer.
    void reset_signals();

    /// \brief Returns status of the device
    void status(PaceMakerStatus& s);

    /// \brief Append event which set's all signals.
    /// \param offset - offset, in seconds, from the last signal change.
    /// \param signal_value - signals values: bits with 1 set outputs to high, bits with 0 set outputs to low.
    void add_set(double offset, uint32_t signal_value);

    /// \brief Append event which flips specified lines.
    /// \param offset - offset, in seconds, from the last signal change.
    /// \param affected_signals - signals to flip: bits with 1 are flipped, bits with 0 are not changed.
    void add_flip(double offset, uint32_t affected_signals);

    /// \brief Append pulse to the specified lines (using flipping).
    /// \param offset - offset, in seconds, from the last signal change.
    /// \param period - pulse period, in seconds
    /// \param affected_signals - signals to flip: bits with 1 are flipped, bits with 0 are not changed.
    void add_pulse(double offset, double period, uint32_t affected_signals);


    /// \brief Append pulse to the specified lines (using flipping).
    /// \param offset - offset, in seconds, from the last signal change.
    /// \param period - period, in seconds
    /// \param pwm_value - PWM value, from 0 to 1. 0 means signal lines will not change.
    /// \param count - number of pulses.
    /// \param affected_signals - signals to flip: bits with 1 are affected, bits with 0 are not affected.
    void add_pwm(double offset, double period, double pwm_value, size_t count, uint32_t affected_signals);

    /// \brief Append clock to the specified lines (using flipping).
    /// \param offset - offset, in seconds, from the last signal change.
    /// \param period - period, in seconds
    /// \param count - number of pulses.
    /// \param affected_signals - signals to flip: bits with 1 are affected, bits with 0 are not affected.
    void add_clock(double offset, double period, size_t count, uint32_t affected_signals);

    /// \brief Append return signals to default.
    /// \param offset - offset, in seconds, from the last signal change.
    void add_default(double offset);

    uint32_t all_signals_mask() const;
};

/// @}
