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
 *   \brief AD9850Dev device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "ad9850_common.hpp"

/// \defgroup group_ad9850dev AD9850Dev
/// \brief AD9850Dev support
/// @{
/// \page page_ad9850dev
/// \tableofcontents
///
/// \section sect_ad9850dev_01 Work with AD9850Dev
///
/// AD9850/AD9851 are complete DDS synthesizers that allow to generate sin (cos) and meander signals. It may be used
/// to analyzes frequency/phase response for different applications.
///
/// Work with AD9850 is easy:
/// 1. Reset device with \ref AD9850Dev::reset() call if required. AD9850/AD9851 is reset on MCU start up. Reset puts device into power down
///    mode.
/// 2. Set frequency and phase with \ref AD9850Dev::update() method. This call will set AD9850/AD9851 into power up mode and will
///    set desired frequency.

/// \class AD9850Dev
/// \brief AD9850Dev implementation. Use this class in order to control AD9850Dev virtual devices.
class AD9850Dev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
    typedef EKitVirtualDevice super;

    public:

    /// \brief Pointer to the #tag_AD9850Config structure that describes AD9850Dev virtual device represented by this class.
    const AD9850Config* config;

    /// \brief No default constructor
    AD9850Dev() = delete;

    /// \brief Copy construction is forbidden
    AD9850Dev(const AD9850Dev&) = delete;

    /// \brief Assignment is forbidden
    AD9850Dev& operator=(const AD9850Dev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
    AD9850Dev(std::shared_ptr<EKitBus>& ebus, const AD9850Config* config);

    /// \brief Destructor (virtual)
    ~AD9850Dev() override;

    /// \brief Reset state of the device
    /// \note AD9851 will be put in power down mode
    void reset();

    /// \brief Update frequency and phase
    /// \param frequency - frequency in hertz
    /// \param phase - phase in radians
    /// \note phase will be rounded to N*Pi/16 <TODO> Make sure phase is actually rounded, not tail or ceil
    void update(double frequency, double phase);
};

/// @}
