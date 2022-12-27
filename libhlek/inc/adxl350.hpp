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
 *   \brief Some SPI flash chips support.
 *   \author Oleh Sharuda
 */

#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include <memory>
#include "tools.hpp"

/// \defgroup group_adxl350 SPIFlash
/// \brief ADXL350 support
/// @{
/// \page page_adxl350
/// \tableofcontents
///
/// \section sect_adxl350_01 Work with ADXL350
///
/// Documentation to be written
/// <CHECKIT> complete documentation ...
///

/// \class ADXL350
/// \brief ADXL350 support.
class ADXL350 final : public EKitDeviceBase {

    /// \typedef super
    /// \brief Defines parent class
    typedef EKitDeviceBase super;

    int timeout;
public:

    /// \brief No default constructor
    ADXL350() = delete;

    /// \brief Copy construction is forbidden
    ADXL350(const ADXL350&) = delete;

    /// \brief Assignment is forbidden
    ADXL350& operator=(const ADXL350&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param name - name of the device
    ADXL350(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name, const uint8_t hint);

    /// \brief Destructor (virtual)
    ~ADXL350() override;

    /// \brief Returns device name
    /// \return string with name
    std::string get_dev_name() const override;
};

/// @}