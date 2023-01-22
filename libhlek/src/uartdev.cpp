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
 *   \brief UARTDev software implementation
 *   \author Oleh Sharuda
 */

#include "uartdev.hpp"
#include "ekit_error.hpp"
#include "ekit_firmware.hpp"

UARTDev::UARTDev(std::shared_ptr<EKitBus>& ebus, const UARTProxyConfig* cfg) :
    EKitBus(EKitBusType::BUS_UART),
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
    static const char* const func_name = "UARTDev::UARTDev";
    ebus->check_bus(EKitBusType::BUS_I2C_FIRMWARE);
}

UARTDev::~UARTDev() {
}
/*
void UARTDev::read(std::vector<uint8_t>& data) {
    EKIT_ERROR err;
    static const char* const func_name = "UARTDev::read";

    // Lock bus
    BusLocker blocker(bus, to);

    // Read data
    err = bus->read_all(data);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }
}

void UARTDev::write(const std::vector<uint8_t>& data) {
    EKIT_ERROR err;
    static const char* const func_name = "UARTDev::write";
    // Lock bus
    BusLocker blocker(bus, to);

    // Send data
    err = bus->write(data.data(), data.size());
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}
*/

EKIT_ERROR UARTDev::read(void *ptr, size_t len, EKitTimeout& to) {
    static const char* const func_name = "UARTDev::read";
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    assert(false); // MUST BE IMPLEMENTED
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR UARTDev::write(const void *ptr, size_t len, EKitTimeout& to) {
    static const char* const func_name = "UARTDev::write";

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    // Send data
    return bus->write(ptr, len, to);
}

EKIT_ERROR UARTDev::read_all(std::vector<uint8_t> &buffer, EKitTimeout& to) {
    static const char* const func_name = "UARTDev::read_all";

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    // Read data
    return bus->read_all(buffer, to);
}

EKIT_ERROR UARTDev::write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to) {
    static const char* const func_name = "UARTDev::write_read";
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    assert(false); // MUST BE IMPLEMENTED
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR UARTDev::lock(EKitTimeout& to) {
    return std::dynamic_pointer_cast<EKitFirmware>(bus)->lock(get_addr(), to);
}
