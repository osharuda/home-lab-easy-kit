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
#include <string.h>

SPWMDev::SPWMDev(std::shared_ptr<EKitBus>& ebus, const SPWMConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config (cfg) {
}

size_t SPWMDev::get_channel_count() const {
    return config->channel_count;
}

const SPWMChannel* SPWMDev::get_channel_info(size_t channel_index) {
    static const char* const func_name = "SPWMDev::get_channel_info";
    if (channel_index>=config->channel_count) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "invalid pin index.");
    }

    return config->channels + channel_index;
}

void SPWMDev::set(SPWM_STATE& state) {
	static const char* const func_name = "SPWMDev::set";
    size_t max_entry_count = config->channel_count + 1;
    size_t pwm_size = PWM_ENTRY_SIZE(config->port_number);
    size_t buffer_size = max_entry_count * pwm_size;
    uint8_t pwmdata_buffer[buffer_size];

    PWM_ENTRY_HELPER pwmdata[max_entry_count];
    for (size_t i=0; i<sizeof(pwmdata)/sizeof(PWM_ENTRY_HELPER); i++) {
        pwmdata[i].assign(pwmdata_buffer + i*pwm_size, config->port_number);
    }

	std::map<uint16_t, PWM_ENTRY_HELPER> pwmmap;
	uint8_t* data_ptr;
	size_t data_len;

	// Block bus and fill data
	{
        EKitTimeout to(get_timeout());
		BusLocker blocker(bus, to);

		// state may specify some of the channels configured. Fill the reset of the channels.
		for (size_t i=0; i<config->channel_count; i++) {
			SPWM_STATE::const_iterator si = state.find(i);
			if (si==state.end()) {
				state[i]=prev_data[i];
			}
		}

		// prepare reversed map
		for (size_t i=0; i<config->channel_count; i++) {
			size_t port = config->channels[i].port_index;
			size_t pinval = 1 << config->channels[i].pin_number;
			uint16_t value = state[i];

			auto ins = pwmmap.emplace(value, PWM_ENTRY_HELPER());
            PPWM_ENTRY entry = ins.first->second.data();
			if (ins.second) {
                entry->n_periods = value;
    		}
            entry->data[port] |= pinval;
		}

		// fill data
        PPWM_ENTRY dst_entry = nullptr;
        PPWM_ENTRY next_dst_entry = nullptr;
		PPWM_ENTRY acc = pwmdata[0].data(); // First element is used as accumulator
		size_t pwmindx = 1;
		for (auto i=pwmmap.begin(); i!=pwmmap.end(); ++i) {
			uint32_t value = (static_cast<uint32_t>(i->first) * static_cast<uint32_t>(max_period)) / 0xFFFF;
			assert(value<=0xFFFF);
            dst_entry = pwmdata[pwmindx-1].data();
            dst_entry->n_periods = value - acc->n_periods;
            next_dst_entry = pwmdata[pwmindx].data();

            PPWM_ENTRY entry = i->second.data();
			for (int j=0; j<config->port_number; j++) {
				acc->data[j] |= entry->data[j];
                next_dst_entry->data[j] = acc->data[j];
			}
			acc->n_periods = value;
			pwmindx++;
		}

        next_dst_entry->n_periods = max_period - acc->n_periods;


		data_ptr = (uint8_t*)(pwmdata_buffer);
		data_len = sizeof(PWM_ENTRY)*pwmindx;
		if (acc->n_periods == 0) {
			data_ptr = (uint8_t*)(pwmdata + 1);
			data_len -= sizeof(PWM_ENTRY);
		}

		if (next_dst_entry->n_periods == 0) {
			data_len -= sizeof(PWM_ENTRY);
		}

		// Write data
		EKIT_ERROR err = bus->write(data_ptr, data_len, to);
	    if (err != EKIT_OK) {
	        throw EKitException(func_name, err, "write() failed");
	    }

		for (size_t i=0; i<config->channel_count; i++) {
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
	for (size_t i=0; i<config->channel_count; i++) {
	    prev_data[i] = config->channels[i].def_val ? 0 : max_period;
	}	
}

void SPWMDev::set_pwm_freq(double freq) {
    static const char* const func_name = "SPWMDev::set_pwm_freq";
    double f_cnt = 72000000.0 / static_cast<double>(config->prescaller + 1); // <CHECKIT> Remove constants here and everywhere
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
