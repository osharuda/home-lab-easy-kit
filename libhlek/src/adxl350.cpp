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

#include "adxl350.hpp"
#include "ekit_error.hpp"
#include "ekit_bus.hpp"

ADXL350::ADXL350(std::shared_ptr<EKitBus>& ebus, int timeout_ms, const char* name, const uint8_t hint) :
    super(ebus, name),
    timeout(timeout_ms){
    static const char* const func_name = "ADXL350::ADXL350";

    int busid = 0;
    ebus->bus_props(busid);

    if (busid != EKitBusType::BUS_SPI) {
        throw EKitException(func_name, "Not compatible bus passed: EKitBusType::BUS_SPI is required");
    }

    // Set the same timeout for underlying bus
    BusLocker blocker(ebus);
    ebus->set_opt(EKitBusOptions::EKITBUS_TIMEOUT, timeout);
}

ADXL350::~ADXL350(){
}
