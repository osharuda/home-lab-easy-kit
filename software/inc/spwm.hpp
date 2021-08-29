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
 *   \brief SPWMDev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "sw.h"

#ifdef SPWM_DEVICE_ENABLED

/// \defgroup group_spwm_dev SPWMDev
/// \brief Multichannel software PWM support
/// @{
/// \page page_spwm_dev
/// \tableofcontents
///
/// \section sect_spwm_dev_01 Work with SPWMDev.
///
/// #SPWMDev is being used in the following way:
/// 1. Create instance of the #SPWMDev class.
/// 2. Allocate #SPWM_STATE map, and add key value pairs, where key is a channel index to be changed, and value - new channel
///    PWM value in range from [0 ... 65536].
/// 3. Call SPWMDev#set() to change PWM for channels mentioned in SPWM_STATE map.
/// 4. To change all channels to their default state call SPWMDev#reset().
///

/// \typedef SPWM_STATE
/// \brief Describes changes for SPWMDev#set() call.
/// \details Keys are channel indexes, values new PWM values.
typedef std::map<size_t, uint16_t> SPWM_STATE;

/// \class SPWMDev
/// \brief SPWMDev implementation
class SPWMDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	/// \brief Array of current PWM values for each channel.
	uint16_t prev_data[SPWM_CHANNEL_COUNT];

	/// \brief Array with description of all the channels.
    static constexpr SPWM_SW_DESCRIPTOR spwm_description[] = SPWM_SW_DESCRIPTION;

    /// \brief Sets stored PWM values to default state.
    void clear_prev_data();
public:

    /// \brief No default constructor
    SPWMDev() = delete;

    /// \brief Copy construction is forbidden
    SPWMDev(const SPWMDev&) = delete;

    /// \brief Assignment is forbidden
    SPWMDev& operator=(const SPWMDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the SPWMDev device.
	SPWMDev(std::shared_ptr<EKitBus>& ebus, int addr);

	/// \brief Destructor (virtual)
	~SPWMDev() override;

	/// \brief Returns number of channels configured.
	/// \return Number of channels configured.
	size_t get_channel_count() const;

	/// \brief Returns channel information
	/// \param channel_index - channel index, must be in range [0 ... #SPWM_CHANNEL_COUNT)
	/// \return #tag_SPWM_SW_DESCRIPTOR structure that describes requested channel.
    const SPWM_SW_DESCRIPTOR* get_channel_info(size_t channel_index);

    /// \brief Set SPWM channels
    /// \param state - Input/output map with channel indexes as keys, and new channel pwm value as value [0 ... 65535].
    ///                On output it will contain all the channels values.
	void set(SPWM_STATE& state);

	/// \brief Reset all channels to their default state
	void reset();

	void set_pwm_freq(double freq);

    /// \brief Returns device name as specified in JSON configuration file
    /// \return string with name
    std::string get_dev_name() const override;

    uint16_t max_period = 0xFFFF;
};

/// @}

#endif
