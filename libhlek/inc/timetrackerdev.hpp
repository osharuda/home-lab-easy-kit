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
 *   \brief TimeTrackerDev device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "timetrackerdev_common.hpp"

/// \defgroup group_timetrackerdev TimeTrackerDevDev
/// \brief TimeTrackerDev support
/// @{
/// \page page_timetrackerdev
/// \tableofcontents
///
/// \section sect_timetrackerdev_01 Work with TimeTrackerDevDev
///
/// TimeTrackerDevDev functionality provides the following features:
/// - features list ...
///
/// Basic logic of TimeTrackerDevDev functionality work is shown on the following schema:
/// \image html TimeTrackerDevDev_schema.png
/// \image latex TimeTrackerDevDev_schema.eps
///

/// \class TimeTrackerDev
/// \brief TimeTrackerDev implementation. Use this class in order to control TimeTrackerDev virtual devices.
class TimeTrackerDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	public:

	/// \brief Pointer to the #tag_TimeTrackerDevConfig structure that describes TimeTrackerDev virtual device represented by this class.
	const TimeTrackerDevConfig* config;

    /// \brief No default constructor
    TimeTrackerDev() = delete;

    /// \brief Copy construction is forbidden
    TimeTrackerDev(const TimeTrackerDev&) = delete;

    /// \brief Assignment is forbidden
    TimeTrackerDev& operator=(const TimeTrackerDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	TimeTrackerDev(std::shared_ptr<EKitBus>& ebus, const TimeTrackerDevConfig* config);

    /// \brief Destructor (virtual)
	~TimeTrackerDev() override;

    /// \brief Start device.
    /// \param reset - reset device before starting.
    void start(bool reset);

    /// \brief Stops device.
    void stop();

    /// \brief Obtain status of the device
    /// \param running - output parameter, it is set to true if device is running, otherwise it is set to false.
    /// \param first_ts - first timestamp since the last reset. If no events happened, UINT64_MAX.
    /// \return number of events accumulated
    size_t get_status(bool& running, uint64_t& first_ts);

    /// \brief Reads all data from the device
    /// \param data - std::vector to be used as storage. Timestamps are raw number of ticks. To get value in seconds,
    ///               division by \ref TimeTrackerDevConfig::tick_freq is required.
    ///               Data is appended to the end of the vector.
    /// \param relative - if true, all events are returned relative to first event since reset.
    void read_all(std::vector<uint64_t>& data, bool relative);

    /// \brief Reads all data from the device
    /// \param data - std::vector of double to be used as storage. Data is appended to the end of the vector.
    ///               Values are seconds (internally divided by \ref TimeTrackerDevConfig::tick_freq).
    /// \param relative - if true, all events are returned relative to first event since reset.
    void read_all(std::vector<double>& data, bool relative);
private:
    PTimeTrackerStatus dev_status;
    uint64_t* data_buffer;
    std::vector<uint8_t> raw_buffer;
    static constexpr size_t max_timestamps_per_i2c_transaction = 512;

    /// \brief Sends a command to the devive
    /// \param flags - command byte flags to be used.
    void send_command(int flags);

    /// \brief Read status and data from device into internal buffer.
    /// \param status - pointer to the device status.
    /// \param data - pointer to the array with data.
    /// \param count - number of timestamp elements in data array to be read.
    /// \param to - timeout counting object
    /// \param ovf - output parameter. If true, overflow has occurred, otherwise false.
    void get_priv(size_t count, EKitTimeout& to, bool& ovf);
};

/// @}
