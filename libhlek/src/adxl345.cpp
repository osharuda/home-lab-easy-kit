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
 *   \brief ADXL345 3-axis accelerometer support.
 *   \author Oleh Sharuda
 */

#include "adxl345.hpp"

#include <math.h>

#include "ekit_bus.hpp"
#include "ekit_error.hpp"

ADXL345::ADXL345(std::shared_ptr<EKitBus>& ebus,
                 int                       timeout_ms,
                 const char*               name)
    : super(ebus, name) {
    static const char* const func_name = "ADXL345::ADXL345";

    ebus->check_bus(EKitBusType::BUS_SPI);

    EKitTimeout to(get_timeout());
    BusLocker   blocker(ebus, to);
}

ADXL345::~ADXL345() {}

EKIT_ERROR ADXL345::read_config_priv(ADXL345Confiuration& config,
                                     EKitTimeout&         to) {
    EKIT_ERROR res = get_power_ctl_priv(config.link,
                                        config.auto_sleep,
                                        config.measure,
                                        config.sleep,
                                        config.wakeup_rate,
                                        to);
    if (res != EKIT_OK) goto done;

    res = get_rate_priv(config.low_power, config.rate, to);
    if (res != EKIT_OK) goto done;

    res = get_data_format_priv(config.self_test,
                               config.three_wire_spi,
                               config.int_invert,
                               config.full_res,
                               config.justify_msb,
                               config.range,
                               to);
    if (res != EKIT_OK) goto done;

    res = get_fifo_ctl_priv(
        config.fifo_len, config.fifo_mode, config.trigger_int, to);
    if (res != EKIT_OK) goto done;

done:
    return res;
}

EKIT_ERROR ADXL345::write_config_priv(ADXL345Confiuration& config,
                                      EKitTimeout&         to) {
    EKIT_ERROR res = set_power_ctl_priv(config.link,
                                        config.auto_sleep,
                                        config.measure,
                                        config.sleep,
                                        config.wakeup_rate,
                                        to);
    if (res != EKIT_OK) goto done;

    res = set_rate_priv(config.low_power, config.rate, to);
    if (res != EKIT_OK) goto done;

    res = set_data_format_priv(config.self_test,
                               config.three_wire_spi,
                               config.int_invert,
                               config.full_res,
                               config.justify_msb,
                               config.range,
                               to);
    if (res != EKIT_OK) goto done;

    res = set_fifo_ctl_priv(
        config.fifo_len, config.fifo_mode, config.trigger_int, to);
    if (res != EKIT_OK) goto done;

done:
    return res;
}

void ADXL345::get_data(ADXL345Sample* data) {
    static const char* const func_name = "ADXL345::get_data_len";
    constexpr size_t         data_len  = sizeof(ADXL345Data) + 1;
    int                      err;

    EKitTimeout              to(get_timeout());
    BusLocker                blocker(bus, to);

    // Buffer points one byte ahead ADXL345Data structure. This is possible
    // because preceding member (timestamp) will be filled in later.
    uint8_t* buffer = reinterpret_cast<uint8_t*>(&(data->data)) - 1;
    buffer[0]       = READ_REG_FLAG | MULTYBYTE_FLAG | ADXL345Registers::DATAX0;

    EKIT_ERROR res  = bus->write_read(buffer, data_len, buffer, data_len, to);

    if (res != EKIT_OK) {
        throw EKitException(func_name, res, "SPI transaction failed.");
    }

    err = clock_gettime(CLOCK_MONOTONIC_RAW, &(data->timestamp));
    if (err < 0) {
        res = ERRNO_TO_EKIT_ERROR(errno);
        throw EKitException(func_name, res, "Failed to obtain timestamp.");
    }

    // Delay of 5us is required by ADXL345 datasheet.
    std::this_thread::sleep_for(std::chrono::microseconds(10));
}

EKIT_ERROR ADXL345::get_fifo_ctl_priv(size_t&      fifolen,
                                      uint8_t&     mode,
                                      bool&        trigger,
                                      EKitTimeout& to) {
    uint8_t    fifo_ctl = 0;

    EKIT_ERROR err =
        single_byte_transaction(true, ADXL345Registers::FIFO_CTL, fifo_ctl, to);
    if (err != EKIT_OK) goto done;

    fifolen = fifo_ctl & ADXL345Constants::FIFO_CTL_SAMPLES_MASK;
    trigger = static_cast<bool>(fifo_ctl & ADXL345Constants::FIFO_CTL_TRIGGER);
    mode    = fifo_ctl & ADXL345Constants::FIFO_CTL_MODE;
done:
    return err;
}

