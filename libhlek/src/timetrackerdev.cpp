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
 *   \brief TimeTrackerDev device software implementation
 *   \author Oleh Sharuda
 */

#include "timetrackerdev_common.hpp"
#include "timetrackerdev.hpp"
#include "ekit_firmware.hpp"
#include <math.h>

TimeTrackerDev::TimeTrackerDev(std::shared_ptr<EKitBus>& ebus, const TimeTrackerDevConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg),
    raw_buffer(cfg->dev_buffer_len + sizeof(TimeTrackerStatus)){
    dev_status = (PTimeTrackerStatus)raw_buffer.data();
    data_buffer = (uint64_t*)(raw_buffer.data() + sizeof(TimeTrackerStatus));
}

TimeTrackerDev::~TimeTrackerDev() {
}

void TimeTrackerDev::send_command(int flags) {
    static const char* const func_name = "TimeTrackerDev::send_command";
    EKitTimeout to(get_timeout());
    BusLocker   blocker(bus, get_addr(), to);

    EKIT_ERROR  err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, flags, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    // Write data
    err = bus->write(nullptr, 0, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void TimeTrackerDev::start(bool reset) {
    int flags = TIMETRACKERDEV_START | (reset ? TIMETRACKERDEV_RESET : 0);
    send_command(flags);
}

void TimeTrackerDev::stop() {
    send_command(0);
}

void TimeTrackerDev::get_priv(size_t count, EKitTimeout& to) {
    static const char* const func_name = "TimeTrackerDev::get_priv";
    size_t buffer_data_size = sizeof(uint64_t)*count;

    if (buffer_data_size <= config->dev_buffer_len) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Internal buffer is not sufficient for this request.");
    }

    // get amount of data
    size_t data_size = sizeof(TimeTrackerStatus) + buffer_data_size;

    // read data into internal buffer
    EKIT_ERROR err = bus->read((uint8_t*)dev_status, data_size, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }

    if (count > dev_status->event_number) {
        throw EKitException(func_name, EKIT_NO_DATA, "Number of samples requested is less than size of the data available in device");
    }
}

size_t TimeTrackerDev::get_status(bool& running, uint64_t& reset_ts) {
    EKitTimeout to(get_timeout());
    BusLocker          blocker(bus, get_addr(), to);

    get_priv(0, to);

    running = (dev_status->status == TIMETRACKERDEV_STATUS_STARTED);
    reset_ts = dev_status->last_reset;
    return dev_status->event_number;
}

void TimeTrackerDev::read_all(std::vector<uint64_t>& data) {
    EKitTimeout to(get_timeout());
    BusLocker          blocker(bus, get_addr(), to);

    get_priv(0, to);
    get_priv(dev_status->event_number, to);
}