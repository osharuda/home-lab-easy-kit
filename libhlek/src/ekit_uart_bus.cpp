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
 *   \brief SPI bus implementation
 *   \author Oleh Sharuda
 */

#include "ekit_uart_bus.hpp"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ekit_helper.hpp"
#include "tools.hpp"

//------------------------------------------------------------------------------------
// EKitUARTBus::EKitUARTBus
// Purpose: EKitUARTBus class constructor
//------------------------------------------------------------------------------------
EKitUARTBus::EKitUARTBus(const std::string& file_name)
    : super(EKitBusType::BUS_UART),
      bus_name(file_name) {
    uart_descriptor = 0;
    state = BUS_CLOSED;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::EKitUARTBus
// Purpose: EKitUARTBus class destructor
//------------------------------------------------------------------------------------
EKitUARTBus::~EKitUARTBus() {
    EKitTimeout to(0);
    BusLocker blocker(this, to);
    close();
}

//------------------------------------------------------------------------------------
// EKitUARTBus::open_internal
// Purpose: opens bus handle internally. Private, for internal use only
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::open_internal(EKitTimeout& to) {
    EKIT_ERROR res = EKIT_FAIL;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (state == BUS_OPENED) {
        res = EKIT_ALREADY_CONNECTED;
        goto done;
    }

    if ((uart_descriptor = ::open(bus_name.c_str(), O_RDWR)) < 0) {
        res = ERRNO_TO_EKIT_ERROR(errno);
        goto done;
    }

done:
    return res;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::open
// Purpose: opens bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::open(EKitTimeout& to) {
    EKIT_ERROR err;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (state != BUS_CLOSED) {
        return EKIT_ALREADY_CONNECTED;
    }
    err = open_internal(to);

    if (err == EKIT_OK) state = BUS_OPENED;

    return err;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::close
// Purpose: closes bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::close() {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (state == BUS_CLOSED) return EKIT_DISCONNECTED;

    if (state == BUS_OPENED) {
        ::close(uart_descriptor);
    }

    state = BUS_CLOSED;

    return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::read
// Purpose: Bus read operation
// uint8_t addr: bus address
// void* ptr: buffer to read data into. Buffer should be resized to appropriate
// amount of bytes to read Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::read(void* ptr, size_t len, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::read_all
// Purpose: Bus read operation
// uint8_t addr: bus address
// std::vector<uint8_t>& buffer: buffer to read data into. Buffer should be
// resized to appropriate amount of bytes to read Returns: corresponding
// EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::read_all(std::vector<uint8_t>& buffer,
                                 EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::write
// Purpose: Bus write operation
// uint8_t addr: bus address
// const void* ptr: buffer to write data from. Function writes all bytes
// allocated in buffer Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::write(const void* ptr, size_t len, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitUARTBus::write_read(const uint8_t* wbuf,
                                  size_t wlen,
                                  uint8_t* rbuf,
                                  size_t rlen,
                                  EKitTimeout& to) {
    static const char* const func_name = "EKitUARTBus::write_read";
    return EKIT_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::suspend
// Purpose: Suspend bus usage in the case bus is required to other processes
// Returns: corresponding EKIT_ERROR code
// Note: This is done by closing bus and reopening it later by resume() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::suspend(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    if (state == BUS_CLOSED) return EKIT_DISCONNECTED;

    if (state == BUS_PAUSED) return EKIT_SUSPENDED;

    ::close(uart_descriptor);
    state = BUS_PAUSED;

    return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitUARTBus::resume
// Purpose: Resumes bus usage after suspend() call
// Returns: corresponding EKIT_ERROR code
// Note: This is done by reopening it after suspend() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitUARTBus::resume(EKitTimeout& to) {
    EKIT_ERROR err = EKIT_OK;

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    if (state == BUS_CLOSED) return EKIT_DISCONNECTED;

    if (state == BUS_OPENED) return EKIT_SUSPENDED;

    err = open_internal(to);
    if (err == EKIT_OK) {
        state = BUS_OPENED;
    }

    return err;
}

EKIT_ERROR EKitUARTBus::set_opt(int opt, int value, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitUARTBus::get_opt(int opt, int& value, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}
