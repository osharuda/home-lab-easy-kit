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
 *   \brief ADC device software implementation
 *   \author Oleh Sharuda
 */

#include "adcdev.hpp"
#include "tools.hpp"
#include "ekit_firmware.hpp"

ADCDev::ADCDev(std::shared_ptr<EKitBus>& ebus, const ADCConfig* cfg)
    : super(ebus, cfg->dev_id, cfg->dev_name), config(cfg) {
    static const char* const func_name = "ADCDev::ADCDev";

    // Preallocate read buffer
    if ((cfg->dev_buffer_len % sizeof(uint16_t)) != 0) {
        throw EKitException(func_name, EKIT_UNALIGNED, "Device buffer size is unaligned.");
    }

    signal_ranges.resize(config->input_count);
    for( auto sr = signal_ranges.begin(); sr!=signal_ranges.end(); ++sr) {
        sr->first = 0.0L;
        sr->second = 3.3L;
    }

    data_buffer.resize(cfg->dev_buffer_len + sizeof(uint16_t));
    data_status = static_cast<volatile uint16_t*>(data_buffer.data());
    data = data_status + 1;
}

ADCDev::~ADCDev() {
}

std::string ADCDev::get_input_name(size_t index, bool channel_name) const {
    static const char* const func_name = "ADCDev::get_input_name";
    if (index >= config->input_count) {
        throw EKitException(
            func_name, EKIT_BAD_PARAM, "ADC input index is out of range");
    }

    return channel_name ? config->inputs[index].adc_input
                        : config->inputs[index].in_name;
}

size_t ADCDev::get_input_count() const {
    return config->input_count;
}

void   ADCDev::start(uint16_t sample_count) {
    static const char* const func_name = "ADCDev::start";
    ADCDevCommand data = {.sample_count=sample_count};
    send_command((uint8_t *) &data, sizeof(ADCDevCommand), ADCDEV_START);
}

void ADCDev::stop() {
    static const char* const func_name = "ADCDev::stop";
    send_command(nullptr, 0, ADCDEV_STOP);
}

void ADCDev::reset() {
    static const char* const func_name = "ADCDev::reset";
    send_command(nullptr, 0, ADCDEV_RESET);
}

void ADCDev::configure(double delay_sec, size_t average_samples, std::map<size_t, uint8_t>& sampling) {
    static const char* const func_name = "ADCDev::set_data";
    std::vector<uint8_t> adc_config_buffer(sizeof(ADCDevConfig) + config->input_count, 0);
    ADCDevConfig* adc_config = reinterpret_cast<ADCDevConfig*>(adc_config_buffer.data());
    uint8_t* adc_channel_sampling = adc_config_buffer.data() + sizeof(ADCDevConfig);
    double  expected;

    if ((average_samples <= 0) || (average_samples > config->measurements_per_sample)) {
        throw EKitException(
            func_name, EKIT_BAD_PARAM, "Average sampling number doesn't match with device configuration.");
    }
    adc_config->measurements_per_sample = static_cast<uint16_t>(average_samples);

    // figure out period and prescaller
    if (delay_sec == 0) {
        adc_config->timer_prescaller = 0;
        adc_config->timer_period     = 0;
    } else {
        int res = tools::stm32_timer_params(config->timer_freq,
                                            delay_sec,
                                            adc_config->timer_prescaller,
                                            adc_config->timer_period,
                                            expected);
        if (res > 0) {
            throw EKitException(
                func_name, EKIT_BAD_PARAM, "delay_sec is too long");
        } else if (res < 0) {
            throw EKitException(
                func_name, EKIT_BAD_PARAM, "delay_sec is too short");
        }
    }

    for (size_t ch=0; ch<config->input_count; ch++) {
        adc_channel_sampling[ch] = tools::get_with_default(sampling, ch, config->inputs[ch].default_sampling_time);
    }

    // Do I/O operation
    send_command(adc_config_buffer.data(), adc_config_buffer.size(), ADCDEV_CONFIGURE);
}

void ADCDev::send_command(uint8_t* ptr, size_t size, uint8_t command) {
    static const char* const func_name = "ADCDev::send_command";
    EKitTimeout to(get_timeout());
    BusLocker   blocker(bus, get_addr(), to);

    EKIT_ERROR  err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, command, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    // Write data
    err = bus->write(ptr, size, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void ADCDev::get(std::vector<std::vector<double>>& dst) {
    static const char* const func_name = "ADCDev::get(1)";
    EKitTimeout        to(get_timeout());
    BusLocker          blocker(bus, get_addr(), to);

    // get amount of data
    size_t data_size = status_priv(nullptr, to);
    assert(data_size>=sizeof(uint16_t));

    // read data (into data_buffer)
    EKIT_ERROR err = bus->read((uint8_t*)data_status, data_size, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }
    data_size = (data_size - sizeof(uint16_t)) / sizeof(uint16_t); // <- Number of measurements (all channels)
    size_t sample_count = data_size / config->input_count; // <- Number of samples

    // convert into doubles
    dst.resize(sample_count);
    for (int s = 0; s < sample_count; s++) {
        dst[s].resize(config->input_count);
        for (size_t ch = 0; ch < config->input_count; ch++) {
            uint16_t v = data[ch + s * config->input_count];
            double v_min = signal_ranges[ch].first;
            double v_max = signal_ranges[ch].second;
            double x = v_min + ((double)v / (double)config->adc_maxval) * (v_max - v_min);
            dst[s][ch] = x;
        }
    }
}

size_t ADCDev::status(uint16_t& flags) {
    static const char* const func_name = "ADCDev::status";

    EKitTimeout        to(get_timeout());
    BusLocker          blocker(bus, get_addr(), to);

    return (status_priv(&flags, to) - sizeof(uint16_t)) / sizeof(uint16_t);
}

size_t ADCDev::status_priv(uint16_t* flags, EKitTimeout& to) {
    static const char* const func_name = "ADCDev::status_priv";
    EKIT_ERROR err = EKIT_FAIL;
    CommResponseHeader hdr;

    auto fw = std::dynamic_pointer_cast<EKitFirmware>(bus);

    do {
        err = fw->get_status(hdr, false, to);
    } while ( (err!=EKIT_OK) && (hdr.comm_status & COMM_STATUS_BUSY) );
    assert((hdr.comm_status & COMM_STATUS_BUSY)==0);
/*
    if (hdr.comm_status & COMM_STATUS_OVF) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Device buffer overflow.");
    }

    if (hdr.comm_status & COMM_STATUS_CRC) {
        throw EKitException(func_name, EKIT_CRC_ERROR, "Communication CRC failure");
    }

    if (hdr.comm_status & COMM_STATUS_FAIL) {
        throw EKitException(func_name, EKIT_COMMAND_FAILED, "Communication with firmware has failed.");
    }
*/
    if (((hdr.length - sizeof(uint16_t)) % (config->input_count * sizeof(uint16_t)))!=0) {
        throw EKitException(func_name, EKIT_UNALIGNED, "Device buffer seems to be unaligned.");
    }

    if (flags == nullptr) goto done;

    err = bus->read((uint8_t*)data_status, sizeof(uint16_t), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }
    *flags = *data_status;

done:
    return hdr.length;
}

