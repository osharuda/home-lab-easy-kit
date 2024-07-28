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
 *   \brief ADC device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "adc_common.hpp"

/// \defgroup group_adc_dev ADCDev
/// \brief Analogue to Digital Converter support
/// @{
/// \page page_adc_dev
/// \tableofcontents
///
/// \section sect_adc_dev_01 Work with ADCDev
///
/// ADCDev functionality provides the following features:
/// - ADCDev may sample configured inputs by timer periodically. It may stop sampling by sampling some number of samples or
///   sample unlimited amount of samples.
/// - It is possible to set time period to wait between sampling channels or to sample inputs as quick as possible (this option
///   doesn't seem to be very practical because sampling period may not be known).
/// - ADCDev may return data as integers or double values that represent sampled value in volts.
/// - It is possible to get average value of sampled data.
/// - STM32F103x provide ability to sample \f$V_{intref}\f$ voltage, which is typically 1.2V. It can be used to get \f$V_{DDA}\f$
///   voltage, and use it to correct conversion from integer values to voltage.
///
/// Basic logic of ADCDev functionality work is shown on the following schema:
/// \image html adcdev_schema.png
/// \image latex adcdev_schema.eps
///
/// ACDDev can be used simply. Use the following pattern to sample data:
/// 1. Create ADCDev object
/// 2. Call ADCDev#start() method to start sampling. This method takes parameters which enough to estimate when data will be ready
///    or ADC circular buffer will be overflown.
/// 3. Wait for a data. It can be either straightforward call to std::this_thread::sleep_for() or more fascinating use of
///    std::async().
/// 4. Call one of the ADCDev#get() methods.
/// 5. If \f$V_{refint}\f$ is read by configuring "ADC_Channel_Vrefint", then it is possible to update vref voltage. This
///    make following conversions of sampled data to voltage more accurate by getting actual \f$V_{DDA}\f$ voltage. You
///    may do it with ADCDev#set_vref() method.
/// 6. Call ADCDev#stop() if you do not need sampling anymore. It will stop internal timer (if it didn't stop itself because
///    all requested samples was read. Optionally it is possible to reset data accumulated in circular buffer.
///

/// \class ADCDev
/// \brief ADCDev implementation. Use this class in order to control ADCDev virtual devices.
class ADCDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
    typedef EKitVirtualDevice super;

    public:

    /// \brief Pointer to the #tag_PADCConfig structure that describes ADCDev virtual device represented by this class.
    const ADCConfig* config;

    /// \brief No default constructor
    ADCDev() = delete;

    /// \brief Copy construction is forbidden
    ADCDev(const ADCDev&) = delete;

    /// \brief Assignment is forbidden
    ADCDev& operator=(const ADCDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS.
    /// \param config - actual configuration of the device taken from generated configuration library.
    ADCDev(std::shared_ptr<EKitBus>& ebus, const ADCConfig* config);

    /// \brief Destructor (virtual)
    ~ADCDev() override;

    /// \brief Start sampling
    /// \param sample_count - amount of samples required. If zero - number of samples is unlimited.
    void start(uint16_t sample_count);

    /// \brief Stops sampling.
    /// \note If ADCDev is stopped during sampling ADC related peripherals are re-initialized.
    void stop();

    /// \brief Clears all the data accumulated inside ADCDev circular buffer.
    /// \note This method may be called when sampling is active.
    void reset();

    /// \brief Configure and prepare device for the sampling.
    /// \param delay_sec - number of micro seconds between samples. If 0 is
    ///        passed conversion will follow each other without delay.
    /// \param average_samples - number of samples to be averaged.
    /// \param sampling - std::map with sampling information. key is a channel
    ///        index, value is one of the \ref ADC_SampleTime_1Cycles5 "ADC sample time constants".
    /// \note This call will stop active sampling and re-initialize ADC related
    ///       peripherals. Use this call to reinitialize ADC related peripherals.
    void configure(double delay_sec, size_t average_samples, std::map<size_t, uint8_t>& sampling);

    /// \brief Get current device status
    /// \param flags - output value to return current device state. It is described by ADCDEV_STATUS_XXX constants.
    /// \return Number of samples stored in a buffer.
    size_t status(uint16_t& flags);

    /// \brief Read samples accumulated in circular buffer as double, normalized by vref JSON configuration parameter.
    /// \param values - measured data represented by vector (samples) of vector (channels). Values are doubles, represent
    ///        calculated with vref.
    void get(std::vector<std::vector<double>>& values);

    /// \brief Returns input name from input index.
    /// \param index - input index.
    /// \param channel_name - set to true to get ADC channel name (ADC_Channel_xxx) or false to get input name
    /// \return string with name
    std::string get_input_name(size_t index, bool channel_name) const;

    /// \brief  Returns inputs (channels) count
    /// \return number of channels
    size_t get_input_count() const;

   private:

    /// \brief Get current device status (private implementation)
    /// \param flags - optional output value to return current device state. It is described by ADCDEV_STATUS_XXX constants.
    ///                If not required, pass nullptr.
    /// \return Number of bytes accumulated in circular buffer (including status).
    size_t status_priv(uint16_t* flags, EKitTimeout& to);

    void send_command(uint8_t* ptr, size_t size, uint8_t command);
    std::vector<std::pair<double, double>> signal_ranges;
    std::vector<uint16_t> data_buffer;
    volatile uint16_t* data_status;
    volatile uint16_t* data;
};

/// @}
