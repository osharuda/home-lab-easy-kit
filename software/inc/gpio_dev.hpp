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
 *   \brief GPIO device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <bitset>
#include "ekit_device.hpp"
#include "sw.h"

#ifdef GPIODEV_DEVICE_ENABLED
/// \defgroup group_gpio_dev GPIODev
/// \brief General purpose input/output support
/// @{
/// \page page_gpio_dev
/// \tableofcontents
///
/// \section sect_gpio_dev_01 Work with GPIODev
///
/// GPIODev provides possibility to use STM32F103x pins as GPIO lines. Use of the GPIODev is pretty straightforward:
/// 1. Create instance of the #GPIODev class.
/// 2. Use calls GPIODev#get_gpio_count() and GPIODev#get_gpio_info() in order to get information about configured pins.
/// 3. Call GPIODev#read() in order to read state of the pins.
/// 4. Call GPIODev#write() in order to change state of the output pins.
///

/// \struct tag_GPIO_descr
/// \brief Describes configured GPIO line.
typedef struct tag_GPIO_descr {
    uint8_t pin_id;         ///< Pin identifier or pin index. Must be in range [0 ... #GPIO_PIN_COUNT). Note, there are constants
                            ///  defined for each pin_id here @ref group_gpio_dev_pin_indexes.
    uint8_t pin_type;       ///< Type of the pin. Must be either #GPIO_DEV_INPUT or #GPIO_DEV_OUTPUT.
    uint8_t default_val;    ///< Default pin value. Is meaningful for outputs only.
    const char* pin_name;   ///< Name of the pin given in JSON configuration file.
} GPIO_descr;

/// \class GPIODev
/// \brief GPIO support implementation.
class GPIODev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

    static constexpr GPIO_descr gpio_pin_descriptors[] = GPIO_DESCRIPTOR; ///< Static array with pin descriptors

    static const size_t gpio_buffer_size;           ///< Precalculated buffer size

public:

    /// \brief No default constructor
    GPIODev() = delete;

    /// \brief Copy construction is forbidden
    GPIODev(const GPIODev&) = delete;

    /// \brief Assignment is forbidden
    GPIODev& operator=(const GPIODev&) = delete;


    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the GPIODev device.
	GPIODev(std::shared_ptr<EKitBus>& ebus, int addr);

	/// \brief Destructor (virtual)
	~GPIODev() override;

	/// \brief Returns number of gpio lines configured.
	/// \return number of gpio lines.
	size_t get_gpio_count() const;

	/// \brief Returns information regarding number of gpio lines configured.
	/// \param pin_index - pin index, correspond to tag_GPIO_descr#pin_id.
	/// \return
	const GPIO_descr* get_gpio_info(size_t pin_index) const;

	/// \brief Read pins value from firmware.
	/// \param pins - std::bitset with pin values (accessed by pin index).
	void read(std::bitset<GPIO_PIN_COUNT>& pins);

	/// \brief Writes new values to output pins.
	/// \param pins - std::bitset with new output pin values (accessed by pin index).
	void write(const std::bitset<GPIO_PIN_COUNT>& pins);

    /// \brief Returns device name as specified in JSON configuration file
    /// \return string with name
    std::string get_dev_name() const override;
};

/// @}

#endif