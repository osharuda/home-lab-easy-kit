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
 *   \brief Desk device implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef DESKDEV_DEVICE_ENABLED

#include "ekit_device.hpp"
#include "tools.hpp"
#include "sw.h"

/// \defgroup group_desk_dev DESKDev
/// \brief Simple set of four buttons and one encoder
/// @{
/// \page page_desk_dev
/// \tableofcontents
///
/// \section sect_desk_dev_01 Work with ADCDev
///
/// DESKDev virtual device provides possibility to control an encoder and four buttons. It gives possibility to control
/// simple menus on #LCD1602ADev.
///
/// Usage of DESKDev is easy: DESKDev#get() should be called periodically in order to get events from controls. Encoder is
/// represented by signed 8-bit integer value that represent amount of steps made clock or counter clock wise. Each button
/// is represented by unsigned 8-bit value that represent amount of timers this button was pushed. Note, that DESKDev contains
/// some kind of protection against switching noise jitter, but it may fail sometimes.
///

/// class DESKDev
/// \brief DESKDev implementation. Use this class in order to control DESKDev virtual device.
class DESKDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

public:

    /// \brief No default constructor
    DESKDev() = delete;

    /// \brief Copy construction is forbidden
    DESKDev(const DESKDev&) = delete;

    /// \brief Assignment is forbidden
    DESKDev& operator=(const DESKDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the IRRCDev device.
	DESKDev(std::shared_ptr<EKitBus>& ebus, int addr);

	/// \brief Destructor (virtual)
	~DESKDev() override;

    /// \brief Returns events accumulated by DESKDev.
    /// \param up - number of times UP button was pressed.
    /// \param down - number of times DOWN button was pressed.
    /// \param left - number of times LEFT button was pressed.
    /// \param right - number of times RIGHT button was pressed.
    /// \param encoder - signed number of steps encoder was rotated.
    /// \details Ones read, internal counters are zeroed.
	void get(bool& up, bool& down, bool& left, bool& right, int8_t& encoder);

    /// \brief Returns device name as specified in JSON configuration file
    /// \return string with name
    std::string get_dev_name() const override;
};

/// @}

#endif