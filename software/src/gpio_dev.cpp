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

#ifdef GPIODEV_DEVICE_ENABLED

constexpr GPIO_descr GPIODev::gpio_pin_descriptors[];

constexpr size_t GPIODev::gpio_buffer_size = ((GPIO_PIN_COUNT / 8) + 1);

GPIODev::GPIODev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
}

GPIODev::~GPIODev() {
}

size_t GPIODev::get_gpio_count() const {
    return GPIO_PIN_COUNT;
}

const GPIO_descr* GPIODev::get_gpio_info(size_t pin_index) const {
    static const char* const func_name = "GPIODev::get_gpio_info";
    if (pin_index>=GPIO_PIN_COUNT) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "invalid pin index.");
    }

    return gpio_pin_descriptors + pin_index;
}

void GPIODev::read(std::bitset<GPIO_PIN_COUNT>& pins) {
	static const char* const func_name = "GPIODev::read";
	uint8_t buffer[gpio_buffer_size] = {0};

	// Block bus
	{
		BusLocker blocker(bus, get_addr());	

		// Instruct controller to updated inputs
		EKIT_ERROR err = bus->write(nullptr, 0);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }		

		// Read bus
		err = bus->read(buffer, gpio_buffer_size);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "read() failed");
	    }
	}

	// fill pins
	for (size_t i=0; i<GPIO_PIN_COUNT; i++) {
		uint8_t bit_value = 1 & (buffer[i >> 3] >> (i & 0x07));
		pins.set(i, bit_value);
	}
}
void GPIODev::write(const std::bitset<GPIO_PIN_COUNT>& pins) {
	static const char* const func_name = "GPIODev::write";
	uint8_t buffer[gpio_buffer_size] = {0};

	// Prepare buffer
	for (size_t i=0; i<GPIO_PIN_COUNT; i++) {
		uint8_t bit_mask = pins.test(i) << (i & 0x07);
		buffer[i >> 3] |= bit_mask;
	}

	// Block bus
	BusLocker blocker(bus, get_addr());	

	// Write bus
	EKIT_ERROR err = bus->write(buffer, gpio_buffer_size);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }		
}

std::string GPIODev::get_dev_name() const {
	return GPIO_DEVICE_NAME;
}

#endif