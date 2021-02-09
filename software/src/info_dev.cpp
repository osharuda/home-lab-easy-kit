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
 *   \brief INFODev software implementation
 *   \author Oleh Sharuda
 */

#include "info_dev.hpp"
#include "ekit_firmware.hpp"

#ifdef INFO_DEVICE_ENABLED

constexpr InfoProjectData INFODev::info;

INFODev::INFODev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
    static const char *const func_name = "INFODev::INFODev";
}

INFODev::~INFODev() {
}

void INFODev::check() {
    static const char* const func_name = "INFODev::check";
    EKIT_ERROR err;
    uint8_t uuid[INFO_UUID_LEN];

    BusLocker blocker(bus, get_addr());

    err = bus->read(uuid, INFO_UUID_LEN);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }

    if (memcmp(uuid, info.uuid, INFO_UUID_LEN)!=0) {
        throw EKitException(func_name, EKIT_FAIL, "wrong build of the firmware");
    }
}

std::string INFODev::get_dev_name() const {
    return INFODev::info.name;
}

const PInfoDeviceDescriptor INFODev::get_device_info(size_t dev_id) {
    static const char *const func_name = "INFODev::get_device_info";
    if (dev_id >= INFO_DEVICES_NUMBER) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "dev_id is out of possible values range");
    }

    return const_cast<const PInfoDeviceDescriptor>(info.devices+dev_id);
}

#endif

