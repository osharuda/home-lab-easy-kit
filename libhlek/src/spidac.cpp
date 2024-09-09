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
#include <functional>
#include "ekit_firmware.hpp"
#include <math.h>
#include <limits.h>
#include "tools.hpp"

#define SPIDAC_CHECK_APPEND_PARAM(val, min, max)                                                               \
    if ((min) >= (max)) {                                                                                      \
        throw EKitException(func_name, EKIT_BAD_PARAM, "Minimum value should be less then maximum value");     \
    }                                                                                                          \
    if ((value) > (max)) {                                                                                     \
        throw EKitException(func_name, EKIT_BAD_PARAM, "Value should be less then maximum value");             \
    }                                                                                                          \
    if ((value) < (min)) {                                                                                     \
        throw EKitException(func_name, EKIT_BAD_PARAM, "Value should be greater or equal then minimum value"); \
    }

static bool g_spidac_software_le = tools::is_little_endian();

/// \brief Appends frame with DAC8564 sample to the buffer
/// \param value - value
/// \param min_value - minimal value for the channel
/// \param max_value - maximum value for the channel
/// \param address - address
/// \param buffer - buffer to be appended
/// \warning This method is reflection of the SPIDACCustomizer::get_frame_for_channel() from customizer part.
///          Implementation of these methods should match!
void spidac_append_dac8564_sample(double value, double min_value, double max_value, size_t address, std::vector<uint8_t>& buffer) {
    /// DAC8564 format (Datasheet information:
    ///     "16-Bit, Quad Channel, Ultra-Low Glitch, Voltage Output DIGITAL-TO-ANALOG CONVERTER with 2.5V, 2ppm/°C Internal Reference",
    ///     internal shift register, page 29-30)
    ///
    /// Frame size - 24 bit
    /// 23 22 21  20   19  18    17   16    15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0
    /// A₁ A₀ LD₁ LD₀  0   SEL₁  SEL₀ PD₀   D₁₅  D₁₄  D₁₃  D₁₂  D₁₁  D₁₀  D₉  D₈  D₇  D₆  D₅  D₄  D₃  D₂  D₁  D₀
    /// A₁, A₀ - Chip selection. Currently set to zeroes, corresponding pins must be grounded.
    /// LD₁, LD₀ - Load command. LD₁=0 and LD₀=0 is used to do single channel store.
    ///            Once data for required channels is set, DACs are updated by LDAC signal.
    /// SEL₁  SEL₀ - Channel selection:
    ///              SEL₁=0  SEL₀=0 to select buffer A
    ///              SEL₁=0  SEL₀=1 to select buffer B
    ///              SEL₁=1  SEL₀=0 to select buffer C
    ///              SEL₁=1  SEL₀=1 to select buffer D
    /// PD₀ - Power down mode selection (by setting to 1)
    /// D₁₅ (MSB) - D₀ (LSB) - Data
    const char *func_name = "spidac_append_dac8564_sample";
    constexpr uint16_t max_val = 0xFFFF;
    SPIDAC_CHECK_APPEND_PARAM(value, min_value, max_value);
    double x = tools::normalize_value(value, min_value, max_value);
    uint16_t val = (uint16_t)(max_val * x);
    uint8_t *val_ptr = (uint8_t * ) & val;
    uint8_t cmd = (((uint8_t) address) & (uint8_t) 3) << 1;

    buffer.push_back((cmd)); // Normal mode.
    buffer.push_back((val_ptr[1]));
    buffer.push_back((val_ptr[0]));
}

