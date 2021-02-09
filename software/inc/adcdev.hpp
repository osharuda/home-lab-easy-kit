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
#include "sw.h"

#ifdef ADCDEV_DEVICE_ENABLED

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

	/// \brief Pointer to the #tag_ADCDevInstance structure that describes ADCDev virtual device represented by this class.
	const ADCDevInstance* descr = nullptr;

	/// \brief
	double vref_cur;

	public:

    /// \brief No default constructor
    ADCDev() = delete;

    /// \brief Copy construction is forbidden
    ADCDev(const ADCDev&) = delete;

    /// \brief Assignment is forbidden
    ADCDev& operator=(const ADCDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the ACDDev device.
	ADCDev(std::shared_ptr<EKitBus>& ebus, int addr);

    /// \brief Destructor (virtual)
	~ADCDev() override;

	/// \brief Start sampling
	/// \param sample_count - amount of samples required. If zero - number of samples is unlimited.
	/// \param delay_sec - number of micro seconds between samples. If 0 is passed conversion will follow each other without
	///        delay.
	void start(uint16_t sample_count, double delay_sec);

	/// \brief Stops sampling
	/// \param reset_buffer - true to discard all the data accumulated in ADCDev circular buffer
	void stop(bool reset_buffer);

	/// \brief Read samples accumulated in circular buffer as uint16_t.
	/// \param data - reference to std::vector to be filled with samples. Samples will appear as two-dimensional array
	///        data[s][c], where inner index c is for channels, s for sample. Return array may be empty, if no data available.
	///        Length of the data (in elements) is multiple to amount of channels.
	/// \param ovf - output parameter to notify if ADCDev circular buffer was overflown.
	void get(std::vector<uint16_t>& data, bool& ovf);

	/// \brief Read samples accumulated in circular buffer as double, normalized by vref JSON configuration parameter.
	/// \param values - measured data represented by vector (samples) of vector (channels). Values are doubles, represent
	///        calculated with vref.
    /// \param ovf - output parameter to notify if ADCDev circular buffer was overflown.
	void get(std::vector<std::vector<double>>& values, bool& ovf);

	/// \brief Read samples and return average value for all samples
	/// \param values - measured data represented by array of values (channels).
    /// \param ovf - output parameter to notify if ADCDev circular buffer was overflown.
	void get(std::vector<double>& values, bool& ovf);

	/// \brief Returns ADCDev descriptor
	/// \param index - ADCDev index
	/// \return ADCDev descriptor
	static const ADCDevInstance* get_descriptor(size_t index);

	/// \brief Returns input name from input index.
	/// \param index - input index.
	/// \param channel_name - set to true to get ADC channel name (ADC_Channel_xxx) or false to get input name
	/// \return string with name
	std::string get_input_name(size_t index, bool channel_name) const;

	/// \brief  Returns inputs (channels) count
	/// \return number of channels
	size_t get_input_count() const;

	/// \brief Returns device name as specified in JSON configuration file
	/// \return string with name
	std::string get_dev_name() const override;

	/// \brief Set current vref value to Vref+
	/// \param Vref_plus - Vref+ voltage.
	void set_vref(double Vref_plus);

	/// \brief Set current vref value based on internal reference voltage and it's measured value
	/// \param vref_channel - measured value of ADC_Channel_Vrefint channel
	/// \param V_ref_int - Reference internal voltage (typical value is 1.2V, for details take a look into MCU documentation)
	void set_vref(uint16_t vref_channel, double V_ref_int = 1.2);
};

/// @}

#endif
