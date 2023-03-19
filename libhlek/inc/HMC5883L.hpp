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
 *   \brief HMC5883L 3-axis compass support.
 *   \author Oleh Sharuda
 */
#pragma once

#include <time.h>
#include <memory>
#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include "tools.hpp"

/// \defgroup group_hmc5883l HMC5883L
/// \brief HMC5883L support
/// @{
/// \page page_hmc5883l
/// \tableofcontents
///
/// \section sect_hmc5883l_01 Work with HMC5883L
///
/// Documentation to be written
/// <CHECKIT> complete documentation ...
///

/// \class HMC5883L
/// \brief HMC5883L support.
class HMC5883L final : public EKitDeviceBase {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitDeviceBase  super;

    public:
    /// \brief No default constructor
    HMC5883L()                          = delete;

    /// \brief Copy construction is forbidden
    HMC5883L(const HMC5883L&)            = delete;

    /// \brief Assignment is forbidden
    HMC5883L& operator=(const HMC5883L&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means
    /// infinite timeout. \param name - name of the device
    HMC5883L(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name);

    /// \brief Destructor (virtual)
    ~HMC5883L() override;
};

/// @}
