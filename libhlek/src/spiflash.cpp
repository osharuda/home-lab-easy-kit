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
 *   \brief Some SPI Flash chips support
 *   \author Oleh Sharuda
 */

#include "spiflash.hpp"
#include "ekit_error.hpp"
#include "ekit_bus.hpp"
#include "info_dev.hpp"


const SpiFlashDescriptorMap SPIFlash::flash_map = {
        {INFO_DEV_HINT_25LC640,
         {
                .write_cmd = 2,
                .read_cmd = 3,
                .status_cmd = 5,
                .write_enable_cmd = 6,
                .write_disable_cmd = 7,
                .start_address = 0,
                .page_size = 32,
                .page_count = 256
            }
        }
    };

SPIFlash::SPIFlash(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name, const uint8_t hint) :
    super(ebus, name),
    timeout(timeout_ms),
    flash_kind(hint){
    static const char* const func_name = "SPIFlash::SPIFlash";

    ebus->check_bus(EKitBusType::BUS_SPI);

    SpiFlashDescriptorMap::const_iterator idscr = flash_map.find(hint);
    if (idscr==flash_map.end()) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Not supported device.");
    }
    flash_descriptor = idscr->second;

    // Set the same timeout for underlying bus
    set_timeout(timeout_ms);
}

SPIFlash::~SPIFlash(){
}

void SPIFlash::read_spi(std::vector<uint8_t>& spi_data, size_t length, EKitTimeout& to, int data_offset) {
    static const char* const func_name = "SPIFlash::read_spi";
    size_t data_received = 0;
    EKIT_ERROR err = EKIT_OK;
    spi_data.resize(length);

    // Send command
    err = bus->read(spi_data.data(), length, to);
    if (err!=EKIT_OK) {
        throw EKitException(func_name, err, "Failed to write SPI");
    }

    // Truncate if required
    if (data_offset>0) {
        spi_data.erase(spi_data.begin(), spi_data.begin() + data_offset);
    }
}

uint8_t SPIFlash::status(EKitTimeout& to) {
    static const char* const func_name = "SPIFlash::status";
    EKIT_ERROR err = EKIT_OK;
    std::vector<uint8_t> spi_cmd {flash_descriptor.status_cmd, 0};
    std::vector<uint8_t> spi_data;

    bool to_expired = false;

    // Lock bus
    auto bus = super::get_bus();
    BusLocker blocker(bus, to);

    // Send command
    err = bus->write(spi_cmd.data(), spi_cmd.size(), to);
    if (err!=EKIT_OK) {
        throw EKitException(func_name, err, "Failed to write SPI");
    }

    // Read from SPI
    read_spi(spi_data, spi_cmd.size(), to, 1);

    return spi_data[0];
}

void SPIFlash::read(uint16_t address, uint16_t len, std::vector<uint8_t>& data) {
    static const char* const func_name = "SPIFlash::read";
    EKIT_ERROR err = EKIT_OK;
    size_t data_len = 3 + len;
    data.resize(data_len);
    address += flash_descriptor.start_address;
    data[0] = flash_descriptor.read_cmd;
    data[1] = (uint8_t)(address >> 8);
    data[2] = (uint8_t)(address);

    EKitTimeout to(get_timeout());

    // Lock bus
    BusLocker blocker(bus, to);

    // Send command
    err = bus->write(data.data(), data_len, to);
    if (err!=EKIT_OK) {
        throw EKitException(func_name, err, "Failed to write SPI");
    }

    // Read from SPI
    read_spi(data, data.size(), to, 3);
}

void SPIFlash::write(uint16_t address, const std::vector<uint8_t>& data) {
    static const char* const func_name = "SPIFlash::write";
    EKIT_ERROR err = EKIT_OK;
    size_t length = data.size();
    std::vector<uint8_t> cmd;

    if (address + length > flash_descriptor.page_size*flash_descriptor.page_count) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Attempt to write beyond the flash limits");
    }

    if (length == 0) {
        return;
    }

    cmd.resize(1);
    cmd[0] = flash_descriptor.write_enable_cmd;

    tools::StopWatch<std::chrono::milliseconds> sw(timeout);

    // Lock bus
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, to);

    // Enable write
    err = bus->write(cmd.data(), cmd.size(), to);
    if (err!=EKIT_OK) {
        throw EKitException(func_name, err, "Failed to enable write on flash");
    }

    // Read from SPI
    read_spi(cmd, cmd.size(), to, 0);

    // Do actual write (page by page)
    uint16_t bytes_wrote = 0;
    uint16_t page_address_mask = flash_descriptor.page_size - 1;
    do {
        uint16_t start = address + flash_descriptor.start_address + bytes_wrote;
        uint16_t end = address + flash_descriptor.start_address + (uint16_t)length - bytes_wrote;

        if ((start | page_address_mask) < end) {
            end = start | page_address_mask;
        }
        uint16_t bytes_to_write = end - start;

        cmd.resize(bytes_to_write + 3);
        cmd.at(0) = flash_descriptor.write_cmd;
        cmd.at(1) = (uint8_t) (start >> 8);
        cmd.at(2) = (uint8_t) (start);
        uint8_t* ptr = cmd.data();
        std::memcpy(ptr + 3, data.data()+bytes_wrote, bytes_to_write);

        err = bus->write(cmd.data(), cmd.size(), to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "Failed to write on flash");
        }

        // Wait internal write cycle
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        // Read from SPI
        read_spi(cmd, cmd.size(), to, 0);

        bytes_wrote += bytes_to_write;
    } while (bytes_wrote < length);

    // Disable write
    cmd.resize(1);
    cmd[0] = flash_descriptor.write_disable_cmd;
    err = bus->write(cmd.data(), cmd.size(), to);
    if (err!=EKIT_OK) {
        throw EKitException(func_name, err, "Failed to disable write on flash");
    }

    // Read from SPI
    read_spi(cmd, cmd.size(), to, 0);
}
