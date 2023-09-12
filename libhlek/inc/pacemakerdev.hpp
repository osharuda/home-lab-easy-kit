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

/// \class PaceMakerDev
/// \brief PaceMakerDev implementation. Use this class in order to control PaceMakerDev virtual devices.
class PaceMakerDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

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

    void set_main_signal(double period, size_t repeat_count);
    void set_devault_signals();
    void add_signal(uint8_t signal_id, double offset);
};

/// @}
