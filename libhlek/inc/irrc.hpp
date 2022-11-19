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
 *   \brief IRRCDev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include "ekit_device.hpp"
#include "tools.hpp"
#include "irrc_common.hpp"

/// \defgroup group_irrc_dev IRRCDev
/// \brief NEC standard IR remote control support
/// @{
/// \page page_irrc_dev
/// \tableofcontents
///
/// \section sect_irrc_dev_01  Work with IRRCDev
/// IRRCDev provides access to Infra Red Remote Controls. Currently NEC standard is implemented. This functionality
/// allows to use TV set remote control (if capable with NEC standard) in order to control microcomputer logic, to trigger
/// some measurements remotely or anything else.
///
/// All commands sent by remote control is stored in MCU circular buffer. Everything you should do is too read data from
/// the buffer periodically. There is just one way to be used: call IRRCDev#get in order to get all commands received
/// so far. Note, all commands are deleted from circular buffer during this call.
///

/// \struct IR_NEC_Command
/// \brief Describes NEC standard command for IRRCDev device
struct IR_NEC_Command {
	uint8_t address;    ///< address
	uint8_t command;    ///< command

	/// \brief Compare operator for equality
	/// \param r - right hand value
	/// \param l - left hand value
	/// \return true if equal, otherwise false.
	friend bool operator==(const IR_NEC_Command& r, const IR_NEC_Command& l) {
		return r.address==l.address && r.command==l.command;
	}

    /// \brief Compare operator for inequality
    /// \param r - right hand value
    /// \param l - left hand value
    /// \return true if not equal, otherwise false.
	friend bool operator!=(const IR_NEC_Command& r, const IR_NEC_Command&l) {
		return !(r==l);
	}
};

/// class IRRCDev
/// \brief IRRCDev implementation. Use this class in order to control IRRCDev virtual device.
class IRRCDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

public:

    /// \brief No default constructor
    IRRCDev() = delete;

    /// \brief Copy construction is forbidden
    IRRCDev(const IRRCDev&) = delete;

    /// \brief Assignment is forbidden
    IRRCDev& operator=(const IRRCDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	IRRCDev(std::shared_ptr<EKitBus>& ebus, const IRRCConfig* config);

	/// \brief Destructor (virtual)
	~IRRCDev() override;

	/// \brief Returns commands accumulated in circular buffer so far
	/// \param commands - reference to the output vector with commands. All pre-existing commands are cleared.
	/// \param ovf - true if overflow has occurred in circular buffer.
	/// \details Returned data is removed from circular buffer.
	void get(std::vector<IR_NEC_Command>& commands, bool& ovf);
};

/// @}