/// \brief Appends frame with DAC7611 sample to the buffer
/// \param value - value
/// \param min_value - minimal value for the channel
/// \param max_value - maximum value for the channel
/// \param address - address
/// \param buffer - buffer to be appended
/// \warning This method is reflection of the SPIDACCustomizer::get_frame_for_channel() from customizer part.
///          Implementation of these methods should match!
void spidac_append_dac7611_sample(double value, double min_value, double max_value, size_t address, std::vector<uint8_t>& buffer) {
    const char *func_name = "spidac_append_dac7611_sample";
    constexpr uint16_t max_val = 0x0FFF;
    SPIDAC_CHECK_APPEND_PARAM(value, min_value, max_value);
    double x = tools::normalize_value(value, min_value, max_value);

    uint16_t val = (uint16_t) (max_val * x);

    uint8_t bl = (uint8_t) (val & 0xFF);
    uint8_t bh = (uint8_t) (val >> 8);

    buffer.push_back(bh);
    buffer.push_back(bl);
}

void spidac_append_dac8550_sample(double value, double min_value, double max_value, size_t address, std::vector<uint8_t>& buffer) {
    /// DAC8550 format (Datasheet information:
    ///     "DAC8550 16-bit, Ultra-Low Glitch, Voltage Output Digital-To-Analog Converter",
    ///     internal shift register, page 19)
    ///
    /// Frame size - 24 bit (numbered as transferred)
    /// 23 22 21 20 19 18    17  16    15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0
    /// [  U N U S E D  ]    PD₁ PD₀   D₁₅  D₁₄  D₁₃  D₁₂  D₁₁  D₁₀  D₉  D₈  D₇  D₆  D₅  D₄  D₃  D₂  D₁  D₀
    /// PD₁, PD₀ - Mode
    /// PD₁=0, PD₀=0 : normal mode
    /// PD₁=0, PD₀=1 : Output typically 1kΩ to GND
    /// PD₁=1, PD₀=0 : Output typically 100kΩ to GND
    /// PD₁=1, PD₀=1 : High-Z state
    /// D₁₅ (MSB) - D₀ (LSB) - Data

    const char* func_name = "spidac_append_dac8550_sample";
    constexpr uint16_t max_val = 0xFFFF;
    SPIDAC_CHECK_APPEND_PARAM(value, min_value, max_value);
    double x = tools::normalize_value(value, min_value, max_value) - 0.5;
    int16_t val = (int16_t)((double)max_val * x);

    uint8_t cmd_byte = 0; // Normal mode.
    uint8_t low_nibble = (uint8_t)(val & 0xFF);
    uint8_t high_nibble = (uint8_t)(val >> 8);

    buffer.push_back(cmd_byte);
    buffer.push_back(high_nibble);
    buffer.push_back(low_nibble);
}

struct SPIDACWaveformParam spidac_default_sin_cos_param = {.amplitude = 0.5L, .offset = 0.5L, .start_x=0.0L, .stop_x=2*M_PI, .sigma=0.0L};
std::vector<double> spidac_waveform_sin(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples > 1);
    std::vector<double> result;
    double range = (wf_param->stop_x - wf_param->start_x);
    double phase = wf_param->start_x;
    double x;
    for (size_t i = 0; i<n_samples; i++) {
        x = wf_param->amplitude * sin(phase) + wf_param->offset;
        result.push_back(x);
        phase = wf_param->start_x + range * i / (n_samples - 1);
    }
    return result;
}

std::vector<double> spidac_waveform_cos(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples > 1);
    std::vector<double> result;
    double range = (wf_param->stop_x - wf_param->start_x);
    double phase = wf_param->start_x;
    double x;
    for (size_t i = 0; i<n_samples; i++) {
        x = wf_param->amplitude * cos(phase) + wf_param->offset;
        result.push_back(x);
        phase = wf_param->start_x + range * i / (n_samples - 1);
    }
    return result;
}

struct SPIDACWaveformParam spidac_default_saw_triangle_param = {.amplitude = 1.0L, .offset = 0.0L, .start_x=0.0L, .stop_x=1.0, .sigma=0.0L};
std::vector<double> spidac_waveform_pos_saw(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples > 1);
    std::vector<double> result;
    double x;
    for (int i = 0; i<n_samples; i++) {
        x = i * wf_param->amplitude / (n_samples - 1) + wf_param->offset;
        result.push_back(x);
    }
    return result;
}

