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

EKIT_ERROR EKitBus::read(std::vector<uint8_t>& buffer) {
	return read(buffer.data(), buffer.size());
}
EKIT_ERROR EKitBus::write(const std::vector<uint8_t>& buffer) {
	return write(buffer.data(), buffer.size());
}

EKitBus::EKitBus() {
}

EKitBus::~EKitBus() {
}

EKIT_ERROR EKitBus::lock(int address) {
	bus_lock.lock();
	return EKIT_OK;
}

EKIT_ERROR EKitBus::unlock() {
	bus_lock.unlock();
	return EKIT_OK;
}
