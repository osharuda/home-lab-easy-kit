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
     \brief Support for hlekio kernel module.
 *   \author Oleh Sharuda
 */

#include "hlekio.hpp"
#include "hlekio_ioctl.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

HLEKIOBase::HLEKIOBase(const std::string& dev_name, HLEKIODevType dev_type) {
    // Open device
    try {
        fd = open(dev_name.c_str(), O_CLOEXEC | O_RDWR);
        if (fd < 0) {
            throw EKitException(__FUNCTION__, errno, "Failed to open device for blocking device");
        }

        // If required open in non-block mode.
        // Note: it is used for inputs only, thus open it for read
        if (dev_type == HLEKIODevType::Input) {
            nb_fd = open(dev_name.c_str(), O_CLOEXEC | O_NONBLOCK);
            if (fd < 0) {
                throw EKitException(__FUNCTION__, errno, "Failed to open device for non-blocking device");
            }
        }

        if (dev_type != get_type()) {
            throw EKitException(__FUNCTION__, EKIT_WRONG_DEVICE, "Wrong device type");
        }

        set_binary();
        reset();
    } catch (const std::exception&) {
        close_all();
        throw;
    }
}
HLEKIOBase::~HLEKIOBase() {
    close_all();
}

void HLEKIOBase::close_all() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    if (nb_fd != -1) {
        close(nb_fd);
        nb_fd = -1;
    }
}

void HLEKIOBase::reset() {
    unsigned long mode = 1;
    int res = ioctl(fd, HLEKIO_RESET);
    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to reset device");
    }
}

void HLEKIOBase::set_binary() {
    unsigned long mode = 1;
    int res = ioctl(fd, HLEKIO_BINARY_MODE, mode);
    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to set binary mode");
    }
}

void HLEKIOBase::reset_fp(int f) {
    int res = lseek(f, SEEK_SET, 0);
    if (res != 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to lseek file descriptor");
    }
}

HLEKIODevType HLEKIOBase::get_type() {
    uint8_t type;
    int res = ioctl(fd, HLEKIO_PIN_TYPE, &type);

    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to get device type");
    }

    return static_cast<HLEKIODevType>(type);
}

HLEKIOInput::HLEKIOInput(const std::string dev_name) :
    HLEKIOBase(dev_name, HLEKIODevType::Input)
{}

HLEKIOInput::~HLEKIOInput(){}

uint8_t HLEKIOInput::get(hlekio_input_info* info) {
    hlekio_input_info local;
    hlekio_input_info* data = (info==nullptr) ? &local : info;
    int res = read(nb_fd, data, sizeof(hlekio_input_info));
    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to read from device (non-blocking mode)");
    }

    reset_fp(nb_fd);

    return data->level;
}

int HLEKIOInput::wait(EKitTimeout& to, hlekio_input_info* info) {
    hlekio_input_info local;
    hlekio_input_info* data = (info==nullptr) ? &local : info;
    int res = read(fd, data, sizeof(hlekio_input_info));
    int err = errno;

    if (err==EINTR) {
        return err;
    }
    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to read from device (blocking mode)");
    }

    reset_fp(fd);

    return 0;
}

void HLEKIOInput::set_debounce(unsigned long d) {
    int res = ioctl(fd, HLEKIO_DEBOUNCE, d);
    if (res < 0) {
        throw EKitException(__FUNCTION__ , errno, "Failed to set debounce");
    }
}

HLEKIOOutput::HLEKIOOutput(const std::string dev_name) :
    HLEKIOBase(dev_name, HLEKIODevType::Output)
{}

HLEKIOOutput::~HLEKIOOutput(){}

uint8_t HLEKIOOutput::get() {
    uint8_t value;
    int err, res;

    do {
        res = read(fd, &value, sizeof(value));
        err = errno;
    } while (err == EINTR);

    if (res != sizeof(value)) {
        throw EKitException(__FUNCTION__ , err, "Failed to read from device (blocking mode)");
    }

    reset_fp(fd);

    return value;
}

void HLEKIOOutput::set(uint8_t v) {
    int res = write(fd, &v, sizeof(v));
    if (res < sizeof(v)) {
        throw EKitException(__FUNCTION__ , errno, "Failed to set output pin value");
    }

    reset_fp(fd);
}