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
 *   \brief HMC5883L 3-axis compass support.
 *   \author Oleh Sharuda
 */

#include "HMC5883L.hpp"
#include <math.h>
#include "ekit_bus.hpp"
#include "ekit_error.hpp"

HMC5883L::HMC5883L(std::shared_ptr<EKitBus>& ebus,
                 int                       timeout_ms,
                 const char*               name)
    : super(ebus, name) {
    static const char* const func_name = "HMC5883L::HMC5883L";

    ebus->check_bus(EKitBusType::BUS_I2C);

    EKitTimeout to(get_timeout());
    BusLocker   blocker(ebus, to);
}

HMC5883L::~HMC5883L() {}
