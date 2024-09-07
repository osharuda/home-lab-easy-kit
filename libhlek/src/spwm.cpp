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
    clear_prev_data();
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
    const size_t pwm_size = PWM_ENTRY_SIZE(config->port_number);
    size_t buffer_size = max_entry_count * pwm_size;
    uint8_t pwmdata_buffer[buffer_size];

    PWM_ENTRY_HELPER pwmdata[max_entry_count];
    for (size_t i=0; i<sizeof(pwmdata)/sizeof(PWM_ENTRY_HELPER); i++) {
        pwmdata[i].assign(pwmdata_buffer + i*pwm_size, config->port_number);
    }

    uint8_t* data_ptr;
    size_t data_len;

    // Block bus and fill data
    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        // State may specify some channels configured. Fill the reset channels with values from prev_data map.
        for (size_t i=0; i<config->channel_count; i++) {
            SPWM_STATE::const_iterator si = state.find(i);
            if (si==state.cend()) {
                state[i]=prev_data[i];
            }
        }

        // Prepare reversed map, where keys are pwm values, and values are PWM_ENTRY_HELPER
        // Also, std::map ensures the values will be enumerated in sorted order.
        std::map<uint16_t, PWM_ENTRY_HELPER> pwmmap;
        for (size_t i=0; i<config->channel_count; i++) {
            size_t port = config->channels[i].port_index;
            size_t pinval = 1 << config->channels[i].pin_number;
            uint16_t value = state[i];

            auto ins = pwmmap.emplace(value, PWM_ENTRY_HELPER(config->port_number));
            struct PWM_ENTRY* entry = ins.first->second.data();
            if (ins.second) {
                entry->n_periods = value;
            }
            entry->data[port] |= pinval;
        }

        size_t pwmindx = 1;
        struct PWM_ENTRY* prev_entry;
        struct PWM_ENTRY*  dst_entry;

        PWM_ENTRY_HELPER accumulator(config->port_number);
        struct PWM_ENTRY* acc_entry = accumulator.data();

        for (auto i=pwmmap.begin(); i!=pwmmap.end(); ++i) {
            struct PWM_ENTRY* pwmmap_entry = i->second.data();
            prev_entry = pwmdata[pwmindx-1].data();

            uint32_t value = (static_cast<uint32_t>(i->first) * static_cast<uint32_t>(max_period)) / 0xFFFF;
            assert(value<=0xFFFF);

            prev_entry->n_periods = value - acc_entry->n_periods;
            acc_entry->n_periods = value;
            dst_entry = pwmdata[pwmindx].data();

            for (int j=0; j<config->port_number; j++) {
                uint16_t port_value = acc_entry->data[j] | pwmmap_entry->data[j];
                dst_entry->data[j] = port_value;
                acc_entry->data[j] = port_value;
            }
            pwmindx++;
        }

        dst_entry->n_periods = max_period - acc_entry->n_periods;

        data_ptr = (uint8_t*)(pwmdata_buffer);
        data_len = pwm_size*pwmindx;

        // Do not emmit first frame if some channel has 100% value
        if (pwmdata[0].data()->n_periods == 0) {
            data_ptr += pwm_size;
            data_len -= pwm_size;
        }

        // Do not emmit the last frame if the last channel has 100% value
        if (dst_entry->n_periods == 0) {
            data_len -= pwm_size;
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
    prev_data.resize(config->channel_count);
    for (size_t i=0; i<config->channel_count; i++) {
        prev_data[i] = config->channels[i].def_val ? 0 : max_period;
    }    
}

void SPWMDev::set_pwm_freq(double freq) {
    static const char* const func_name = "SPWMDev::set_pwm_freq";
    double f_cnt = 72000000.0 / static_cast<double>(config->prescaller + 1);
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
