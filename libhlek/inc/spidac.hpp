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

using SPIDAC_SAMPLE_VECT = std::vector<double>;
using SPIDAC_CHANNELS = std::vector<SPIDAC_SAMPLE_VECT>;
using SPIDAC_VALUE_RANGE = std::pair<double, double>; // first is minimum, second is maximum
using SPIDAC_CHANNELS_VALUE_RANGE = std::vector<SPIDAC_VALUE_RANGE>;

/// \class SPIDACDev
/// \brief SPIDACDev implementation. Use this class in order to control SPIDACDev virtual devices.
class SPIDACDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

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

    /// \brief Starts continuous sampling
    /// \param freq - rate of the signal
    /// \param phase_inc - phase increment (in samples)
    /// \param continuous - true to repeat signal indefinitely, false generate single period.
    void start_signal(double freq, size_t phase_inc, bool continuous);

    /// \brief Stops signal generation and sets default value.
    void stop();

    /// \brief Set output value range
    /// \param values_range - minimum and maximum values for the channels.
    /// \param default_values - default values for all channels
    void set_value_range(const SPIDAC_CHANNELS_VALUE_RANGE& values_range, const SPIDAC_SAMPLE_VECT& default_values);

    /// \brief Get output value range
    /// \return Value range per channels
    SPIDAC_CHANNELS_VALUE_RANGE get_value_range();

    /// \brief Upload prepared waveform into internal buffer
    /// \param data - two-dimensional array with data to be uploaded.
    /// \param def_vals - true if default values are set (just single value per channel is acceptable),
    ///                   false if signal samples are passed.
    void upload(const SPIDAC_CHANNELS data, bool def_vals);

    size_t get_max_samples_per_channel();

    /// \brief Returns configured number of bits per sample.
    /// \return Number of bits per sample
    size_t get_bits_per_sample() const;

    /// \brief Returns the number of channels configured.
    /// \return Number of channels.
    size_t get_channels_count() const;

    /// \brief Returns the number of frames per sample (for all channels)
    /// \return Number of frames per sample.
    size_t get_frames_per_sample() const;

    /// \brief Returns default value (for all channels) to be set when signal generation is stopped.
    /// \return Default value.
    SPIDAC_SAMPLE_VECT get_default_value() const;

    /// \brief Set default values (for all channels) to be set when signal generation is stopped.
    /// \param values - Reference to default values.
    void set_default_values(const SPIDAC_SAMPLE_VECT& values);

    /// \brief Returns information regarding current SPIDAC device status
    /// \return true - device is running (generating signal), false device is not running
    bool is_running();

    private:
        SPIDAC_SAMPLE_VECT default_value;
        SPIDAC_CHANNELS_VALUE_RANGE value_range;
        bool little_endian;

    /// \brief Appends frame with sample to the buffer
    /// \param value - sample value
    /// \param channel_index - channel index
    /// \param buffer - buffer to be appended with the frame
    /// \warning This method is reflection of the SPIDACCustomizer::get_frame_for_channel() from customizer part.
    ///          Implementation of these methods should match!
    void append_frame_with_sample(double value, size_t channel_index, std::vector<uint8_t>& buffer);

    /// \brief Uploads prepared buffer to the DAC device
    /// \param buffer - buffer to be uploaded.
    /// \param def_vals - true if default values are set (just single value per channel is acceptable),
    ///                   false if signal samples are passed.
    void upload_raw(const std::vector<uint8_t>& buffer, bool def_vals);

    /// \brief Transforms frame to confirm DAC requirements
    /// \param frame - pointer to the frame to be transformed.
    /// \param frame_size - size of the frame
    /// \return pointer to the transformed frame. Size of the frame remains the same.
    /// \note Transformation is made in place therefore \ref frame parameter is returned.
    uint8_t* re_align_frame(uint8_t* frame, size_t frame_size);

    /// \brief Checks the size of the container to confirm the number of channels configured.
    /// \param c - container to be checked
    template <class CONTAINER>
    void validate_channel_count(const CONTAINER& c) {
        const char* func_name = "SPIDACDev::validate_channel_count(template)";
        if (c.size()!=config->channel_count) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "Argument doesn't match to the configured number of channels");
        }
    }

    void validate_equal_sample_count(const SPIDAC_CHANNELS& c) {
        const char* func_name = "SPIDACDev::validate_equal_sample_count";
        validate_channel_count(c);
        size_t sample_count = c.at(0).size();
        for (size_t i=1; i<config->channel_count; i++) {
            if (c.at(i).size() != sample_count) {
                throw EKitException(func_name, EKIT_BAD_PARAM, "Channels has different number of samples");
            }
        }
    }

    template<class CONTAINER, class RANGE>
    void validate_values_are_in_range(const CONTAINER& channels, const RANGE& r) {
        const char* func_name = "SPIDACDev::validate_values_are_in_range(template)";
        validate_channel_count(channels);
        validate_channel_count(r);
        validate_equal_sample_count(channels);

        for (size_t ch = 0; ch<config->channel_count; ch++) {
            double min_val = r.at(ch).first;
            double max_val = r.at(ch).second;
            SPIDAC_SAMPLE_VECT samples = channels.at(ch);
            size_t sample_count = samples.size();

            for (size_t i=0; i<sample_count; i++) {
                double v = samples[i];
                if (v < min_val) {
                    throw EKitException(func_name, EKIT_BAD_PARAM, "Value is less then the minimal possible value.");
                }

                if (v > max_val) {
                    throw EKitException(func_name, EKIT_BAD_PARAM, "Value is higher then the maximum possible value.");
                }
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

    SPIDAC_CHANNELS values_to_channels(const SPIDAC_SAMPLE_VECT& values) {
        assert(values.size()==config->channel_count);
        SPIDAC_CHANNELS channels(config->channel_count);
        for (size_t i=0; i<config->channel_count; i++) {
            channels.at(i).push_back(values.at(i));
        }
        return channels;
    }
};

/// @}
