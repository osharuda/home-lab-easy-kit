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
 *   \brief SPIDAC device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include <functional>
#include "ekit_device.hpp"
#include "spidac_common.hpp"

/// \defgroup group_spidac SPIDACDev
/// \brief SPIDAC support
/// @{
/// \page page_spidac
/// \tableofcontents
///
/// \section sect_spidac_01 Work with SPIDACDev
///
/// SPIDACDev functionality provides the following features:
/// - features list ...
///
/// Basic logic of SPIDACDev functionality work is shown on the following schema:
/// \image html SPIDACDev_schema.png
/// \image latex SPIDACDev_schema.eps
///
/// SPIDACDev can be used as:
/// 1. Create SPIDACDev object
/// 2. Call SPIDACDev#do_something() method to do something.
///

struct SPIDACChannelConfig {
    std::string name;
    std::vector<double> samples;
    uint32_t address;
    double min_value;
    double max_value;
    double default_value;
};

// Key - address
// Value - configuration
using SPIDAC_CHANNELS_CONFIG=std::map<uint32_t , struct SPIDACChannelConfig>;

/// \brief Appends SPI frame for the sample as sequence of bytes
/// \param value - value
/// \param min_value - minimal value for the channel
/// \param max_value - maximum value for the channel
/// \param address - address
/// \param buffer - buffer to be appended
using APPEND_SPI_SAMPLE_FUNC=std::function<void(double, double, double, uint32_t, std::vector<uint8_t>&)>;

struct SPIDACWaveformParam {
    double amplitude;
    double offset;
    double start_x;
    double stop_x;
    double sigma;
};

extern struct SPIDACWaveformParam spidac_default_sin_cos_param;
std::vector<double> spidac_waveform_sin(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_sin_cos_param);
std::vector<double> spidac_waveform_cos(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_sin_cos_param);

extern struct SPIDACWaveformParam spidac_default_saw_triangle_param;
std::vector<double> spidac_waveform_pos_saw(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_saw_triangle_param);
std::vector<double> spidac_waveform_neg_saw(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_saw_triangle_param);
std::vector<double> spidac_waveform_triangle(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_saw_triangle_param);

extern struct SPIDACWaveformParam spidac_default_gauss_param;
std::vector<double> spidac_waveform_gauss(size_t n_samples, struct SPIDACWaveformParam* wf_param = &spidac_default_gauss_param);

/// \class SPIDACDev
/// \brief SPIDACDev implementation. Use this class in order to control SPIDACDev virtual devices.
class SPIDACDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

    /// Function to form SPI frames
    APPEND_SPI_SAMPLE_FUNC append_spi_sample_func;

	public:

    /// \brief Pointer to the #tag_SPIDACInstance structure that describes SPIDACDev virtual device represented by this class.
    const SPIDACConfig* config;

    /// \brief No default constructor
    SPIDACDev() = delete;

    /// \brief Copy construction is forbidden
    SPIDACDev(const SPIDACDev&) = delete;

    /// \brief Assignment is forbidden
    SPIDACDev& operator=(const SPIDACDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	SPIDACDev(std::shared_ptr<EKitBus>& ebus, const SPIDACConfig* config);

    /// \brief Destructor (virtual)
	~SPIDACDev() override;

    /// \brief Returns configured number of bits per sample.
    /// \return Number of bits per sample
    size_t get_bits_per_sample() const;

    /// \brief Returns the number of channels configured.
    /// \return Number of channels.
    size_t get_channels_count() const;

    double get_default_value(uint32_t address) const;

    void set_default_value(uint32_t address, double value);

    double get_min_value(uint32_t address) const;

    void set_min_value(uint32_t address, double value);

    double get_max_value(uint32_t address) const;

    void set_max_value(uint32_t address, double value);

    std::string get_channel_name(uint32_t address) const;

    std::vector<uint32_t> get_channels_list() const;

    void set_samples(uint32_t address, const std::vector<double>& samples);

    void clear_samples(uint32_t address);

    /// \brief Returns total (for all channels) internal device buffer length in samples.
    size_t get_buffer_len();

    /// \brief Upload prepared waveforms (samples) into device internal buffer
    /// \param default_vals - true of default values were uploaded and must be set to device (in this case another,
    ///        default sample buffer is used by device, it has capacity for single sample per each channel).
    void upload(bool default_vals);

    /// \brief Stops signal generation and sets default value.
    void stop();

    /// \brief Starts continuous sampling
    /// \param freq - rate of the signal
    /// \param start_phase - start phase (in samples)
    /// \param phase_inc - phase increment (in samples)
    /// \param continuous - true to repeat signal indefinitely, false generate single period.
    void start_signal(double freq, size_t start_phase, size_t phase_inc, bool continuous);


    /// \brief Returns information regarding current SPIDAC device status
    SPIDAC_STATUS get_status();

    private:

    void reset_config();

    SPIDAC_CHANNELS_CONFIG channels;
    bool little_endian;
/*
    /// \brief Appends frame with DAC7611 sample to the buffer
    /// \param value - sample value
    /// \param channel_index - channel index
    /// \param buffer - buffer to be appended with the frame
    /// \warning This method is reflection of the SPIDACCustomizer::get_frame_for_channel() from customizer part.
    ///          Implementation of these methods should match!
    void append_dac7611_sample(double value, size_t channel_index, std::vector<uint8_t>& buffer);

    /// \brief Appends frame with DAC8550 sample to the buffer
    /// \param value - sample value
    /// \param channel_index - channel index
    /// \param buffer - buffer to be appended with the frame
    /// \warning This method is reflection of the SPIDACCustomizer::get_frame_for_channel() from customizer part.
    ///          Implementation of these methods should match!
    void append_dac8550_sample(double value, size_t channel_index, std::vector<uint8_t>& buffer);
*/


    /// \brief Uploads prepared buffer to the DAC device
    /// \param buffer - buffer to be uploaded.
    void upload_data(const std::vector<uint8_t>& buffer);

    /// \brief Uploads default sample to the DAC device
    /// \param buffer - buffer to be uploaded.
    void upload_default_sample(const std::vector<uint8_t>& buffer);

    /// \brief Transforms frame to confirm DAC requirements
    /// \param frame - pointer to the frame to be transformed.
    /// \param frame_size - size of the frame
    /// \return pointer to the transformed frame. Size of the frame remains the same.
    /// \note Transformation is made in place therefore \ref frame parameter is returned.
    uint8_t* re_align_frame(uint8_t* frame, size_t frame_size);

    void validate_values_are_in_range(uint32_t address) {
        const char* func_name = "SPIDACDev::validate_values_are_in_range";
        const struct SPIDACChannelConfig& ch_config = channels.at(address);
        const size_t sample_num = ch_config.samples.size();


        for (size_t i = 0; i<sample_num; i++) {
            double v = ch_config.samples[i];
            if (v < ch_config.min_value) {
                throw EKitException(func_name, EKIT_BAD_PARAM, "Value is less then the minimal possible value.");
            }

            if (v > ch_config.max_value) {
                throw EKitException(func_name, EKIT_BAD_PARAM, "Value is higher then the maximum possible value.");
            }
        }
    }

    void validate_sampling_frequency(double freq) {
        const char* func_name = "SPIDACDev::validate_sampling_frequency";
        if (freq <= 0.0L) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "Sampling frequency is too low.");
        }

        if (freq >= 800000.0L) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "Sampling frequency is too high.");
        }
    }
};

/// @}
