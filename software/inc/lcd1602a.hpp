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
 *   \brief LCD1602ADev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include "ekit_device.hpp"
#include "tools.hpp"
#include "sw.h"

/// \defgroup group_lcd_1602a_dev LCD1602ADev
/// \brief Simple LCD1602a screen support
/// @{
/// \page page_lcd1602a_dev
/// \tableofcontents
///
/// \section sect_lcd1602a_dev_01 Work with LCD1602ADev
/// LCD1602a is two lines with sixteen character length LCD screen. Enough to make some simple menu, to output short status messages,
/// local time and many other things to provide informational functionality. Of cause it won't give you smooth and nice
/// graphical interface, but it is not necessary, especially when thing must just work.
///
/// Main features of #LCD1602ADev are:
/// - 4-bit mode is used
/// - It's possible to set back light in on/off/blink mode
///
/// To use it follow these instructions:
/// 1. Create #LCD1602ADev object
/// 2. Call LCD1602ADev#light() in order to control backlight.
/// 3. Call one of the LCD1602ADev#write() methods to output all the lines
/// 4. It is possible to write just several characters, it makes possible quick information updates. To do this use
///    LCD1602ADev#writepos()

#ifdef LCD1602a_DEVICE_ENABLED

/// \class LCD1602ADev
/// \brief LCD1602ADev implementation.
class LCD1602ADev final : public EKitVirtualDevice {
	typedef EKitVirtualDevice super;

	int light_mode = LCD1602a_OFF;
	std::vector<std::string> empty_screen;

	public:

    /// \brief No default constructor
    LCD1602ADev() = delete;

    /// \brief Copy construction is forbidden
    LCD1602ADev(const LCD1602ADev&) = delete;

    /// \brief Assignment is forbidden
    LCD1602ADev& operator=(const LCD1602ADev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the LCD1602ADev device.
	LCD1602ADev(std::shared_ptr<EKitBus>& ebus, int addr);

	/// \brief Destructor (virtual)
	~LCD1602ADev() override;

	/// \brief Sets backlight mode
	/// \param lmode - one of these values: @ref LCD1602a_LIGHT, @ref LCD1602a_OFF, @ref LCD1602a_BLINK.
	void light(int lmode);

	/// \brief Writes strings from container using iterators.
	/// \param first - iterator to the top string.
	/// \param last - iterator to the bottom string.
	void write(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last);

	/// \brief Writes strings from vector of strings
	/// \param lines - vector of strings
	void write(const std::vector<std::string>& lines);

	/// \brief Writes text at specified position
	/// \param line - number of the lines. Must be in range between #LCD1602a_POSITION_MINLINE and #LCD1602a_POSITION_MAXLINE.
	/// \param pos - Position withing this line
	/// \param s - string to output
	void writepos(uint8_t line, uint8_t pos, const std::string& s);

	/// \brief Clears LCD screen
	void clear();

	/// \brief Returns number of lines
	/// \return number of lines
	int nlines() const;

	/// \brief Returns number of characters in line
	/// \return number of characters
	int nchars() const;

    /// \brief Returns device name as specified in JSON configuration file
    /// \return string with name
    std::string get_dev_name() const override;
};

/// @}

#endif