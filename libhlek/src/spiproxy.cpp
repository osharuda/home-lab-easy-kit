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

SPIProxyDev::SPIProxyDev(std::shared_ptr<EKitBus>& ebus, const SPIProxyConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
	static const char* const func_name = "SPIProxyDev::SPIProxyDev";

    int busid;
    ebus->bus_props(busid);
    if (busid != BUS_I2C_FIRMWARE) {
        assert(false); // Wrong bus is used. SPIProxy may work with firmware only!
    }

    // Preallocate receive buffer
    recv_buffer.assign(config->dev_buffer_len + sizeof(SPIProxyStatus), 0);
}

SPIProxyDev::~SPIProxyDev() {
}

EKIT_ERROR SPIProxyDev::lock() {
    return std::dynamic_pointer_cast<EKitFirmware>(bus)->lock(get_addr());
}

EKIT_ERROR SPIProxyDev::write(const void* ptr, size_t len) {
    static const char* const func_name = "SPIProxyDev::write";
    EKIT_ERROR err = EKIT_OK;

    // Lock bus
    tools::StopWatch<std::chrono::milliseconds> sw(timeout);
    BusLocker blocker(bus);

    err = spi_proxy_wait(sw);
    if (err!=EKIT_OK) {
        goto done;
    }

    err = bus->write(ptr, len);
    if (err==EKIT_OK && sw.expired()) {
        err = EKIT_TIMEOUT;
    }

done:
    return err;
}

EKIT_ERROR SPIProxyDev::read(void* ptr, size_t len) { // <CHECKIT>: Consider to use this kind of style with timeout and polling data size for other buses
    static const char* const func_name = "SPIProxyDev::read";

    tools::StopWatch<std::chrono::milliseconds> sw(timeout);
    static_assert(sizeof(SPIProxyStatus)==1);
    EKIT_ERROR res = EKIT_OK;
    std::vector<uint8_t> data;
    bool expired = false;

    data.resize(sizeof(SPIProxyStatus));
    PSPIProxyStatus status = (PSPIProxyStatus)data.data();

    // Lock bus
    BusLocker blocker(bus);

    res = spi_proxy_wait(sw);
    if (res != EKIT_OK) {
        goto done;
    }
    // Read data, there should be enough bytes accumulated
    data.resize(len + sizeof(SPIProxyStatus));
    res = bus->read(data);
    if (res!=EKIT_OK) {
        goto done;
    }

    // Copy to buffer
    status = (PSPIProxyStatus)data.data();
    std::memcpy(ptr, data.data()+sizeof(SPIProxyStatus), len);
    expired = sw.expired();

    if (expired) {
        res = EKIT_TIMEOUT;
        goto done;
    }

    if (status->rx_ovf) {
        res = EKIT_OVERFLOW;
        goto done;
    }

    done:
    return res;
}

EKIT_ERROR SPIProxyDev::read_all(std::vector<uint8_t>& buffer) {
    static const char* const func_name = "SPIProxyDev::read_all";

    tools::StopWatch<std::chrono::milliseconds> sw(timeout);
    static_assert(sizeof(SPIProxyStatus)==1);
    EKIT_ERROR res = EKIT_OK;
    std::vector<uint8_t> data;
    bool expired = false;
    size_t data_len;

    data.resize(sizeof(SPIProxyStatus));
    PSPIProxyStatus status;// = (PSPIProxyStatus)data.data();

    // Lock bus
    BusLocker blocker(bus);

    res = spi_proxy_wait(sw);
    if (res != EKIT_OK) {
        goto done;
    }

    CommResponseHeader hdr;
    res = std::dynamic_pointer_cast<EKitFirmware>(bus)->get_status(hdr, false);
    if (res != EKIT_OK && res != EKIT_OVERFLOW ) {
        goto done;
    }

    // Read data, there should be enough bytes accumulated
    data_len = hdr.length - sizeof(SPIProxyStatus);
    assert(hdr.length >= sizeof(SPIProxyStatus));

    data.resize(hdr.length);
    res = bus->read(data);
    if (res!=EKIT_OK) {
        goto done;
    }

    // Copy to buffer
    status = (PSPIProxyStatus)data.data();
    buffer.resize(data_len);
    std::memcpy(buffer.data(), data.data()+sizeof(SPIProxyStatus), data_len);
    expired = sw.expired();

    if (expired) {
        res = EKIT_TIMEOUT;
        goto done;
    }

    if (status->rx_ovf) {
        res = EKIT_OVERFLOW;
        goto done;
    }

    done:
    return res;
}

int SPIProxyDev::bus_props(int& busid) const {
    busid = BUS_SPI;
    return BUS_PROP_READALL;
}

EKIT_ERROR SPIProxyDev::get_opt(int opt, int& value) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    BusLocker bl(bus);
    return EKIT_OK;
}

EKIT_ERROR SPIProxyDev::set_opt(int opt, int v) {
    EKIT_ERROR err = EKIT_OK;
    int value = 0;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    // For virtual device, redirect options to underlying bus (firmware)
    BusLocker bl(bus);

    bus->get_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, value);

    switch (opt) {
        case EKitBusOptions::EKITBUS_TIMEOUT:
            timeout = v;    // <CHECKIT> Also, need to set timeout for underlying bus
        break;

        default:
            return EKIT_BAD_PARAM;
    }
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, value);
    return err;
}

EKIT_ERROR SPIProxyDev::spi_proxy_wait(tools::StopWatch<std::chrono::milliseconds>& sw) {
    static const char* const func_name = "SPIProxyDev::spi_proxy_wait";
    EKIT_ERROR err = EKIT_OK;
    size_t recv_buffer_size = config->dev_buffer_len + sizeof(SPIProxyStatus);
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);
    assert(recv_buffer.size()==recv_buffer_size);
    PSPIProxyStatus status = (PSPIProxyStatus)recv_buffer.data();
    bool expired = false;

    do {
        err = bus->read((void *) status, sizeof(SPIProxyStatus));
        expired = sw.expired();
    } while (err==EKIT_DEVICE_BUSY && status->running && (!expired));

    if (expired) {
        err = EKIT_TIMEOUT;
    }
    return err;
}
