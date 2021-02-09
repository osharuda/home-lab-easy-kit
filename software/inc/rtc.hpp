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
 *   \brief RTCDev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include "ekit_device.hpp"
#include "tools.hpp"
#include "sw.h"

#ifdef RTC_DEVICE_ENABLED

/// \defgroup group_rtc_dev RTCDev
/// \brief Real Time Clock support
/// @{
/// \page page_rtc_dev
/// \tableofcontents
///
/// \section sect_rtc_dev_01 Work with RTCDev
///
/// RTCDev provides simple access for Real Time Clock available in MCU. This simple feature may be used to get time, in
/// situations where there is no possibility to get time at all. For example, once power line is recovered (after power
/// outage), there will be some time for network equipment to get online. This time may last minutes, and Raspberry Pi
/// will have no clue what is the current time.
///
/// RTCDev may help in such situations, so time recovery will be faster. Of cause, it will be very useful for offline
/// servers.
///
/// Beware, that accuracy of RTCDev is limited by 1 second, and RTC itself may be inaccurate. It depends on many
/// things, including the way PCB with MCU was routed and which components were used. Network time will be more accurate,
/// but if no other option exist, it is better than nothing.
///
/// Usage of RTCDev is simple:
/// 1. Call RTCDev#now() to get current RTC time
/// 2. Call RTCDev#sync_rtc() in order to set RTC time the same as current system time.
/// 3. Call RTCDev#sync_host() in order to update current system time with RTC time. This may require elevated permissions.
///

/// \class RTCDev
/// \brief This class implement RTCDev virtual device functionality
class RTCDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;
	public:

    /// \brief No default constructor
    RTCDev() = delete;

    /// \brief Copy construction is forbidden
    RTCDev(const RTCDev&) = delete;

    /// \brief Assignment is forbidden
    RTCDev& operator=(const RTCDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the ACDDev device.
	RTCDev(std::shared_ptr<EKitBus>& ebus, int addr);

    /// \brief Destructor (virtual)
	~RTCDev() override;

	/// \brief Returns current RTC time
	/// \return current RTC time
	std::time_t now();

	/// \brief Synchronizes RTC with current system time
	/// \return time used to synchronize RTC
	std::time_t sync_rtc();

	/// \brief Synchronizes current system time with RTC time
	/// \return time used to synchronize system time
	std::time_t sync_host();

    /// \brief Returns device name as specified in JSON configuration file
    /// \return string with name
    std::string get_dev_name() const override;

	private:

    /// \brief Returns RTC time in seconds
    /// \return RTC time in seconds
	uint32_t now_priv();
};

/// @}

#endif