EKIT_ERROR ADXL345::set_fifo_ctl_priv(const size_t  fifolen,
                                      const uint8_t fifo_mode,
                                      const bool    trigger_int,
                                      EKitTimeout&  to) {
    assert((fifolen & ADXL345Constants::FIFO_CTL_SAMPLES_MASK) == fifolen);
    assert((fifo_mode & ADXL345Constants::FIFO_CTL_MODE) == fifo_mode);

    uint8_t fifo_ctl = (trigger_int ? ADXL345Constants::FIFO_CTL_TRIGGER : 0)
                       | (fifolen & ADXL345Constants::FIFO_CTL_SAMPLES_MASK)
                       | (fifo_mode & ADXL345Constants::FIFO_CTL_MODE);

    EKIT_ERROR err = single_byte_transaction(
        false, ADXL345Registers::FIFO_CTL, fifo_ctl, to);

    return err;
}

EKIT_ERROR ADXL345::single_byte_transaction(bool          read_data,
                                            const uint8_t addr,
                                            uint8_t&      data,
                                            EKitTimeout&  to) {
    constexpr size_t sb_trans_len = 2;
    uint8_t          io_buffer[sb_trans_len];
    assert((addr & REG_ADDR_MASK) == addr);
    io_buffer[0] = (read_data ? READ_REG_FLAG : 0) | (addr & REG_ADDR_MASK);
    io_buffer[1] = data;

    EKIT_ERROR err =
        bus->write_read(io_buffer, sb_trans_len, io_buffer, sb_trans_len, to);
    if (err == EKIT_OK) {
        data = io_buffer[1];
    }

    return err;
}

EKIT_ERROR ADXL345::set_data_format_priv(bool         self_test,
                                         bool         three_wire_spi,
                                         bool         int_invert,
                                         bool         full_res,
                                         bool         justify_msb,
                                         uint8_t      range,
                                         EKitTimeout& to) {
    assert((range & ADXL345Constants::DATA_FORMAT_RANGE_MASK) == range);

    uint8_t data_format =
        (self_test ? ADXL345Constants::DATA_FORMAT_SELF_TEST : 0)
        | (three_wire_spi ? ADXL345Constants::DATA_FORMAT_3WIRE_SPI : 0)
        | (int_invert ? ADXL345Constants::DATA_FORMAT_INT_INVERT : 0)
        | (full_res ? ADXL345Constants::DATA_FORMAT_FULL_RES : 0)
        | (justify_msb ? ADXL345Constants::DATA_FORMAT_MSB : 0)
        | (range & ADXL345Constants::DATA_FORMAT_RANGE_MASK);

    EKIT_ERROR err = single_byte_transaction(
        false, ADXL345Registers::DATA_FORMAT, data_format, to);
    return err;
}

EKIT_ERROR ADXL345::get_data_format_priv(bool&        self_test,
                                         bool&        three_wire_spi,
                                         bool&        int_invert,
                                         bool&        full_res,
                                         bool&        justify_msb,
                                         uint8_t&     range,
                                         EKitTimeout& to) {
    uint8_t    data_format = 0;
    EKIT_ERROR err         = single_byte_transaction(
        true, ADXL345Registers::DATA_FORMAT, data_format, to);
    if (err == EKIT_OK) {
        self_test      = data_format & DATA_FORMAT_SELF_TEST;
        three_wire_spi = data_format & DATA_FORMAT_3WIRE_SPI;
        int_invert     = data_format & DATA_FORMAT_INT_INVERT;
        full_res       = data_format & DATA_FORMAT_FULL_RES;
        justify_msb    = data_format & DATA_FORMAT_MSB;
        range          = data_format & DATA_FORMAT_RANGE_MASK;
    }

    return err;
}

EKIT_ERROR ADXL345::set_rate_priv(bool         low_power,
                                  uint8_t      rate,
                                  EKitTimeout& to) {
    assert((rate & BW_RATE_RATE_MASK) == rate);
    uint8_t rate_value =
        (low_power ? BW_RATE_LOW_POWER : 0) | (rate & BW_RATE_RATE_MASK);

    return single_byte_transaction(
        false, ADXL345Registers::BW_RATE, rate_value, to);
}

EKIT_ERROR ADXL345::get_rate_priv(bool&        low_power,
                                  uint8_t&     rate,
                                  EKitTimeout& to) {
    uint8_t    rate_value;
    EKIT_ERROR err = single_byte_transaction(
        true, ADXL345Registers::BW_RATE, rate_value, to);
    if (err == EKIT_OK) {
        low_power = rate_value & BW_RATE_LOW_POWER;
        rate      = rate_value & BW_RATE_RATE_MASK;
    }
    return err;
}

