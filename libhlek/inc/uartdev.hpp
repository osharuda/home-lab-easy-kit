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
 *   \brief UARTDev software implementation header
 *   \author Oleh Sharuda
 */

#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include <memory>
#include <unicode/regex.h>
#include "tools.hpp"
#include "uart_proxy_common.hpp"

/// \defgroup group_uart_proxy_dev UARTDev
/// \brief UART proxy support
/// @{
/// \page page_uart_proxy_dev
/// \tableofcontents
///
/// \section sect_uart_proxy_dev_01 Work with UARTDev
/// #UARTDev allows to read and write data from UART device connected to STM32F103x.
/// In order to access connected device use the following scenario:
/// 1. Create instance of the #UARTDev class.
/// 2. Call UARTDev#read() in order to read all the data available from UARTDev circular buffer.
/// 3. Call UARTDev#write() in order to write data to device.
///
/// Note, dislike #GSMModem, #UARTDev may be used with #EKitFirmware only.
///

/// \class UARTDev
/// \brief UARTDev implementation.
class UARTDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
    typedef EKitVirtualDevice super;

public:

    const UARTProxyConfig* config;    ///< Pointer to the device descriptor structure.

    /// \brief No default constructor
    UARTDev() = delete;

    /// \brief Copy construction is forbidden
    UARTDev(const UARTDev&) = delete;

    /// \brief Assignment is forbidden
    UARTDev& operator=(const UARTDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
    UARTDev(std::shared_ptr<EKitBus>& ebus, const UARTProxyConfig* config);

    /// Destructor (virtual)
    ~UARTDev() override;

    /// \brief Read all the data available in UARTDev circular buffer.
    /// \param data - output vector with read data. (all previous data is discarded)
    void read(std::vector<uint8_t>& data);

    /// \brief Writes data to the UARTDev
    /// \param data - vector with data to write.
    void write(const std::vector<uint8_t>& data);
};

/// @}
