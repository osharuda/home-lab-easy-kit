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
 *   \brief {DevName} device software implementation
 *   \author Oleh Sharuda
 */

#include "{devname}.hpp"
#include "ekit_firmware.hpp"

#ifdef {DEVNAME}_DEVICE_ENABLED

/// \addtogroup group_{devname}
/// @{

/// \brief Global array that stores all virtual {DevName}Dev devices configurations.
const {DevName}Instance g_{devname}_descriptors[] = {DEVNAME}_SW_DEV_DESCRIPTOR;
/// @}

const {DevName}Instance* {DevName}Dev::get_descriptor(size_t index) {
	if (index<{DEVNAME}_DEVICE_COUNT) {
		return const_cast<{DevName}Instance*>(g_{devname}_descriptors + index);
	} else {
		assert(false);
	}

	return nullptr;
}

std::string {DevName}Dev::get_dev_name() const {
	return descr->dev_name;
}

{DevName}Dev::{DevName}Dev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
	static const char* const func_name = "{DevName}Dev::{DevName}Dev";
	for (int i=0; i<{DEVNAME}_DEVICE_COUNT; i++) {
		if (addr==g_{devname}_descriptors[i].dev_id) {
			descr = g_{devname}_descriptors + i;
			break;
		}
	}

	if (descr==nullptr) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "addr specified doesn't correspond to any of {DevName} devices");
	}
}

{DevName}Dev::~{DevName}Dev() {
}

void {DevName}Dev::do_something(){
	static const char* const func_name = "{DevName}Dev::do_something";
	throw EKitException(func_name, EKIT_NOT_SUPPORTED);
}

#endif