std::vector<double> spidac_waveform_neg_saw(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples > 1);
    std::vector<double> result;
    double x;
    for (int i = n_samples-1; i>=0; i--) {
        x = i * wf_param->amplitude / (n_samples - 1) + wf_param->offset;
        result.push_back(x);
    }
    return result;
}

std::vector<double> spidac_waveform_triangle(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples >= 3);
    std::vector<double> result;
    int n = n_samples / 2;
    double x;
    for (int i = 0; i<n; i++) {
        x = i * wf_param->amplitude / (n - 1) + wf_param->offset;
        result.push_back(x);
    }

    n = n_samples - n;
    assert(n != 0);
    for (int i = n - 1; i>=0; i--) {
        x = i * wf_param->amplitude / (n - 1) + wf_param->offset;
        result.push_back(x);
    }
    return result;
}

struct SPIDACWaveformParam spidac_default_gauss_param = {.amplitude = 1.0L, .offset = 0.0L, .start_x=-1.0L, .stop_x=1.0, .sigma=7.0L};
std::vector<double> spidac_waveform_gauss(size_t n_samples, struct SPIDACWaveformParam* wf_param) {
    assert(n_samples > 1);
    std::vector<double> result;
    double range = (wf_param->stop_x - wf_param->start_x);
    double x,y;
    for (size_t i = 0; i<n_samples; i++) {
        x = wf_param->start_x + i * range / (n_samples - 1);
        y = wf_param->amplitude * exp( -1.0L * wf_param->sigma * x * x) + wf_param->offset;
        result.push_back(y);
    }
    return result;
}




SPIDACDev::SPIDACDev(std::shared_ptr<EKitBus>& ebus, const SPIDACConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg)
    {
	static const char* const func_name = "SPIDACDev::SPIDACDev";
    reset_config();
    switch (config->sample_format) {
        case SPIDAC_SAMPLE_FORMAT_DAC8564:
            append_spi_sample_func = std::bind(spidac_append_dac8564_sample,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3,
                                              std::placeholders::_4,
                                              std::placeholders::_5);
        break;

        case SPIDAC_SAMPLE_FORMAT_DAC7611:
            append_spi_sample_func = std::bind(spidac_append_dac7611_sample,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3,
                                               std::placeholders::_4,
                                               std::placeholders::_5);
        break;

        case SPIDAC_SAMPLE_FORMAT_DAC8550:
            append_spi_sample_func = std::bind(spidac_append_dac8550_sample,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3,
                                               std::placeholders::_4,
                                               std::placeholders::_5);
        break;

        default:
            throw EKitException(func_name, EKIT_BAD_PARAM, "Unsupported sample format");

    }

}

SPIDACDev::~SPIDACDev() {
}

size_t SPIDACDev::get_bits_per_sample() const {
    return config->bits_per_sample;
}

size_t SPIDACDev::get_channels_count() const {
    return config->channel_count;
};

double SPIDACDev::get_default_value(uint32_t address) const {
    return channels.at(address).default_value;
}

void SPIDACDev::set_default_value(uint32_t address, double value) {
    channels[address].default_value = value;
}

double SPIDACDev::get_min_value(uint32_t address) const {
    return channels.at(address).min_value;
}

void SPIDACDev::set_min_value(uint32_t address, double value) {
    channels[address].min_value = value;
}

double SPIDACDev::get_max_value(uint32_t address) const {
    return channels.at(address).max_value;
}

void SPIDACDev::set_max_value(uint32_t address, double value) {
    channels[address].max_value = value;
}

std::string SPIDACDev::get_channel_name(uint32_t address) const {
    return channels.at(address).name;
}

std::vector<uint32_t> SPIDACDev::get_channels_list() const {
    std::vector<uint32_t> result(config->channel_count);
    for (size_t i=0; i<config->channel_count; i++) {
        result[i] = config->channels[i].address;
    }
    return result;
}

void SPIDACDev::set_samples(uint32_t address, const std::vector<double>& samples) {
    channels[address].samples = samples;
}

void SPIDACDev::clear_samples(uint32_t address) {
    channels[address].samples.clear();
}

