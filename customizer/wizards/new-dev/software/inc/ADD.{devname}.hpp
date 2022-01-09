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
 *   \brief {DevName} device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "sw.h"

#ifdef {DEVNAME}_DEVICE_ENABLED

/// \defgroup group_{devname} {DevName}Dev
/// \brief {DevName} support
/// @{
/// \page page_{devname}
/// \tableofcontents
///
/// \section sect_{devname}_01 Work with {DevName}Dev
///
/// {DevName}Dev functionality provides the following features:
/// - features list ...
///
/// Basic logic of {DevName}Dev functionality work is shown on the following schema:
/// \image html {DevName}Dev_schema.png
/// \image latex {DevName}Dev_schema.eps
///
/// {DevName}Dev can be used as:
/// 1. Create {DevName}Dev object
/// 2. Call {DevName}Dev#do_something() method to do something.
///

/// \class {DevName}Dev
/// \brief {DevName}Dev implementation. Use this class in order to control {DevName}Dev virtual devices.
class {DevName}Dev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	/// \brief Pointer to the #tag_{DevName}Instance structure that describes {DevName}Dev virtual device represented by this class.
	const {DevName}Instance* descr = nullptr;

	public:

    /// \brief No default constructor
    {DevName}Dev() = delete;

    /// \brief Copy construction is forbidden
    {DevName}Dev(const {DevName}Dev&) = delete;

    /// \brief Assignment is forbidden
    {DevName}Dev& operator=(const {DevName}Dev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the {DevName}Dev device.
	{DevName}Dev(std::shared_ptr<EKitBus>& ebus, int addr);

    /// \brief Destructor (virtual)
	~{DevName}Dev() override;

	static const {DevName}Instance* get_descriptor(size_t index);
	std::string get_dev_name() const;

	/// \brief do_something
	void do_something();
};

/// @}

#endif
