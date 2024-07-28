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
 *   \brief INFODev software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include "ekit_device.hpp"
#include "info_common.hpp"

/// \defgroup group_info_dev INFODev
/// \brief Firmware identification and information support
/// @{
/// \page page_info_dev
/// \tableofcontents
///
/// \section sect_info_dev_01 Work with INFODev
/// INFODev provides ability to check if firmware and software was generated using the same JSON configuration file.
/// Resulting JSON configuration file hash sum is calculated during custumization and stored into INFODev functionality.
/// This will help to protect against mistakes when software and firmware do not match.
///
/// Also, it is possible to get brief device description by device id value.
///

/// \class INFODev
/// \brief INFODev implementation.
class INFODev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

    public:

    const InfoConfig* config;

    /// \brief No default constructor
    INFODev() = delete;

    /// \brief Copy construction is forbidden
    INFODev(const INFODev&) = delete;

    /// \brief Assignment is forbidden
    INFODev& operator=(const INFODev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
    INFODev(std::shared_ptr<EKitBus>& ebus, const InfoConfig* config);

    /// \brief Destructor (virtual)
    ~INFODev() override;

    /// \brief Checks if software and firmware was generated using the same JSON configuration file.
    /// \details If check fails or mismatch is detected corresponding #EKitException exception is thrown.
    void check();

    /// \brief Checks if the device of the specified type is being used
    bool is_available(uint8_t dev_type);

    /// \brief Return virtual device information by device id value
    /// \param dev_id - device id
    /// \return pointer to the #tag_InfoDeviceDescriptor structure that describes virtual device
    const struct InfoDeviceDescriptor* get_device_info(size_t dev_id);
};

/// @}
