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
#include "{devname}_common.hpp"

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

/// \class {DevName}
/// \brief {DevName} implementation. Use this class in order to control {DevName} virtual devices.
class {DevName} final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	public:

	/// \brief Pointer to the #tag_{DevName}Config structure that describes {DevName} virtual device represented by this class.
	const {DevName}Config* config;

    /// \brief No default constructor
    {DevName}() = delete;

    /// \brief Copy construction is forbidden
    {DevName}(const {DevName}&) = delete;

    /// \brief Assignment is forbidden
    {DevName}& operator=(const {DevName}&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	{DevName}(std::shared_ptr<EKitBus>& ebus, const {DevName}Config* config);

    /// \brief Destructor (virtual)
	~{DevName}() override;
};

/// @}