int SPIDACDev::get_phase(uint32_t address) const {
    return channels.at(address).phase;
}

void SPIDACDev::set_phase(uint32_t address, int value) {
    channels.at(address).phase = value;
}

size_t SPIDACDev::get_phase_increment(uint32_t address) const {
    return channels.at(address).phase_increment;
}

void SPIDACDev::set_phase_increment(uint32_t address, size_t value) {
    channels.at(address).phase_increment = value;
}

SPIDAC_STATUS SPIDACDev::get_status() {
    const char* func_name = "SPIDACDev::get_status";
    EKIT_ERROR err = EKIT_OK;
    SPIDACStatus status;
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    err = bus->read(&status, sizeof(SPIDACStatus), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read failed.");
    }
    return (SPIDAC_STATUS)status.status;
}

void SPIDACDev::start(double freq, bool continuous) {
    static const char* const func_name = "SPIDACDev::start";
    EKIT_ERROR err = EKIT_OK;
    validate_sampling_frequency(freq);

    size_t start_info_len = sizeof(SPIDACStartInfo) + sizeof(SPIDACChannelSamplingInfo)*config->channel_count;
    std::vector<uint8_t> start_info_buffer(start_info_len);
    struct SPIDACStartInfo* start_info = reinterpret_cast<struct SPIDACStartInfo*>(start_info_buffer.data());

    // Prepare prescaler and period
    double eff_sample_rate;
    tools::stm32_timer_params(config->timer_freq, 1.0L/freq, start_info->prescaler, start_info->period, eff_sample_rate);

    // Prepare channels sampling information
    struct SPIDACChannelSamplingInfo* channel_info = start_info->channel_info;
    for (auto c : channels) {
        uint32_t address = c.first;
        struct SPIDACChannelConfig& ch_config = c.second;

        channel_info->phase.phase_increment = ch_config.phase_increment % channel_info->loaded_samples_number;
        channel_info->loaded_samples_number = ch_config.samples.size();
        channel_info->phase.phase = ch_config.phase;

        if (channel_info->loaded_samples_number == 0) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "no samples are loaded into the channel");
        }

        if (channel_info->phase.phase_increment > channel_info->loaded_samples_number) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "phase_inc is out of limits");
        }

        channel_info++; // next channel
    }

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    // Start
    SPIDAC_COMMAND cmd = continuous ? SPIDAC_COMMAND::START : SPIDAC_COMMAND::START_PERIOD;
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(start_info, start_info_len, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

int16_t SPIDACDev::normalize_phase(int16_t phase, size_t n) {
    static const char* const func_name = "SPIDACDev::normalize_phase";
    int t = phase / n;
    int result = ((int)phase + n * (abs(t) + 1)) % n;
    assert(result>=0);
    if (result > INT16_MAX) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Resulting phase overflow");
    }
    return result;
}

