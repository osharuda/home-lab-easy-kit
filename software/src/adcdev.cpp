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

#ifdef ADCDEV_DEVICE_ENABLED

/// \addtogroup group_adc_dev
/// @{

ADCDEV_SW_CHANNELS

/// \brief Global array that stores all virtual ADCDev devices configurations.
const ADCDevInstance g_adcdev_descriptors[] = ADCDEV_SW_DEV_DESCRIPTOR;
/// @}

const ADCDevInstance* ADCDev::get_descriptor(size_t index) {
	if (index<ADCDEV_DEVICE_COUNT) {
		return const_cast<ADCDevInstance*>(g_adcdev_descriptors + index);
	} else {
		assert(false);
	}

	return nullptr;
}

std::string ADCDev::get_dev_name() const {
	return descr->dev_name;
}

ADCDev::ADCDev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
	static const char* const func_name = "ADCDev::ADCDev";
	for (int i=0; i<ADCDEV_DEVICE_COUNT; i++) {
		if (addr==g_adcdev_descriptors[i].dev_id) {
			descr = g_adcdev_descriptors + i;
			break;
		}
	}

	if (descr==nullptr) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "addr specified doesn't correspond to any of ADC devices");
	}

	vref_cur = descr->vref;
}

ADCDev::~ADCDev() {
}

std::string ADCDev::get_input_name(size_t index, bool channel_name) const {
	static const char* const func_name = "ADCDev::get_input_name";
	if (index>=descr->input_count) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "ADC input index is out of range");
	}

	return channel_name ? descr->inputs[index].adc_input : descr->inputs[index].in_name;
}

size_t ADCDev::get_input_count() const {
	return descr->input_count;
}


void ADCDev::start(uint16_t sample_count, double delay_sec){
	static const char* const func_name = "ADCDev::start";
	ADCDevCommand data;
	double expected;
	uint8_t f = 0;

	data.sample_count = sample_count;

	// figure out period and prescaller
	if (delay_sec==0) {
		data.timer_prescaller=0;
		data.timer_period=0;
	} else {
		int res = tools::stm32_timer_params(descr->timer_freq, delay_sec, data.timer_prescaller, data.timer_period, expected);
		if (res>0) {
			throw EKitException(func_name, EKIT_BAD_PARAM, "delay_sec is too long");
		} else if (res<0) {
			throw EKitException(func_name, EKIT_BAD_PARAM, "delay_sec is too short");
		}
	}

	if (sample_count==0) {
		f |= ADCDEV_UNSTOPPABLE;
	}


	// issue a command
	{
		BusLocker blocker(bus, get_addr());	

		EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, f);
	    if (err != EKIT_OK) {
	    	throw EKitException(func_name, err, "set_opt() failed");
	    }

		// Write data
		err = bus->write((uint8_t*)&data, sizeof(ADCDevCommand));
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }
	}
}

void ADCDev::stop(bool reset_buffer){
	static const char* const func_name = "ADCDev::stop";
	uint8_t f = 0;

	if (reset_buffer) {
		f |= ADCDEV_RESET_DATA;
	}

	{
		BusLocker blocker(bus, get_addr());	

		EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, f);
	    if (err != EKIT_OK) {
	    	throw EKitException(func_name, err, "set_opt() failed");
	    }

		// Write zero-length buffer (just command byte, this will cause sampling stop)
		err = bus->write(nullptr, 0);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }
	}
}

void ADCDev::get(std::vector<uint16_t>& data, bool& ovf){
	static const char* const func_name = "ADCDev::get(1)";
	// issue a command
	{
		BusLocker blocker(bus, get_addr());	

		// get amount of data
		CommResponseHeader hdr;
		EKIT_ERROR err = std::dynamic_pointer_cast<EKitFirmware>(bus)->get_status(hdr, false);
	    if (err != EKIT_OK && err != EKIT_OVERFLOW ) {
	        throw EKitException(func_name, err, "get_status() failed");
	    }

	    // check for overflow
	    ovf = (hdr.comm_status & COMM_STATUS_OVF) != 0;

	    if (hdr.length == 0)
	    	goto done;

	    // sanity check, buffer must be aligned by size of all inputs
	    if ((hdr.length % (descr->input_count*sizeof(uint16_t))) != 0) {
	    	assert(false);
	    	goto done;
	    }

	    // resize buffer
	    data.resize(hdr.length / sizeof(uint16_t));

		// read data
		err = bus->read((uint8_t*)data.data(), hdr.length);
		if (err != EKIT_OK) {
		    throw EKitException(func_name, err, "read() failed");
		}
	}

done:
	return;
}

// returns all samples converted to double
void ADCDev::get(std::vector<std::vector<double>>& values, bool& ovf){
	static const char* const func_name = "ADCDev::get(2)";
	std::vector<uint16_t> data;
	get(data, ovf);
	size_t sample_count = data.size() / descr->input_count;	

	values.clear();

	// convert into doubles
	for (size_t ch=0; ch<descr->input_count; ch++) {
		std::vector<double> ch_val;

		for (int s = 0; s<sample_count; s++) {
			uint16_t v = data.at(ch + s*descr->input_count);
			double x = vref_cur*((double)v/(double)descr->adc_maxval);
			ch_val.push_back(x);
		}

		values.push_back(std::move(ch_val));
	}
}

// returns average for all samples converted to double
void ADCDev::get(std::vector<double>& values, bool& ovf){
	static const char* const func_name = "ADCDev::get(3)";
	std::vector<uint16_t> data;
	get(data, ovf);
	size_t sample_count = data.size() / descr->input_count;	

	values.clear();

	// convert into doubles
	for (size_t ch=0; ch<descr->input_count; ch++) {
		uint32_t acc = 0;
		for (int s = 0; s<sample_count; s++) {
			acc += data.at(ch + s*descr->input_count);
		}

		acc = acc / sample_count;
		double x = vref_cur*((double)acc/(double)descr->adc_maxval);
		values.push_back(x);
	}
}

void ADCDev::set_vref(double Vref_plus) {
    vref_cur = Vref_plus;
}
void ADCDev::set_vref(uint16_t vref_channel, double V_ref_int) {
    vref_cur = V_ref_int * (double)descr->adc_maxval / (double)vref_channel;
}

#endif