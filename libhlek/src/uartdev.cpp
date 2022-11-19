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

UARTDev::UARTDev(std::shared_ptr<EKitBus>& ebus, const UARTProxyConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
    static const char* const func_name = "UARTDev::UARTDev";
}

UARTDev::~UARTDev() {
}

void UARTDev::read(std::vector<uint8_t>& data) {
    EKIT_ERROR err;
    static const char* const func_name = "UARTDev::read";

    // Lock bus
    BusLocker blocker(bus, get_addr());

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
    BusLocker blocker(bus, get_addr());

    // Send data
    err = bus->write(data);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

