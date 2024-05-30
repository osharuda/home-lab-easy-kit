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
};

/// @}