EKIT_ERROR ADXL345::set_power_ctl_priv(bool         link,
                                       bool         auto_sleep,
                                       bool         measure,
                                       bool         sleep,
                                       uint8_t      wakeup_rate,
                                       EKitTimeout& to) {
    assert((wakeup_rate & POWER_CTL_WAKEUP_MASK) == wakeup_rate);
    uint8_t power_value =
        (link ? POWER_CTL_LINK : 0) | (auto_sleep ? POWER_CTL_AUTO_SLEEP : 0)
        | (measure ? POWER_CTL_MEASURE : 0) | (sleep ? POWER_CTL_SLEEP : 0)
        | (wakeup_rate & POWER_CTL_WAKEUP_MASK);

    return single_byte_transaction(
        false, ADXL345Registers::POWER_CTL, power_value, to);
}

EKIT_ERROR ADXL345::get_power_ctl_priv(bool&        link,
                                       bool&        auto_sleep,
                                       bool&        measure,
                                       bool&        sleep,
                                       uint8_t&     wakeup_rate,
                                       EKitTimeout& to) {
    uint8_t    power_value;
    EKIT_ERROR err = single_byte_transaction(
        true, ADXL345Registers::POWER_CTL, power_value, to);
    if (err == EKIT_OK) {
        link        = power_value & POWER_CTL_LINK;
        auto_sleep  = power_value & POWER_CTL_AUTO_SLEEP;
        measure     = power_value & POWER_CTL_MEASURE;
        sleep       = power_value & POWER_CTL_SLEEP;
        wakeup_rate = power_value & POWER_CTL_WAKEUP_MASK;
    }
    return err;
}

EKIT_ERROR ADXL345::check_device_id_priv(EKitTimeout& to) {
    uint8_t                  devid     = 0;
    EKIT_ERROR               err =
        single_byte_transaction(true, ADXL345Registers::DEVID, devid, to);
    if ((err == EKIT_OK) && (devid != ADXL345Constants::DEVID_VALUE)) {
        err = EKIT_WRONG_DEVICE;
    }

    return err;
}

size_t ADXL345::get_data_len(bool& fifo_triggered) {
    static const char* const func_name = "ADXL345::get_data_len";

    uint8_t                  fifo_status;
    EKIT_ERROR               res;

    EKitTimeout              to(get_timeout());
    BusLocker                blocker(bus, to);

    res = single_byte_transaction(true, FIFO_STATUS, fifo_status, to);
    if (res != EKIT_OK) {
        throw EKitException(
            func_name, res, "Failed to read FIFO_STATUS register");
    }
    fifo_triggered = fifo_status & FIFO_STATUS_TRIGGER;

    return static_cast<size_t>(fifo_status & FIFO_STATUS_ENTRIES);
}

void ADXL345::enable(bool enabled) {
    static const char* const func_name = "ADXL345::start";
    EKitTimeout              to(get_timeout());
    BusLocker                blocker(bus, to);

    ADXL345Confiuration      config;
    EKIT_ERROR               res = read_config_priv(config, to);
    if (res != EKIT_OK) {
        throw EKitException(
            func_name, res, "Reading configuration has failed.");
    }

    if (config.measure == enabled) {
        throw EKitException(
            func_name,
            EKIT_FAIL,
            (enabled ? "Device already enabled" : "Device already disabled"));
    }

    config.measure = enabled;
    res            = write_config_priv(config, to);
    if (res != EKIT_OK) {
        throw EKitException(func_name, res, "Failed to update config");
    }
}

