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
 *   \brief SPIProxy device software implementation
 *   \author Oleh Sharuda
 */

#include "spiproxy.hpp"
#include "ekit_firmware.hpp"
#include <cstring>

SPIProxyDev::SPIProxyDev(std::shared_ptr<EKitBus>& ebus, const struct SPIProxyConfig* cfg) :
    EKitBus(EKitBusType::BUS_SPI),
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
    static const char* const func_name = "SPIProxyDev::SPIProxyDev";

    ebus->check_bus(EKitBusType::BUS_I2C_FIRMWARE);

    // Preallocate receive buffer
    recv_buffer.assign(config->dev_buffer_len + sizeof(SPIProxyStatus), 0);
}

SPIProxyDev::~SPIProxyDev() {
}

EKIT_ERROR SPIProxyDev::lock(EKitTimeout& to) {
    bus_lock.lock();
    return std::dynamic_pointer_cast<EKitFirmware>(bus)->lock(get_addr(), to);
}

EKIT_ERROR SPIProxyDev::unlock() {
    EKIT_ERROR err = std::dynamic_pointer_cast<EKitFirmware>(bus)->unlock();
    bus_lock.unlock();
    return err;
}

EKIT_ERROR SPIProxyDev::write(const void* ptr, size_t len, EKitTimeout& to) {
    static const char* const func_name = "SPIProxyDev::write";

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    EKIT_ERROR err = spi_proxy_wait(to);
    if (err==EKIT_OK) {
        err = bus->write(ptr, len, to);
    }

    return err;
}

EKIT_ERROR SPIProxyDev::read(void* ptr, size_t len, EKitTimeout& to) {
    static const char* const func_name = "SPIProxyDev::read";

    static_assert(sizeof(SPIProxyStatus)==1, "SPIProxyStatus size must be 1 byte, check structure padding and alignment.");
    EKIT_ERROR res = EKIT_OK;
    std::vector<uint8_t> data;

    data.resize(sizeof(SPIProxyStatus));
    struct SPIProxyStatus* status = (struct SPIProxyStatus*)data.data();

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    res = spi_proxy_wait(to);
    if (res != EKIT_OK) {
        goto done;
    }
    // Read data, there should be enough bytes accumulated
    data.resize(len + sizeof(SPIProxyStatus));
    res = bus->read(data.data(), data.size(), to);
    if (res!=EKIT_OK) {
        goto done;
    }

    // Copy to buffer
    status = (struct SPIProxyStatus*)data.data();
    std::memcpy(ptr, data.data()+sizeof(SPIProxyStatus), len);

    if (status->rx_ovf) {
        res = EKIT_OVERFLOW;
        goto done;
    }

    done:
    return res;
}

EKIT_ERROR SPIProxyDev::read_all(std::vector<uint8_t>& buffer, EKitTimeout& to) {
    static const char* const func_name = "SPIProxyDev::read_all";

    static_assert(sizeof(SPIProxyStatus)==1, "SPIProxyStatus size must be 1 byte, check structure padding and alignment.");
    EKIT_ERROR res = EKIT_OK;
    std::vector<uint8_t> data;
    size_t data_len;

    data.resize(sizeof(struct SPIProxyStatus));
    struct SPIProxyStatus* status;

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    res = spi_proxy_wait(to);
    if (res != EKIT_OK) {
        goto done;
    }

    CommResponseHeader hdr;
    res = std::dynamic_pointer_cast<EKitFirmware>(bus)->get_status(hdr, false, to);
    if (res != EKIT_OK && res != EKIT_OVERFLOW ) {
        goto done;
    }

    // Read data, there should be enough bytes accumulated
    data_len = hdr.length - sizeof(SPIProxyStatus);
    assert(hdr.length >= sizeof(SPIProxyStatus));

    data.resize(hdr.length);
    res = bus->read(data.data(), hdr.length, to);
    if (res!=EKIT_OK) {
        goto done;
    }

    // Copy to buffer
    status = (struct SPIProxyStatus*)data.data();
    buffer.resize(data_len);
    std::memcpy(buffer.data(), data.data()+sizeof(SPIProxyStatus), data_len);

    if (status->rx_ovf) {
        res = EKIT_OVERFLOW;
        goto done;
    }

    done:
    return res;
}

EKIT_ERROR SPIProxyDev::write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to) {
  static const char* const func_name = "SPIProxyDev::write_read";
  assert(false); // MUST BE IMPLEMENTED
  return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR SPIProxyDev::get_opt(int opt, int& value, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR SPIProxyDev::set_opt(int opt, int v, EKitTimeout& to) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR SPIProxyDev::spi_proxy_wait(EKitTimeout& to) {
    static const char* const func_name = "SPIProxyDev::spi_proxy_wait";
    EKIT_ERROR err = EKIT_OK;
    size_t recv_buffer_size = config->dev_buffer_len + sizeof(SPIProxyStatus);
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    assert(recv_buffer.size()==recv_buffer_size);
    struct SPIProxyStatus* status = (struct SPIProxyStatus*)recv_buffer.data();

    do {
        err = bus->read((void *) status, sizeof(SPIProxyStatus), to);
    } while (err==EKIT_DEVICE_BUSY && status->running);

    return err;
}
