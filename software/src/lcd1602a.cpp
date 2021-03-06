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
 *   \brief LCD1602ADev software implementation
 *   \author Oleh Sharuda
 */

#include <cstdint>
#include "sw.h"
#include "lcd1602a.hpp"
#include "ekit_firmware.hpp"
#include "ekit_error.hpp"

#ifdef LCD1602a_DEVICE_ENABLED

LCD1602ADev::LCD1602ADev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr),
                                                                     empty_screen(LCD1602a_POSITION_MAXLINE-LCD1602a_POSITION_MINLINE+1, std::string(LCD1602a_WIDTH, ' '))
{
}

LCD1602ADev::~LCD1602ADev() {
}

int LCD1602ADev::nlines() const {
	return LCD1602a_POSITION_MAXLINE-LCD1602a_POSITION_MINLINE+1;
}

int LCD1602ADev::nchars() const {
	return LCD1602a_WIDTH;
}

void LCD1602ADev::light(int lmode) {
	static const char* const func_name = "LCD1602ADev::light";
	assert(lmode==LCD1602a_LIGHT || lmode==LCD1602a_OFF || lmode==LCD1602a_BLINK);
	BusLocker blocker(bus, get_addr());

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, lmode);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }	    

    err = bus->write({});
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }

    light_mode = lmode;
}

void LCD1602ADev::write(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last) {
	static const char* const func_name = "LCD1602ADev::write(1)";
	BusLocker blocker(bus, get_addr());

	std::vector<uint8_t> buffer;
	int nl = nlines();

	for (std::vector<std::string>::const_iterator i = first; i!=last; ++i, --nl) {
		if (nl<=0) {
            throw EKitException(func_name, EKIT_OUT_OF_RANGE, "too many lines");
		}
		std::string l = *i;
		l.resize(LCD1602a_WIDTH, ' ');
		std::copy(l.begin(), l.end(), std::back_inserter(buffer));
	}

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, light_mode);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }	
}

void LCD1602ADev::write(const std::vector<std::string>& lines) {
	write(lines.cbegin(), lines.cend());
}

void LCD1602ADev::writepos(uint8_t line, uint8_t pos, const std::string& s) {
	static const char* const func_name = "LCD1602ADev::writepos";

	BusLocker blocker(bus, get_addr());
	std::vector<uint8_t> buffer;
	size_t buflen = s.length() + sizeof(LcdPositionalText);
	buffer.resize(buflen);
	PLcdPositionalText hdr = (PLcdPositionalText)buffer.data();
	hdr->line = line;
	hdr->position = pos;
	strcpy((char*)hdr->text, s.c_str());

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, light_mode | LCD1602a_POSITION);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }
}

void LCD1602ADev::clear() {
	write(empty_screen);
}

std::string LCD1602ADev::get_dev_name() const {
    return LCD1602a_DEVICE_NAME;
}

#endif