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

#include "ekit_firmware.hpp"

ADCDev::ADCDev(std::shared_ptr<EKitBus>& ebus, const ADCConfig* cfg)
    : super(ebus, cfg->dev_id, cfg->dev_name), config(cfg) {
    vref_cur = config->vref;
}

ADCDev::~ADCDev() {}

std::string ADCDev::get_input_name(size_t index, bool channel_name) const {
    static const char* const func_name = "ADCDev::get_input_name";
    if (index >= config->input_count) {
        throw EKitException(
            func_name, EKIT_BAD_PARAM, "ADC input index is out of range");
    }

    return channel_name ? config->inputs[index].adc_input
                        : config->inputs[index].in_name;
}

size_t ADCDev::get_input_count() const { return config->input_count; }

void   ADCDev::start(uint16_t sample_count, double delay_sec) {
    static const char* const func_name = "ADCDev::start";
    ADCDevCommand            data;
    double                   expected;
    uint8_t                  f = 0;

    data.sample_count          = sample_count;

    // figure out period and prescaller
    if (delay_sec == 0) {
        data.timer_prescaller = 0;
        data.timer_period     = 0;
    } else {
        int res = tools::stm32_timer_params(config->timer_freq,
                                            delay_sec,
                                            data.timer_prescaller,
                                            data.timer_period,
                                            expected);
        if (res > 0) {
            throw EKitException(
                func_name, EKIT_BAD_PARAM, "delay_sec is too long");
        } else if (res < 0) {
            throw EKitException(
                func_name, EKIT_BAD_PARAM, "delay_sec is too short");
        }
    }

    if (sample_count == 0) {
        f |= ADCDEV_UNSTOPPABLE;
    }

    // Do I/O operation
    {
        EKitTimeout to(get_timeout());
        BusLocker   blocker(bus, get_addr(), to);

        EKIT_ERROR  err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, f, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Write data
        err = bus->write((uint8_t*)&data, sizeof(ADCDevCommand), to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void ADCDev::stop(bool reset_buffer) {
    static const char* const func_name = "ADCDev::stop";
    uint8_t                  f         = 0;

    if (reset_buffer) {
        f |= ADCDEV_RESET_DATA;
    }

    // Do I/O operation
    {
        EKitTimeout to(get_timeout());
        BusLocker   blocker(bus, get_addr(), to);

        EKIT_ERROR  err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, f, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Write zero-length buffer (just command byte, this will cause sampling
        // stop)
        err = bus->write(nullptr, 0, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void ADCDev::get(std::vector<uint16_t>& data, bool& ovf) {
    static const char* const func_name = "ADCDev::get(1)";
    // issue a command
    {
        EKitTimeout        to(get_timeout());
        BusLocker          blocker(bus, get_addr(), to);

        // get amount of data
        CommResponseHeader hdr;
        EKIT_ERROR         err =
            std::dynamic_pointer_cast<EKitFirmware>(bus)->get_status(
                hdr, false, to);
        if (err != EKIT_OK && err != EKIT_OVERFLOW) {
            throw EKitException(func_name, err, "get_status() failed");
        }

        // check for overflow
        ovf = (hdr.comm_status & COMM_STATUS_OVF) != 0;

        if (hdr.length == 0) goto done;

        // sanity check, buffer must be aligned by size of all inputs
        if ((hdr.length % (config->input_count * sizeof(uint16_t))) != 0) {
            assert(false);
            goto done;
        }

        // resize buffer
        data.resize(hdr.length / sizeof(uint16_t));

        // read data
        err = bus->read((uint8_t*)data.data(), hdr.length, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "read() failed");
        }
    }

done:
    return;
}

// returns all samples converted to double
void ADCDev::get(std::vector<std::vector<double>>& values, bool& ovf) {
    static const char* const func_name = "ADCDev::get(2)";
    std::vector<uint16_t>    data;
    get(data, ovf);
    size_t sample_count = data.size() / config->input_count;

    values.clear();

    // convert into doubles
    for (size_t ch = 0; ch < config->input_count; ch++) {
        std::vector<double> ch_val;

        for (int s = 0; s < sample_count; s++) {
            uint16_t v = data.at(ch + s * config->input_count);
            double   x = vref_cur * ((double)v / (double)config->adc_maxval);
            ch_val.push_back(x);
        }

        values.push_back(std::move(ch_val));
    }
}

// returns average for all samples converted to double
void ADCDev::get(std::vector<double>& values, bool& ovf) {
    static const char* const func_name = "ADCDev::get(3)";
    std::vector<uint16_t>    data;
    get(data, ovf);
    size_t sample_count = data.size() / config->input_count;

    values.clear();

    // convert into doubles
    for (size_t ch = 0; ch < config->input_count; ch++) {
        uint32_t acc = 0;
        for (int s = 0; s < sample_count; s++) {
            acc += data.at(ch + s * config->input_count);
        }

        acc      = acc / sample_count;
        double x = vref_cur * ((double)acc / (double)config->adc_maxval);
        values.push_back(x);
    }
}

void ADCDev::set_vref(double Vref_plus) { vref_cur = Vref_plus; }
void ADCDev::set_vref(uint16_t vref_channel, double V_ref_int) {
    vref_cur = V_ref_int * (double)config->adc_maxval / (double)vref_channel;
}
