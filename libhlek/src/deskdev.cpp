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
 *   \brief Desk device implementation
 *   \author Oleh Sharuda
 */

#include <cstdint>
#include "deskdev.hpp"
#include "ekit_error.hpp"

DESKDev::DESKDev(std::shared_ptr<EKitBus>& ebus, const DeskConfig* config) :
    super(ebus, config->device_id, config->device_name)
{
}

DESKDev::~DESKDev() {
}

void DESKDev::get(bool& up, bool& down, bool& left, bool& right, int8_t& encoder) {
	static const char* const func_name = "DESKDev::get";
	EKIT_ERROR err;
	DeskDevData data;
    EKitTimeout to(get_timeout());
	BusLocker blocker(bus, to);

	err = bus->read(&data, sizeof(data), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }

    up = data.buttons[BUTTON_UP]!=0;
    down = data.buttons[BUTTON_DOWN]!=0;
    left = data.buttons[BUTTON_LEFT]!=0;
    right = data.buttons[BUTTON_RIGHT]!=0;
    encoder = data.encoder;
}