void ADXL345::configure(uint8_t rate,
                        uint8_t watermark_samples,
                        uint8_t range) {
    static const char* const func_name = "ADXL345::set_data";

    // Set maximum range
    res_scale = 1.0L / static_cast<double>(INT16_MAX);
    switch (range) {
        case ADXL345Constants::DATA_FORMAT_RANGE_2g:
            max_val   = grav_accel * 2.0L;
            break;

        case ADXL345Constants::DATA_FORMAT_RANGE_4g:
            max_val   = grav_accel * 4.0L;
            break;

        case ADXL345Constants::DATA_FORMAT_RANGE_8g:
            max_val   = grav_accel * 8.0L;
            break;

        case ADXL345Constants::DATA_FORMAT_RANGE_16g:
            max_val   = grav_accel * 16.0L;
            break;

        default:
            throw EKitException(
                func_name, EKIT_BAD_PARAM, "Invalid range value");
    }

    if ((rate & BW_RATE_RATE_MASK) != rate) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Invalid rate value");
    }

    if ((watermark_samples & FIFO_CTL_SAMPLES_MASK) != watermark_samples) {
        throw EKitException(
            func_name, EKIT_BAD_PARAM, "Invalid watermark_samples value");
    }

    EKitTimeout to(get_timeout());
    BusLocker   blocker(bus, to);

    EKIT_ERROR  res = check_device_id_priv(to);
    if (res != EKIT_OK) {
        throw EKitException(func_name, res, "Device ID mismatch.");
    }

    res = read_config_priv(adxl_config, to);
    if (res != EKIT_OK) {
        throw EKitException(
            func_name, res, "Failed to read device configuration");
    }

    adxl_config.fifo_len    = watermark_samples;
    adxl_config.fifo_mode   = ADXL345::ADXL345Constants::FIFO_CTL_MODE_STREAM;
    adxl_config.trigger_int = false;
    adxl_config.self_test   = false;
    adxl_config.three_wire_spi = false;
    adxl_config.int_invert     = false;
    adxl_config.full_res       = true;
    adxl_config.justify_msb    = true;
    adxl_config.range          = range;
    adxl_config.low_power      = false;
    adxl_config.rate           = rate;
    adxl_config.link           = false;
    adxl_config.auto_sleep     = false;
    adxl_config.measure        = false;
    adxl_config.sleep          = false;
    adxl_config.wakeup_rate = ADXL345::ADXL345Constants::POWER_CTL_WAKEUP_8HZ;

    res                     = write_config_priv(adxl_config, to);
    if (res != EKIT_OK) {
        throw EKitException(
            func_name, res, "Failed to update device configuration");
    }
}

uint8_t ADXL345::get_events() {
    static const char* const func_name = "ADXL345::get_events";
    uint8_t                  events;
    EKitTimeout              to(get_timeout());
    BusLocker                blocker(bus, to);
    EKIT_ERROR err = single_byte_transaction(true, INT_SOURCE, events, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "SPI transaction failed.");
    }

    return events;
}

void ADXL345::clear_fifo() {
    static const char* const func_name = "ADXL345::clear_fifo";
    ADXL345Confiuration      config;
    ADXL345Confiuration      bypass_mode_config;

    EKitTimeout              to(get_timeout());
    BusLocker                blocker(bus, to);

    EKIT_ERROR               err = read_config_priv(config, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to read config");
    }

    bypass_mode_config           = config;
    bypass_mode_config.fifo_mode = ADXL345Constants::FIFO_CTL_MODE_BYPASS;
    bypass_mode_config.measure   = false;

    err                          = write_config_priv(bypass_mode_config, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to switch to bypass mode");
    }

    err = write_config_priv(config, to);
    if (err != EKIT_OK) {
        throw EKitException(
            func_name, err, "Failed to switch to original mode");
    }
}

void ADXL345::get_offset_data(ADXL345OffsetData& data) {
    static const char* const func_name = "ADXL345::get_offset_data";
    constexpr size_t         data_len  = sizeof(ADXL345OffsetData);
    data.header                        = ADXL345Constants::READ_REG_FLAG
                  | ADXL345Constants::MULTYBYTE_FLAG | ADXL345Registers::OFSX;

    EKitTimeout to(get_timeout());
    BusLocker   blocker(bus, to);
    EKIT_ERROR  err = bus->write_read(reinterpret_cast<const uint8_t*>(&data),
                                     data_len,
                                     reinterpret_cast<uint8_t*>(&data),
                                     data_len,
                                     to);

    if (err != EKIT_OK) {
        throw EKitException(
            func_name, err, "Failed to request OSX, OSY and OSZ registers.");
    }
}

void ADXL345::set_offset_data(ADXL345OffsetData& data) {
    static const char* const func_name = "ADXL345::set_offset_data";
    constexpr size_t         data_len  = sizeof(ADXL345OffsetData);
    data.header = ADXL345Constants::MULTYBYTE_FLAG | ADXL345Registers::OFSX;

    EKitTimeout to(get_timeout());
    BusLocker   blocker(bus, to);
    EKIT_ERROR  err = bus->write_read(
        reinterpret_cast<const uint8_t*>(&data), data_len, nullptr, 0, to);

    if (err != EKIT_OK) {
        throw EKitException(
            func_name, err, "Failed to update OSX, OSY and OSZ registers.");
    }
}

void ADXL345::to_double_data(const ADXL345Data& int_data,
                             ADXL345DataDbl&    dbl_data) const{
    dbl_data.x = max_val * (double)int_data.x * res_scale;
    dbl_data.y = max_val * (double)int_data.y * res_scale;
    dbl_data.z = max_val * (double)int_data.z * res_scale;
}
