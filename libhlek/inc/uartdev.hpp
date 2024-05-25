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
 *   \brief UARTProxyDev software implementation header
 *   \author Oleh Sharuda
 */

#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include <memory>
#include <unicode/regex.h>
#include "tools.hpp"
#include "uart_proxy_common.hpp"

/// \defgroup group_uart_proxy_dev UARTProxyDev
/// \brief UART proxy support
/// @{
/// \page page_uart_proxy_dev
/// \tableofcontents
///
/// \section sect_uart_proxy_dev_01 Work with UARTProxyDev
/// #UARTProxyDev allows to read and write data from UART device connected to STM32F103x.
/// In order to access connected device use the following scenario:
/// 1. Create instance of the #UARTProxyDev class.
/// 2. Call UARTProxyDev#read() in order to read all the data available from UARTProxyDev circular buffer.
/// 3. Call UARTProxyDev#write() in order to write data to device.
///
/// Note, dislike #GSMModem, #UARTProxyDev may be used with #EKitFirmware only.
///

/// \class UARTProxyDev
/// \brief UARTProxyDev implementation.
class UARTProxyDev final : public EKitVirtualDevice,
                           public EKitBus {

    /// \typedef super
    /// \brief Defines parent class
    typedef EKitVirtualDevice super;

public:

    const UARTProxyConfig* config;    ///< Pointer to the device descriptor structure.

    /// \brief No default constructor
    UARTProxyDev() = delete;

    /// \brief Copy construction is forbidden
    UARTProxyDev(const UARTProxyDev&) = delete;

    /// \brief Assignment is forbidden
    UARTProxyDev& operator=(const UARTProxyDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
    UARTProxyDev(std::shared_ptr<EKitBus>& ebus, const UARTProxyConfig* config);

    /// Destructor (virtual)
    ~UARTProxyDev() override;

    /// \brief Implementation of the EKitBus#lock() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note This function is overridden to pass correct address into EKitFirmware
    EKIT_ERROR lock(EKitTimeout& to) override;

    /// \brief Read data from a bus.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR read(void *ptr, size_t len, EKitTimeout& to) override;

    /// \brief Write data to a bus.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR write(const void *ptr, size_t len, EKitTimeout& to) override;

    /// \brief Read all the data available on a bus.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Note every bus may support it. In this case EKIT_NOT_SUPPORTED must be returned.
    EKIT_ERROR read_all(std::vector<uint8_t> &buffer, EKitTimeout& to) override;

    /// \brief Does write and read by single operation, the first write with subsequent read.
    /// \param wbuf - memory to write.
    /// \param wlen - length of the write buffer.
    /// \param rbuf - memory to read data (may be the same pointer as write buffer, wbuf).
    /// \param rlen - length of the buffer to read data into (amount of data to read).
    /// \param to - timeout counting object.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to)  override;
};

/// @}
