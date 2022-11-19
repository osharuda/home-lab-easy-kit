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
 *   \brief RTCDev software implementation
 *   \author Oleh Sharuda
 */

#include <time.h>
#include <cstdint>
#include "rtc_common.hpp"
#include "rtc.hpp"
#include "ekit_error.hpp"

RTCDev::RTCDev(std::shared_ptr<EKitBus>& ebus, const RTCConfig* config)  : super(ebus, config->device_id, config->device_name) {}

RTCDev::~RTCDev() {}

uint32_t RTCDev::now_priv() {
	static const char* const func_name = "RTCDev::now_priv";
	EKIT_ERROR err;
	RtcData data;

	// write nothing to update rtc value in MCU
	err = bus->write(nullptr, 0);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }

	// read updated value
	err = bus->read(&data, sizeof(data));
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }

    return data.rtcval;
}

std::time_t RTCDev::now() {
	static const char* const func_name = "RTCDev::now";

	BusLocker blocker(bus, get_addr());	
	uint32_t secs = now_priv();
	return static_cast<std::time_t>(secs);
}


std::time_t RTCDev::sync_host() {
	static const char* const func_name = "RTCDev::sync_host";
	BusLocker blocker(bus, get_addr());
    uint32_t secs = now_priv();
    const timespec ts{static_cast<time_t>(secs),0};


    int res = clock_settime(CLOCK_REALTIME, &ts);
	if (res!=0) {
		throw EKitException(func_name, strerror(errno));
	}

	return static_cast<std::time_t>(secs);
}

std::time_t RTCDev::sync_rtc() {
	static const char* const func_name = "RTCDev::sync_rtc";
	EKIT_ERROR err;
	std::time_t val = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());	
	RtcData data;
	data.rtcval = static_cast<uint32_t>(val);	

	BusLocker blocker(bus, get_addr());
	err = bus->write(&data, sizeof(data));
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }

    return val;
}
