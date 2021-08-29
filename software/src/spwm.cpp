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
 *   \brief SPWMDev software implementation
 *   \author Oleh Sharuda
 */

#include "spwm.hpp"

#ifdef SPWM_DEVICE_ENABLED

constexpr SPWM_SW_DESCRIPTOR SPWMDev::spwm_description[];

SPWMDev::SPWMDev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
	clear_prev_data();
	set_pwm_freq(SPWM_DEFAULT_FREQ);
}

SPWMDev::~SPWMDev() {
}

size_t SPWMDev::get_channel_count() const {
    return SPWM_CHANNEL_COUNT;
}

const SPWM_SW_DESCRIPTOR* SPWMDev::get_channel_info(size_t channel_index) {
    static const char* const func_name = "SPWMDev::get_channel_info";
    if (channel_index>=SPWM_CHANNEL_COUNT) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "invalid pin index.");
    }

    return spwm_description + channel_index;
}

void SPWMDev::set(SPWM_STATE& state) {
	static const char* const func_name = "SPWMDev::set";
	PWM_ENTRY pwmdata[SPWM_MAX_PWM_ENTRIES_COUNT] = {0};

	std::map<uint16_t, PWM_ENTRY> pwmmap;
	uint8_t* data_ptr;
	size_t data_len;

	// Block bus and fill data
	{
		BusLocker blocker(bus, get_addr());	

		// state may specify some of the channels configured. Fill the reset of the channels.
		for (size_t i=0; i<SPWM_CHANNEL_COUNT; i++) {
			SPWM_STATE::const_iterator si = state.find(i);
			if (si==state.end()) {
				state[i]=prev_data[i];
			}
		}

		// prepare reversed map
		for (size_t i=0; i<SPWM_CHANNEL_COUNT; i++) {
			size_t port = spwm_description[i].port_index;
			size_t pinval = 1 << spwm_description[i].pin_number;
			uint16_t value = state[i];

			PWM_ENTRY entry = {value, // value
							  {0}};   // data
			entry.data[port] = pinval;

			auto ins = pwmmap.emplace(value, entry);
			if (!ins.second) {
				ins.first->second.data[port] |= pinval;
			}
		}

		// fill data
		PWM_ENTRY acc = {0, {0}};
		pwmdata[0] = acc;
		size_t pwmindx = 1;
		for (auto i=pwmmap.begin(); i!=pwmmap.end(); ++i) {
			uint32_t value = (static_cast<uint32_t>(i->first) * static_cast<uint32_t>(max_period)) / 0xFFFF;
			assert(value<=0xFFFF);
			pwmdata[pwmindx-1].n_periods = value - acc.n_periods;
			for (int j=0; j<SPWM_PORT_COUNT; j++) {
				acc.data[j] |= i->second.data[j];
				pwmdata[pwmindx].data[j] = acc.data[j];
			}
			acc.n_periods = value;
			pwmindx++;
		}

		pwmdata[pwmindx-1].n_periods = max_period - acc.n_periods;


		data_ptr = (uint8_t*)(pwmdata);
		data_len = sizeof(PWM_ENTRY)*pwmindx;
		if (pwmdata[0].n_periods == 0) {
			data_ptr = (uint8_t*)(pwmdata + 1);
			data_len -= sizeof(PWM_ENTRY);
		}

		if (pwmdata[pwmindx-1].n_periods == 0) {
			data_len -= sizeof(PWM_ENTRY);
		}

		// Write data
		EKIT_ERROR err = bus->write(data_ptr, data_len);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }

		for (size_t i=0; i<SPWM_CHANNEL_COUNT; i++) {
			prev_data[i] = state[i];
		}	   
		
	}
}

void SPWMDev::reset() {
    clear_prev_data();
	SPWM_STATE state;
	set(state);
}

void SPWMDev::clear_prev_data() {
	for (size_t i=0; i<SPWM_CHANNEL_COUNT; i++) {
	    prev_data[i] = spwm_description[i].def_val ? 0 : max_period;
	}	
}

std::string SPWMDev::get_dev_name() const {
    return SPWM_DEVICE_NAME;
}

void SPWMDev::set_pwm_freq(double freq) {
    static const char* const func_name = "SPWMDev::set_pwm_freq";
    double f_cnt = 72000000.0 / static_cast<double>(SPWM_PRESCALE_VALUE + 1);
    uint16_t  mp;

    if (freq*65536.0<f_cnt) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "freq is too low.");
    }

    mp = f_cnt / freq;

    if (mp<100) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "maximum period is too low, try to decrease prescaler value.");
    }

    max_period = mp;
    SPWM_STATE state;
    set(state); // recalculate
}

#endif