void SPIDACDev::update_phase() {
    static const char* const func_name = "SPIDACDev::update_phase";
    EKIT_ERROR err = EKIT_OK;

    size_t phase_update_info_len = sizeof(SPIDACChannelPhaseInfo)*config->channel_count;
    std::vector<uint8_t> phase_info_buffer(phase_update_info_len);
    struct SPIDACChannelPhaseInfo* phase_info = reinterpret_cast<struct SPIDACChannelPhaseInfo*>(phase_info_buffer.data());

    // Prepare channels sampling information
    for (auto c : channels) {
        uint32_t address = c.first;
        struct SPIDACChannelConfig& ch_config = c.second;
        size_t samples_count = ch_config.samples.size();

        phase_info->phase_increment = ch_config.phase_increment % samples_count;
        phase_info->phase = normalize_phase(ch_config.phase, samples_count);

        phase_info++; // next channel
    }

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    // Start
    SPIDAC_COMMAND cmd = SPIDAC_COMMAND::UPD_PHASE;
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(phase_info_buffer.data(), phase_update_info_len, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

size_t SPIDACDev::get_buffer_len() {
    return config->max_sample_count;
}

void SPIDACDev::upload(bool default_vals) {
    static const char* const func_name = "SPIDACDev::upload";
    EKIT_ERROR err = EKIT_OK;

    // Construct buffer
    std::vector<uint8_t> buffer;

    for (auto c: channels) {
        const uint32_t address = c.first;
        const struct SPIDACChannelConfig& ch_config = c.second;
        const size_t sample_num = ch_config.samples.size();

        if (default_vals) {
            append_spi_sample_func(ch_config.default_value, ch_config.min_value, ch_config.max_value, address, buffer);
        } else {
            validate_values_are_in_range(address);
            for (size_t s=0; s<sample_num; s++) {
                append_spi_sample_func(ch_config.samples[s], ch_config.min_value, ch_config.max_value, address, buffer);
            }
        }
    }

    if (default_vals) {
        upload_default_sample(buffer);
    } else {
        upload_data(buffer);
    }
}
void SPIDACDev::stop() {
    static const char* const func_name = "SPIDACDev::stop";
    EKIT_ERROR err = EKIT_OK;
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    // Stop
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, SPIDAC_COMMAND::STOP, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    // Write zero-length buffer (just command byte, this will cause sampling stop)
    err = bus->write(nullptr, 0, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void SPIDACDev::upload_default_sample(const std::vector<uint8_t>& buffer) {
    static const char* const func_name = "SPIDACDev::upload_default_sample";
    size_t buf_len = buffer.size();
    EKIT_ERROR err = EKIT_OK;
    if (buf_len > config->dev_buffer_len) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Passed data exceed device buffer size");
    }

    if (buf_len % (config->frame_size * config->frames_per_sample) != 0) {
        throw EKitException(func_name, EKIT_UNALIGNED, "Passed data doesn't match with device sampling alignment parameters");
    }

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    SPIDAC_COMMAND cmd = SPIDAC_COMMAND::SETDEFAULT;
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(buffer.data(), buf_len, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void SPIDACDev::upload_data(const std::vector<uint8_t>& buffer) {
    static const char* const func_name = "SPIDACDev::upload_data";
    size_t buf_len = buffer.size();
    EKIT_ERROR err = EKIT_OK;
    if (buf_len > config->dev_buffer_len) {
        throw EKitException(func_name, EKIT_OVERFLOW, "Passed data exceed device buffer size");
    }

    if (buf_len % (config->frame_size * config->frames_per_sample) != 0) {
        throw EKitException(func_name, EKIT_UNALIGNED, "Passed data doesn't match with device sampling alignment parameters");
    }

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    CommResponseHeader resp;
    std::dynamic_pointer_cast<EKitFirmware>(bus)->wait_vdev(resp, true, to);

    SPIDAC_COMMAND cmd = SPIDAC_COMMAND::DATA_START;
    size_t bytes_sent = 0;
    do {
        const uint8_t* buf_ptr = buffer.data() + bytes_sent;
        size_t bytes_to_sent = std::min(buf_len - bytes_sent, config->max_bytes_per_transaction);
        err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, cmd, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        err = bus->write(buf_ptr, bytes_to_sent, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
        bytes_sent += bytes_to_sent;
        cmd = SPIDAC_COMMAND::DATA;
    } while (bytes_sent < buf_len);
}


void SPIDACDev::reset_config() {
    static const char* const func_name = "SPIDACDev::reset_config";

    channels.clear();

    for (size_t i=0; i<config->channel_count; i++) {
        const SPIDACChannelDescriptor* channel_descriptor = config->channels + i;
        struct SPIDACChannelConfig ch_config;
        ch_config.address = channel_descriptor->address;
        ch_config.min_value = channel_descriptor->min_value;
        ch_config.max_value = channel_descriptor->max_value;
        ch_config.default_value = channel_descriptor->default_value;
        ch_config.name = channel_descriptor->name;
        ch_config.phase = 0;
        ch_config.phase_increment = 1;
        ch_config.samples.clear();

        channels[ch_config.address] = ch_config;
    }
}