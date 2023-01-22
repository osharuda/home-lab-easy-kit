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
#include "lcd1602a.hpp"
#include "ekit_firmware.hpp"
#include "ekit_error.hpp"

LCD1602ADev::LCD1602ADev(std::shared_ptr<EKitBus>& ebus, const LCD1602aConfig* config) : super(ebus, config->device_id, config->device_name),
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
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, lmode, to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }	    

    err = bus->write(nullptr, 0, to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }

    light_mode = lmode;
}

void LCD1602ADev::write(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last) {
	static const char* const func_name = "LCD1602ADev::write(1)";
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

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

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, light_mode, to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer.data(), buffer.size(), to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }	
}

void LCD1602ADev::write(const std::vector<std::string>& lines) {
	write(lines.cbegin(), lines.cend());
}

void LCD1602ADev::writepos(uint8_t line, uint8_t pos, const std::string& s) {
	static const char* const func_name = "LCD1602ADev::writepos";

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);
	std::vector<uint8_t> buffer;
	size_t buflen = s.length() + sizeof(LcdPositionalText);
	buffer.resize(buflen);
	PLcdPositionalText hdr = (PLcdPositionalText)buffer.data();
	hdr->line = line;
	hdr->position = pos;
	strcpy((char*)hdr->text, s.c_str());

	EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, light_mode | LCD1602a_POSITION, to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer.data(), buflen, to);
    if (err != EKIT_OK) {
    	throw EKitException(func_name, err, "write() failed");
    }
}

void LCD1602ADev::clear() {
	write(empty_screen);
}
