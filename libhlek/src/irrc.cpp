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
 *   \brief IRRCDev software implementation
 *   \author Oleh Sharuda
 */

#include "irrc.hpp"
#include "i2c_proto.h"
#include "ekit_error.hpp"
#include "ekit_firmware.hpp"

IRRCDev::IRRCDev(std::shared_ptr<EKitBus>& ebus, const IRRCConfig* config) : super(ebus, config->device_id, config->device_name) {
}

IRRCDev::~IRRCDev() {
}

void IRRCDev::get(std::vector<IR_NEC_Command>& commands, bool& ovf) {
	static const char* const func_name = "IRRCDev::get";
	EKIT_ERROR err;
	std::vector<uint8_t> data;
	size_t data_len;
	size_t cmd_len;

	commands.clear();
	BusLocker blocker(bus);

	// get amount of data
	CommResponseHeader hdr;
	err = std::dynamic_pointer_cast<EKitFirmware>(bus)->get_status(hdr, false);
    if (err != EKIT_OK && err != EKIT_OVERFLOW ) {
        throw EKitException(func_name, err, "get_status() failed");
    }

    ovf = (hdr.comm_status & COMM_STATUS_OVF) != 0;
    data_len = (hdr.length >> 1) << 1;	// limit to be even
    cmd_len = data_len/2;
    assert((data_len & 1) == 0);

	// read event amount of bytes

	if (data_len!=0) {
		data.resize(data_len);
		err = bus->read(data);

		if (err != EKIT_OK) {
		    throw EKitException(func_name, err, "read() failed");
		}

		commands.resize(cmd_len);
		for (size_t i=0; i<cmd_len; i++) {
			commands[i].address = data[i*2];
			commands[i].command = data[i*2+1];
		}
	}
}