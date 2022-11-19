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
#include "spwm_common.hpp"

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

struct PWM_ENTRY_HELPER final {

    PWM_ENTRY_HELPER() :
        buffer(nullptr),
        ppwm_entry(nullptr){
    }

    PWM_ENTRY_HELPER(size_t pn) {
        allocate(pn);
    }

    PWM_ENTRY_HELPER(const PWM_ENTRY_HELPER& that){
        assert(that.buffer);
        allocate(that.port_number);
        memcpy(buffer, that.buffer, PWM_ENTRY_SIZE(port_number));
    }

    PWM_ENTRY_HELPER(PWM_ENTRY_HELPER&& that) :
    buffer(that.buffer),
    ppwm_entry(that.ppwm_entry),
    port_number(that.port_number){
        that.buffer = nullptr;
        that.ppwm_entry = nullptr;
        that.port_number = 0;
    }

    ~PWM_ENTRY_HELPER() {
        if (buffer) {
            delete[] buffer;
            clear();
        }
    }

    inline void allocate(size_t port_number) {
        assert(port_number > 0);
        size_t buffer_len = PWM_ENTRY_SIZE(port_number);
        buffer = new uint8_t[buffer_len];
        memset(buffer, 0, buffer_len);
        ppwm_entry = reinterpret_cast<PPWM_ENTRY>(buffer);
    }

    inline void assign(uint8_t* ptr, size_t port_number) {
        assert(port_number > 0);
        size_t buffer_len = PWM_ENTRY_SIZE(port_number);
        ppwm_entry = reinterpret_cast<PPWM_ENTRY>(buffer);
        memset(ptr, 0, buffer_len);
    }

    PPWM_ENTRY data() {
        return ppwm_entry;
    }

    size_t get_port_number() const {
        return port_number;
    }

    static std::vector<uint8_t> join(PWM_ENTRY_HELPER* entries, size_t number) {
        assert(number>0);
        size_t port_number = entries[0].get_port_number();

#ifndef NDEBUG
        for (size_t i=1; i<number; i++) {
            assert(entries[i].get_port_number()==port_number);
        }
#endif

        size_t e_size = PWM_ENTRY_SIZE(port_number);
        size_t data_size = number * e_size;
        std::vector<uint8_t> result(data_size, 0);
        uint8_t* data_ptr = result.data();
        for (size_t i=0, offset = 0; i<number; i++, offset+=e_size) {
            memcpy(data_ptr+offset, const_cast<PWM_ENTRY*>(entries[i].data()), e_size);
        }
        return result;
    }

private:
    inline void clear() {
        buffer = nullptr;
        ppwm_entry = nullptr;
        port_number = 0;
    }

    uint8_t* buffer = nullptr;
    PPWM_ENTRY ppwm_entry = nullptr;
    size_t port_number = 0;
};



/// \class SPWMDev
/// \brief SPWMDev implementation
class SPWMDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	/// \brief Array of current PWM values for each channel.
    std::vector<uint16_t> prev_data;

    /// \brief Sets stored PWM values to default state.
    void clear_prev_data();

public:

    const SPWMConfig* config;

    /// \brief No default constructor
    SPWMDev() = delete;

    /// \brief Copy construction is forbidden
    SPWMDev(const SPWMDev&) = delete;

    /// \brief Assignment is forbidden
    SPWMDev& operator=(const SPWMDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	SPWMDev(std::shared_ptr<EKitBus>& ebus, const SPWMConfig* config);

	/// \brief Destructor (virtual)
	~SPWMDev() override = default;

	/// \brief Returns number of channels configured.
	/// \return Number of channels configured.
	size_t get_channel_count() const;

	/// \brief Returns channel information
	/// \param channel_index - channel index, must be in range [0 ... channel count)
	/// \return #tag_SPWMChannel structure that describes requested channel.
    const SPWMChannel* get_channel_info(size_t channel_index);

    /// \brief Set SPWM channels
    /// \param state - Input/output map with channel indexes as keys, and new channel pwm value as value [0 ... 65535].
    ///                On output it will contain all the channels values.
	void set(SPWM_STATE& state);

	/// \brief Reset all channels to their default state
	void reset();

	void set_pwm_freq(double freq);

    uint16_t max_period = 0xFFFF;
};

/// @}
