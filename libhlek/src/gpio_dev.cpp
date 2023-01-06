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
 *   \brief GPIO device software implementation
 *   \author Oleh Sharuda
 */

#include "gpio_dev.hpp"
#include "gpio_common.hpp"

GPIODev::GPIODev(std::shared_ptr<EKitBus>& ebus, const GPIOConfig* cfg) :
    super(ebus, cfg->device_id, cfg->device_name),
    config(cfg),
    gpio_buffer_size((cfg->pin_number / 8) + 1) {
}

GPIODev::~GPIODev() {
}

size_t GPIODev::get_gpio_count() const {
    return config->pin_number;
}

const GPIOPin* GPIODev::get_gpio_info(size_t pin_index) const {
    static const char* const func_name = "GPIODev::get_gpio_info";
    if (pin_index>=config->pin_number) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "invalid pin index.");
    }

    return config->pins + pin_index;
}

void GPIODev::read(std::vector<bool>& pins) {
	static const char* const func_name = "GPIODev::read";
	uint8_t buffer[gpio_buffer_size];
    memset(buffer, 0, gpio_buffer_size);

    if (pins.size()!=config->pin_number) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Number of elements in input argument doesn't match to pin number.");
    }

	// I/O operation
	{
        EKitTimeout to(get_timeout());
		BusLocker blocker(bus, to);

		// Instruct controller to updated inputs
		EKIT_ERROR err = bus->write(nullptr, 0, to);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }		

		// Read bus
		err = bus->read(buffer, gpio_buffer_size, to);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "read() failed");
	    }
	}

	// fill pins
	for (size_t i=0; i<config->pin_number; i++) {
		bool bit_value = 1 & (buffer[i >> 3] >> (i & 0x07));
		pins[i] = bit_value;
	}
}
void GPIODev::write(const std::vector<bool>& pins) {
	static const char* const func_name = "GPIODev::write";

    if (pins.size()!=config->pin_number) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Number of elements in input argument doesn't match to pin number.");
    }

    uint8_t buffer[gpio_buffer_size];
    memset(buffer, 0, gpio_buffer_size);

	// Prepare buffer
	for (size_t i=0; i<config->pin_number; i++) {
		uint8_t bit_mask = static_cast<uint8_t>(pins[i]) << (i & 0x07);
		buffer[i >> 3] |= bit_mask;
	}

	// Block bus
    EKitTimeout to(get_timeout());
	BusLocker blocker(bus, to);

	// Write bus
	EKIT_ERROR err = bus->write(buffer, gpio_buffer_size, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }		
}