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
 *   \brief SPIProxy device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "spiproxy_common.hpp"

/// \defgroup group_spiproxy SPIProxyDev
/// \brief SPIProxy support
/// @{
/// \page page_spiproxy
/// \tableofcontents
///
/// \section sect_spiproxy_01 Work with SPIProxyDev
///
/// SPIProxyDev functionality provides the following features:
/// - features list ...
///
/// Basic logic of SPIProxyDev functionality work is shown on the following schema:
/// \image html SPIProxyDev_schema.png
/// \image latex SPIProxyDev_schema.eps
///
/// SPIProxyDev can be used as:
/// 1. Create SPIProxyDev object
/// 2. Call SPIProxyDev#do_something() method to do something.
///

/// \class SPIProxyDev
/// \brief SPIProxyDev implementation. Use this class in order to control SPIProxyDev virtual devices.
class SPIProxyDev final : public EKitVirtualDevice,
                          public EKitBus {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

    EKitBusState state;                 ///< Bus state
    int timeout = -1;                   ///< Timeout in milliseconds, infinite by default
    std::vector<uint8_t> recv_buffer;   ///< Receive buffer

    /// \brief Waits until SPI transaction is finished or either timeout expires or error occurs.
    /// \param sw - current timer used to detect operation timeouts.
    EKIT_ERROR spi_proxy_wait(tools::StopWatchMs& sw);

	public:

    /// \brief Pointer to the #tag_SPIProxyConfig structure that describes SPIProxy virtual device represented by this class.
    const SPIProxyConfig* config;

    /// \brief No default constructor
    SPIProxyDev() = delete;

    /// \brief Copy construction is forbidden
    SPIProxyDev(const SPIProxyDev&) = delete;

    /// \brief Assignment is forbidden
    SPIProxyDev& operator=(const SPIProxyDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	SPIProxyDev(std::shared_ptr<EKitBus>& ebus, const SPIProxyConfig* config);

    /// \brief Destructor (virtual)
	~SPIProxyDev() override;

    /// \brief Write data to a bus (implementation of the EKitBus).
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR write(const void* ptr, size_t len) override;

    using EKitBus::read;

    /// \brief Read data from a bus (implementation of the EKitBus)..
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR read(void* ptr, size_t len) override;

    /// \brief Read all the data available on a bus (implementation of the EKitBus)..
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Note every bus may support it. In this case EKIT_NOT_SUPPORTED must be returned.
    virtual EKIT_ERROR read_all(std::vector<uint8_t>& buffer) override;

    /// \brief Set a bus specific option.
    /// \param opt - bus specific option.
    /// \param value - bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR set_opt(int opt, int value) override;

    /// \brief Reads a bus specific option.
    /// \param opt - bus specific option.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR get_opt(int opt, int& value) override;

    /// \brief Returns information about implemented bus.
    /// \param busid - One of the #EKitBusType values.
    /// \return - a bit mask of #EKitBusProperties flags.
    virtual int bus_props(int& busid) const override;
};

/// @}
