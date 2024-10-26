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
 *   \brief PaceMakerDev device software implementation
 *   \author Oleh Sharuda
 */

#include "pacemakerdev_common.hpp"
#include "pacemakerdev.hpp"
#include "ekit_firmware.hpp"
#include <math.h>

PaceMakerDev::PaceMakerDev(std::shared_ptr<EKitBus>& ebus, const PaceMakerDevConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg),
    all_signals( (1 << cfg->signals_number) - 1) {
    reset_signals();
}

PaceMakerDev::~PaceMakerDev() {
}

void PaceMakerDev::start(double frequency, size_t repeat_count) {
    const char* func_name = "PaceMakerDev::start";
    PaceMakerStartCommand buffer;

    if (frequency <= 0.0L) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Frequency should not be negative or zero.");
    }

    if (frequency > max_main_freq ) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Frequency is to high.");
    }

    buffer.main_cycles_number = repeat_count;
    double eff_main_freq = 0.0L;
    tools::stm32_timer_params(config->main_timer_freq, 1.0L/frequency, buffer.main_prescaller, buffer.main_counter, eff_main_freq);

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, PACEMAKERDEV_START, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(&buffer, sizeof(buffer), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void PaceMakerDev::stop() {
    const char* func_name = "PaceMakerDev::stop";

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, PACEMAKERDEV_STOP, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(nullptr, 0, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void PaceMakerDev::reset() {
    const char* func_name = "PaceMakerDev::reset";
    reset_signals();
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, PACEMAKERDEV_RESET, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(nullptr, 0, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void PaceMakerDev::status(PaceMakerStatus& s) {
    const char* func_name = "PaceMakerDev::status";
    CommResponseHeader hdr;
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    EKIT_ERROR err = std::dynamic_pointer_cast<EKitFirmware>(bus)->sync_vdev(hdr, false, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "sync_vdev() failed");
    }

    err = bus->read(&s, sizeof(PaceMakerStatus), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "status read failed.");
    }
}

void PaceMakerDev::set_data() {
    const char* func_name = "PaceMakerDev::set_data";
    size_t transit_count = signals.size();
    size_t buffer_size = sizeof(PaceMakerDevData) + transit_count*sizeof(PaceMakerTransition);
    std::unique_ptr<uint8_t[]> data(new uint8_t[buffer_size]);
    PaceMakerDevData* trans_data = (PaceMakerDevData*)data.get();
    trans_data->transition_number = transit_count;
    auto current_signal = signals.begin();
    for (size_t i=0; i<transit_count; i++) {
        const PaceMakerSignalTransit& st = *current_signal;
        current_signal++;
        PaceMakerTransition* td = trans_data->transitions + i;
/*
        if ( i == (transit_count-1)) {
            // The last signal, timer is ignored
            td->prescaller = 0;
            td->counter = 0;
        } else {
        */
            if (st.next_delay <= min_internal_delay) {
                throw EKitException(func_name, EKIT_BAD_PARAM, "Delay is too small.");
            }

            if (st.next_delay > max_internal_delay) {
                throw EKitException(func_name, EKIT_BAD_PARAM, "Delay is too high.");
            }
            double effective_frequency;
            tools::stm32_timer_params(config->internal_timer_freq, st.next_delay, td->prescaller, td->counter,
                                      effective_frequency);
            /*
        }*/
        td->signal_mask = st.signal;
    }

    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, PACEMAKERDEV_DATA, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(trans_data, buffer_size, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void PaceMakerDev::reset_signals() {
    current_signal = config->default_signals;
    signals.clear();
}

void PaceMakerDev::add_set(double offset, uint32_t signal_value){
    const char* func_name = "PaceMakerDev::add_set";
    assert(offset >= 0.0L);
    assert(signal_value <= all_signals);

    // TEST CODE - TO BE REMOVED
    /*
    if (signals.empty() && offset > 0.0L) {
        // insert empty signal with default values
        signals.push_back(PaceMakerSignalTransit {config->default_signals, offset});
    }
    signals.back().next_delay = offset;
     */

    // Append signal
    signals.push_back({signal_value, offset});
    current_signal = signal_value;
}

void PaceMakerDev::add_flip(double offset, uint32_t affected_signals) {
    const char* func_name = "PaceMakerDev::add_flip";
    assert(offset >= 0.0L);
    assert(affected_signals <= all_signals);

    uint32_t flipped_bits = (~(current_signal & affected_signals)) & affected_signals;
    uint32_t other_bits = current_signal & (~affected_signals);

    add_set(offset, flipped_bits | other_bits);
}

void PaceMakerDev::add_pulse(double offset, double period, uint32_t affected_signals) {
    const char* func_name = "PaceMakerDev::add_pulse";
    assert(offset >= 0.0L);
    assert(affected_signals <= all_signals);
    assert(period > 0.0L);

    add_flip(offset, affected_signals);
    add_flip(period, affected_signals);
}

void PaceMakerDev::add_pwm(double offset, double period, double pwm_value, size_t count, uint32_t affected_signals) {
    const char* func_name = "PaceMakerDev::add_pwm";
    assert(offset >= 0.0L);
    assert(affected_signals <= all_signals);
    assert(period > 0.0L);
    assert(pwm_value >= 0.0L);
    assert(pwm_value <= 1.0L);

    double flip_state_period = period*pwm_value;
    double original_state_period = period*(1.0L - pwm_value);

    add_flip(offset, 0);
    for (size_t i=0; i<count; i++) {
        add_flip(original_state_period, affected_signals);
        add_flip(flip_state_period, affected_signals);
    }
}

void PaceMakerDev::add_clock(double offset, double period, size_t count, uint32_t affected_signals) {
    add_pwm(offset, period, 0.5, count, affected_signals);
}

void PaceMakerDev::add_default(double offset) {
    add_set(offset, config->default_signals);
}

uint32_t PaceMakerDev::all_signals_mask() const {
    return all_signals;
}

