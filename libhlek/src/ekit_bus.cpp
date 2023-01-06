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
 *   \brief EKitBus software implementation
 *   \author Oleh Sharuda
 */

#include "ekit_bus.hpp"

EKitBus::EKitBus(const EKitBusType bt) :
    bus_type(bt){
}

EKitBus::~EKitBus() {
}

EKIT_ERROR EKitBus::lock(EKitTimeout& to) {
	bus_lock.lock();
	return EKIT_OK;
}

EKIT_ERROR EKitBus::lock(int addr, EKitTimeout& to) {
    assert(false);
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitBus::unlock() {
	bus_lock.unlock();
	return EKIT_OK;
}

EKIT_ERROR EKitBus::open(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
	if (state!=BUS_CLOSED) {
		return EKIT_ALREADY_CONNECTED;
	} else {
		state = BUS_OPENED;
		return EKIT_OK;
	}
}

EKIT_ERROR EKitBus::close() {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    if (state==BUS_CLOSED) {
            return EKIT_DISCONNECTED;
    } else {
            state = BUS_CLOSED;
            return EKIT_OK;
    }
}

EKIT_ERROR EKitBus::suspend(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	if (state==BUS_CLOSED) {
		return EKIT_DISCONNECTED;
	}

	if (state==BUS_PAUSED) {
		return EKIT_SUSPENDED;
	}

	state = BUS_PAUSED;

	return EKIT_OK;
}

EKIT_ERROR EKitBus::resume(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	if (state==BUS_CLOSED) {
		return EKIT_DISCONNECTED;
	}

	if (state==BUS_OPENED) {
		return EKIT_SUSPENDED;
	}

	state = BUS_OPENED;

	return EKIT_OK;
}

EKIT_ERROR EKitBus::set_opt(int opt, int value, EKitTimeout& to) {
	return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitBus::get_opt(int opt, int& value, EKitTimeout& to) {
	return EKIT_NOT_SUPPORTED;
}

void EKitBus::check_bus(const EKitBusType busid) const {
    static const char* const func_name = "EKitBus::check_bus";
    if (busid != bus_type) {
        throw EKitException(func_name, EKIT_WRONG_DEVICE, "Wrong busid is specified. This bus is not the requested bus type.");
    }
}
