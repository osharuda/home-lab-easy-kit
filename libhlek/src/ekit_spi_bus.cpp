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

#include "ekit_spi_bus.hpp"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ekit_helper.hpp"
#include "tools.hpp"

//------------------------------------------------------------------------------------
// EKitSPIBus::EKitSPIBus
// Purpose: EKitSPIBus class constructor
//------------------------------------------------------------------------------------
EKitSPIBus::EKitSPIBus(const std::string& file_name) :
     super(EKitBusType::BUS_SPI),
      bus_name(file_name) {
    spi_descriptor = 0;
    state = BUS_CLOSED;
    mode = 0;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::EKitSPIBus
// Purpose: EKitSPIBus class destructor
//------------------------------------------------------------------------------------
EKitSPIBus::~EKitSPIBus() {
    EKitTimeout to(0);
    BusLocker blocker(this, to);
    close();
}

//------------------------------------------------------------------------------------
// EKitSPIBus::open_internal
// Purpose: opens bus handle internally. Private, for internal use only
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::open_internal(EKitTimeout& to) {
    EKIT_ERROR res = EKIT_FAIL;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    if (state == BUS_OPENED) {
        res = EKIT_ALREADY_CONNECTED;
        goto done;
    }

    if ((spi_descriptor = ::open(bus_name.c_str(), O_RDWR)) < 0) {
        res = ERRNO_TO_EKIT_ERROR(errno);
        goto done;
    }

    res = spi_update_mode(to);
    if (res != EKIT_OK) goto done;

    res = spi_update_word_size(to);
    if (res != EKIT_OK) goto done;

    res = spi_update_frequency(to);
    if (res != EKIT_OK) goto done;

done:
    return res;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::open
// Purpose: opens bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::open(EKitTimeout& to) {
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
// EKitSPIBus::close
// Purpose: closes bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::close() {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (state == BUS_CLOSED) return EKIT_DISCONNECTED;

    if (state == BUS_OPENED) {
        ::close(spi_descriptor);
    }

    state = BUS_CLOSED;

    return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::read
// Purpose: Bus read operation
// uint8_t addr: bus address
// void* ptr: buffer to read data into. Buffer should be resized to appropriate
// amount of bytes to read Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::read(void* ptr, size_t len, EKitTimeout& to) {
    EKIT_ERROR res = EKIT_OK;

    // This asert should fail if there is an attempt to use read/write
    // without locking bus first
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (len + miso_read_offset > miso_data_size) {
        res = EKIT_OUT_OF_RANGE;
        goto done;
    }

    memcpy(ptr, miso_data.data() + miso_read_offset, len);

done:
    return res;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::read_all
// Purpose: Bus read operation
// uint8_t addr: bus address
// std::vector<uint8_t>& buffer: buffer to read data into. Buffer should be
// resized to appropriate amount of bytes to read Returns: corresponding
// EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::read_all(std::vector<uint8_t>& buffer, EKitTimeout& to) {
    buffer.resize(miso_data_size);
    return read(buffer.data(), miso_data_size - miso_read_offset, to);
}

//------------------------------------------------------------------------------------
// EKitSPIBus::write
// Purpose: Bus write operation
// uint8_t addr: bus address
// const void* ptr: buffer to write data from. Function writes all bytes
// allocated in buffer Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::write(const void* ptr, size_t len, EKitTimeout& to) {

    miso_data.resize(len);
    EKIT_ERROR err = write_read(reinterpret_cast<const uint8_t*>(ptr),
                                len,
                                miso_data.data(),
                                len,
                                to);
    if (err == EKIT_OK) {
        miso_data_size = len;
        miso_read_offset = 0;
    }

    return err;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::suspend
// Purpose: Suspend bus usage in the case bus is required to other processes
// Returns: corresponding EKIT_ERROR code
// Note: This is done by closing bus and reopening it later by resume() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::suspend(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    if (state == BUS_CLOSED) return EKIT_DISCONNECTED;

    if (state == BUS_PAUSED) return EKIT_SUSPENDED;

    ::close(spi_descriptor);
    state = BUS_PAUSED;

    return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitSPIBus::resume
// Purpose: Resumes bus usage after suspend() call
// Returns: corresponding EKIT_ERROR code
// Note: This is done by reopening it after suspend() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitSPIBus::resume(EKitTimeout& to) {
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

EKIT_ERROR EKitSPIBus::spi_update_mode(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EkitHelper::ioctl_request(spi_descriptor, SPI_IOC_WR_MODE32, &mode);
}

EKIT_ERROR EKitSPIBus::spi_update_frequency(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EkitHelper::ioctl_request(
        spi_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &frequency);
}

EKIT_ERROR EKitSPIBus::spi_update_word_size(EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EkitHelper::ioctl_request(
        spi_descriptor, SPI_IOC_WR_BITS_PER_WORD, &word_size);
}

EKIT_ERROR EKitSPIBus::set_opt(int opt, int value, EKitTimeout& to) {
    EKIT_ERROR res = EKIT_NOT_SUPPORTED;

    uint32_t non_zero_mask = 0 - (value != 0);

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    switch (opt) {
        case SPI_OPT_CLOCK_PHASE:
            mode = (mode & (~SPI_CPHA)) | (non_zero_mask & SPI_CPHA);
            res = spi_update_mode(to);
            break;

        case SPI_OPT_CLOCK_POLARITY:
            mode = (mode & (~SPI_CPOL)) | (non_zero_mask & SPI_CPOL);
            res = spi_update_mode(to);
            break;

        case SPI_OPT_CS_HIGH:
            mode = (mode & (~SPI_CS_HIGH)) | (non_zero_mask & SPI_CS_HIGH);
            res = spi_update_mode(to);
            break;

        case SPI_OPT_LSB_FIRST:
            mode = (mode & (~SPI_LSB_FIRST)) | (non_zero_mask & SPI_LSB_FIRST);
            res = spi_update_mode(to);
            break;

        case SPI_OPT_NO_CS:
            mode = (mode & (~SPI_NO_CS)) | (non_zero_mask & SPI_NO_CS);
            res = spi_update_mode(to);
            break;

        case SPI_OPT_CLOCK_FREQUENCY:
            if (value > 0) {
                frequency = static_cast<uint32_t>(value);
                res = spi_update_frequency(to);
            } else {
                res = EKIT_BAD_PARAM;
            }
            break;

        case SPI_OPT_WORD_SIZE:
            // Correct value must be multiple 8
            if ((value >= 0) && ((value & 7) == 0)) {
                word_size = value;
                res = spi_update_word_size(to);
            } else {
                res = EKIT_BAD_PARAM;
            }
            break;

        case SPI_OPT_CS_CHANGE:
            cs_change = value;
            res = EKIT_OK;
            break;
    }

    return res;
}

EKIT_ERROR EKitSPIBus::get_opt(int opt, int& value, EKitTimeout& to) {
    EKIT_ERROR res = EKIT_OK;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    switch (opt) {
        case SPI_OPT_CLOCK_PHASE:
            value = (mode & SPI_CPHA) != 0;
            break;

        case SPI_OPT_CLOCK_POLARITY:
            value = (mode & SPI_CPOL) != 0;
            break;

        case SPI_OPT_CS_HIGH:
            value = (mode & SPI_CS_HIGH) != 0;
            break;

        case SPI_OPT_LSB_FIRST:
            value = (mode & SPI_LSB_FIRST) != 0;
            break;

        case SPI_OPT_NO_CS:
            value = (mode & SPI_NO_CS) != 0;
            break;

        case SPI_OPT_CLOCK_FREQUENCY:
            value = static_cast<int>(frequency);
            break;

        case SPI_OPT_WORD_SIZE:
            value = static_cast<int>(word_size);
            break;

        case SPI_OPT_CS_CHANGE:
            value = static_cast<int>(cs_change);
            break;

        default:
            res = EKIT_NOT_SUPPORTED;
    }

    return res;
}

EKIT_ERROR EKitSPIBus::write_read(const uint8_t* wbuf,
                                 size_t wlen,
                                 uint8_t* rbuf,
                                 size_t rlen,
                                 EKitTimeout& to) {
    EKIT_ERROR err;
    int res;
    int ern;
    struct spi_ioc_transfer xfr;

    // Special handling for write only operation
    if ( ( rbuf == nullptr ) && ( rlen == 0 ) ) {
        rlen = wlen;
    }

    if (wlen != rlen) {
        err = EKIT_BAD_PARAM;
        goto done;
    }

    if (wlen == 0) {
        miso_read_offset = 0;
        miso_data_size = 0;
        err = EKIT_OK;  // Nothing to send, just success
        goto done;
    }

    memset(&xfr, 0, sizeof(xfr));

    // This statement checks if there were an attempt to use IO operation without locking bus first.
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    if (state == BUS_CLOSED) {
        err = EKIT_NOT_OPENED;
        goto done;
    }

    if (state == BUS_PAUSED) {
        err = EKIT_SUSPENDED;
        goto done;
    }

    // Fill msgs structure
    xfr.tx_buf = reinterpret_cast<std::uintptr_t>(wbuf);
    xfr.rx_buf = reinterpret_cast<std::uintptr_t>(rbuf);
    xfr.len = wlen;
    xfr.cs_change = cs_change;

    do {
        res = ioctl(spi_descriptor, SPI_IOC_MESSAGE(1), &xfr);
        ern = errno;
    } while (res < 0 && (ern == EINTR || ern == EAGAIN || ern == EWOULDBLOCK));

    if (res >= 1) {
        // just sent something
        assert(res == wlen);
        miso_read_offset = 0;
        miso_data_size = 0;
        err = EKIT_OK;
    } else {
        err = EKIT_FAIL;
    }

done:
    return err;
}
