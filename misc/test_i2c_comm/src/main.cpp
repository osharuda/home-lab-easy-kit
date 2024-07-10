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
 *   \brief Example project skeleton
 *   \author Oleh Sharuda
 */

#include "main.hpp"
#include <iostream>

#include <libhlek/info_dev.hpp>
#include <libhlek/ekit_i2c_bus.hpp>
#include <libhlek/ekit_firmware.hpp>
#include <libhlek/texttools.hpp>

#include <libhlek/spwm.hpp>
#include <libhlek/timetrackerdev.hpp>
#include <libhlek/adcdev.hpp>

#include <libtestbench/info_conf.hpp>
#include <libtestbench/spwm_conf.hpp>
#include <libtestbench/adc_conf.hpp>
#include <libtestbench/timetrackerdev_conf.hpp>

#include <atomic>

void help() {
    std::cout << "Usage: example </dev/i2c-X> <duration minutes>" << std::endl;
}

std::atomic_bool g_exit(false);

void adc_thread_func(std::shared_ptr<ADCDev> adc) {
    std::map<size_t, uint8_t> sampling_info;
    sampling_info[0]=ADC_SampleTime_28Cycles5;
    std::vector<std::vector<double>> samples;

    while (!g_exit.load()) {
        adc.reset();
        adc->configure(1.0e3L, 5, sampling_info);
        adc->start(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        adc->get(samples);
        adc->stop();
        samples.clear();
    }
}

void time_tracker_thread_func(std::shared_ptr<TimeTrackerDev> tt) {
    std::vector<double> timestamps;
    while (!g_exit.load()) {
        tt->start(true);

        // SPWM is set to 10 KHZ, so we would have 200 events in 100ms
        std::this_thread::sleep_for(std::chrono::seconds(1));

        tt->read_all(timestamps, true);
        tt->stop();
        timestamps.clear();
    }
}

void info_checker_thread_func(std::shared_ptr<INFODev> info) {
    while (!g_exit.load()) {
        info->check();
    }
}

int main(int argc, char* argv[]) {
    const char* func_name = __FUNCTION__;
    std::vector<double> ts;
    try {
        if (argc != 2) {
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Wrong number of arguments");
        }

        size_t pos = 0;

        size_t duration_min = std::stoul(argv[2], &pos);
        if (argv[2][pos] != 0) {
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Bad duration value");
        }

        std::cout << "*** I2C Test utility ***" << std::endl;

        // Open I2C bus
        const char* i2c_dev = argv[1];
        std::shared_ptr<EKitBus> i2cbus(new EKitI2CBus(i2c_dev));
        EKitTimeout time_out(0);
        EKIT_ERROR err = i2cbus->open(time_out);
        if (err != EKIT_OK) {
            throw EKitException(__FUNCTION__, EKIT_FAIL, tools::format_string("Failed to open %s", i2c_dev));
        }

        // Open firmware protocol for AD9850 (via I2C) and create devices
        std::shared_ptr<EKitBus> fw (new EKitFirmware(i2cbus, testbench::INFO_I2C_ADDRESS));
        std::shared_ptr<INFODev> info(new INFODev(fw, testbench::info_config_ptr));
        std::shared_ptr<ADCDev> adc(new ADCDev(fw, testbench::adc_adc_dma_config));
        std::shared_ptr<SPWMDev> spwm(new SPWMDev(fw, testbench::spwm_config_ptr));
        std::shared_ptr<TimeTrackerDev> tt(new TimeTrackerDev(fw, testbench::timetrackerdev_timetrackerdev_0_config_ptr));

        // Switch spwm at 10KHz
        spwm->reset();
        SPWM_STATE state;
        state[testbench::SPWM_PWM] = 0xFFFF / 2;
        spwm->set(state);
        spwm->set_pwm_freq(1.0e5);


        // Start two threads reading data from ADC and time tracker devices
        std::thread adc_thread(adc_thread_func, adc);
        std::thread tt_thread(time_tracker_thread_func, tt);
        std::thread info_thread(info_checker_thread_func, info);

        std::this_thread::sleep_for(std::chrono::minutes(duration_min));

        g_exit.store(true);

        adc_thread.join();
        tt_thread.join();
        info_thread.join();

        // Turn off spwm
        state[testbench::SPWM_PWM] = 0;
        spwm->set(state);

        std::cout << "[OK]" << std::endl;
    } catch (EKitException e) {
        std::cout << e.what() << std::endl;

        // Specific error processing
        switch (e.ekit_error) {
            case EKIT_BAD_PARAM:
                help();
                break;
        };
    }
}
