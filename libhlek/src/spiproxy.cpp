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

    // Preallocate receive buffer
    recv_buffer.assign(config->dev_buffer_len + sizeof(SPIProxyStatus), 0);
}

SPIProxyDev::~SPIProxyDev() {
}

EKIT_ERROR SPIProxyDev::write(const void* ptr, size_t len) {
    static const char* const func_name = "SPIProxyDev::write";
    EKIT_ERROR err = EKIT_OK;

    // Lock bus
    tools::StopWatch<std::chrono::milliseconds> sw(timeout);
    BusLocker blocker(bus, get_addr());

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
    BusLocker blocker(bus, get_addr());

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
    return EKIT_NOT_SUPPORTED;
}

int SPIProxyDev::bus_props(int& busid) const {
    busid = BUS_SPI;
    return 0;
}

EKIT_ERROR SPIProxyDev::get_opt(int opt, int& value) {
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    BusLocker bl(bus, get_addr());
    return EKIT_OK;
}

EKIT_ERROR SPIProxyDev::set_opt(int opt, int v) {
    EKIT_ERROR err = EKIT_OK;
    int value = 0;
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    // For virtual device, redirect options to underlying bus (firmware)
    BusLocker bl(bus, get_addr());

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
