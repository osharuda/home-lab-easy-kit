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
 *   \brief SPIDAC device software implementation
 *   \author Oleh Sharuda
 */

#include "spidac.hpp"
#include "ekit_firmware.hpp"
#include <math.h>
#include <limits.h>

SPIDACDev::SPIDACDev(std::shared_ptr<EKitBus>& ebus, const SPIDACConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg)
    {
	static const char* const func_name = "SPIDACDev::SPIDACDev";

    default_value.resize(config->channel_count);
    value_range.resize(config->channel_count);
    for (size_t i=0; i<config->channel_count; i++) {
        const SPIDACChannelDescriptor* channel_descriptor = config->channels + i;
        default_value[i] = channel_descriptor->default_value;
        value_range[i] = SPIDAC_VALUE_RANGE(channel_descriptor->min_value, channel_descriptor->max_value);
    }

    little_endian = tools::is_little_endian();
}

SPIDACDev::~SPIDACDev() {
}

size_t SPIDACDev::get_bits_per_sample() const {
    return config->bits_per_sample;
}

size_t SPIDACDev::get_channels_count() const {
    return config->channel_count;
};

size_t SPIDACDev::get_frames_per_sample() const {
    return config->frames_per_sample;
}

SPIDAC_SAMPLE_VECT SPIDACDev::get_default_value() const {
    return default_value;
}

void SPIDACDev::set_default_values(const SPIDAC_SAMPLE_VECT& values) {
    const char* func_name = "SPIDACDev::set_default_values";
    SPIDAC_CHANNELS channels = values_to_channels(values);
    validate_values_are_in_range(channels, value_range);
    default_value = values;

    upload(channels, true);
}

bool SPIDACDev::is_running() {
    const char* func_name = "SPIDACDev::is_running";
    EKIT_ERROR err = EKIT_OK;
    SPIDACStatus status;
    BusLocker blocker(bus);

    err = bus->read(&status, sizeof(SPIDACStatus));
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read failed.");
    }
    return status.status != SHUTDOWN;
}

void SPIDACDev::start_signal(double freq, size_t phase_inc, bool continuous) {
    static const char* const func_name = "SPIDACDev::start";
    EKIT_ERROR err = EKIT_OK;
    validate_sampling_frequency(freq);
    double eff_sample_rate;
    phase_inc *= config->frames_per_sample*(config->frame_size);

    if (phase_inc > USHRT_MAX) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "phase_inc is out of limits");
    }

    BusLocker blocker(bus);

    // Start
    SPIDAC_COMMAND cmd = continuous ? SPIDAC_COMMAND::START : SPIDAC_COMMAND::START_PERIOD;
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    // Write zero-length buffer (just command byte, this will cause sampling stop)
    SPIDACSampling sampling;
    sampling.phase_increment = phase_inc;
    tools::stm32_timer_params(config->timer_freq, 1.0L/freq, sampling.prescaler, sampling.period, eff_sample_rate);

    err = bus->write(&sampling, sizeof(sampling));
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void SPIDACDev::set_value_range(  const SPIDAC_CHANNELS_VALUE_RANGE& vr,
                                    const SPIDAC_SAMPLE_VECT& dv) {
    SPIDAC_CHANNELS channels = values_to_channels(dv);
    validate_values_are_in_range(channels, vr);
    value_range = vr;
    set_default_values(dv);
}

SPIDAC_CHANNELS_VALUE_RANGE SPIDACDev::get_value_range() {
    return value_range;
}

size_t SPIDACDev::get_max_samples_per_channel() {
    return config->max_sample_count;
}

void SPIDACDev::upload(const SPIDAC_CHANNELS channels, bool def_vals) {
    static const char* const func_name = "SPIDACDev::upload";
    EKIT_ERROR err = EKIT_OK;
    validate_values_are_in_range(channels, value_range);

    // Construct buffer
    std::vector<uint8_t> buffer;
    size_t samples_per_channel = channels[0].size();

    if (def_vals) {
        assert(samples_per_channel==1);
    } else {
        assert(samples_per_channel>0);
    }

    for (size_t i=0; i<samples_per_channel; i++) {
        for (size_t c=0; c<config->channel_count; c++) {
            append_frame_with_sample(channels[c][i], c, buffer);
        }
    }

    upload_raw(buffer, def_vals);
}
void SPIDACDev::stop() {
    static const char* const func_name = "SPIDACDev::stop";
    EKIT_ERROR err = EKIT_OK;
    BusLocker blocker(bus);

    // Stop
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, SPIDAC_COMMAND::STOP);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    // Write zero-length buffer (just command byte, this will cause sampling stop)
    err = bus->write(nullptr, 0);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void SPIDACDev::upload_raw(const std::vector<uint8_t>& buffer, bool def_vals) {
    static const char* const func_name = "SPIDACDev::upload_raw";
    size_t buf_len = buffer.size();
    EKIT_ERROR err = EKIT_OK;
    if (buf_len > config->dev_buffer_len) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Passed data exceed device buffer size");
    }

    if (buf_len % (config->frame_size * config->frames_per_sample) != 0) {
        throw EKitException(func_name, EKIT_UNALIGNED, "Passed data doesn't match with device sampling alignment parameters");
    }

    BusLocker blocker(bus);
    SPIDAC_COMMAND cmd = def_vals ? SPIDAC_COMMAND::SETDEFAULT : SPIDAC_COMMAND::DATA;
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer.data(), buf_len);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void SPIDACDev::append_frame_with_sample(double value, size_t channel_index, std::vector<uint8_t>& buffer) {
    const char* func_name = "SPIDACDev::append_frame_with_sample";

    assert(config->frame_size==2);   // Two byte frames are supported only
    assert((config->bits_per_sample > 8) && (config->bits_per_sample <= 16)); // Resolution between 9 and 16 bits only

    double min_val = value_range[channel_index].first;
    double max_val = value_range[channel_index].second;
    assert(min_val < max_val);
    assert(min_val <= value);
    assert(value <= max_val);
    double x = (value - min_val) / (max_val - min_val);
    assert(x>=0.0L);
    assert(x<=1.0L);

    // Conversion
    uint64_t bits_mask = (1 << config->bits_per_sample) - 1;
    uint64_t x64 = (uint64_t)(bits_mask * x);
    uint16_t x16 = (uint16_t)(x64 & bits_mask);
    uint8_t* frame = (uint8_t*)&x16;
    size_t frame_size = 2;

    switch (config->frame_format) {
        case MSB:
            if (little_endian) {
                for (int i = 0; i<frame_size; i++) buffer.push_back(frame[i]);
            } else {
                for (int i = frame_size-1; i>=0; i--) buffer.push_back(frame[i]);
            }
            break;
        case LSB:
        default:
            throw EKitException(func_name, EKIT_NOT_SUPPORTED, "Frame format is not implemented.");
    };
}
