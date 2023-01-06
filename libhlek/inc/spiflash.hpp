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

/// \defgroup group_spi_flash SPIFlash
/// \brief Some SPI flash chips support
/// @{
/// \page page_spi_flash
/// \tableofcontents
///
/// \section sect_spi_flash_01 Work with SPIFlash
///
/// Documentation to be written
/// <CHECKIT> complete documentation ...
///

struct SPIFLASHDescriptor {
    uint8_t write_cmd;
    uint8_t read_cmd;
    uint8_t status_cmd;
    uint8_t write_enable_cmd;
    uint8_t write_disable_cmd;
    uint16_t start_address;
    uint16_t page_size;
    uint16_t page_count;
};
typedef std::map<uint8_t , SPIFLASHDescriptor> SpiFlashDescriptorMap;

/// \class SPIFlash
/// \brief SPIFlash support.
class SPIFlash final : public EKitDeviceBase {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitDeviceBase super;

    const uint8_t flash_kind;
    int timeout;

    uint8_t status(EKitTimeout& to);
    void read_spi(std::vector<uint8_t>& spi_data, size_t length, EKitTimeout& to, int data_offset = -1);

    static const SpiFlashDescriptorMap flash_map;
    SPIFLASHDescriptor flash_descriptor;
public:

    /// \brief No default constructor
    SPIFlash() = delete;

    /// \brief Copy construction is forbidden
    SPIFlash(const SPIFlash&) = delete;

    /// \brief Assignment is forbidden
    SPIFlash& operator=(const SPIFlash&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param name - name of the device
    SPIFlash(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name, const uint8_t hint);

    /// \brief Destructor (virtual)
	~SPIFlash() override;

    void read(uint16_t address, uint16_t len, std::vector<uint8_t>& data);
    void write(uint16_t address, const std::vector<uint8_t>& data);
};

/// @}
