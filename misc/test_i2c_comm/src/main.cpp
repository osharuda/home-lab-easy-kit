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
#include <iomanip>

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

void print_result(  const std::string& name,
                    size_t n_smpl,
                    double mean_value,
                    double min_value,
                    double max_value,
                    char var_letter,
                    const std::string& unit) {
    if (n_smpl) {
        std::cout << std::setw(5) << "[" << name << ": Nₛₐₘₚₗₑ=" << n_smpl << "] " << std::setw(10)
                  << var_letter << "ₘₑₐₙ =" << mean_value << unit << "  "
                  << var_letter << "ₘᵢₙ  =" << min_value << unit << "  "
                  << var_letter << "ₘₐₓ  =" << max_value << unit << std::endl;
    } else {
        std::cout << std::setw(5) << "[" << name << ": NO DATA]" << std::endl;
    }
}

void adc_thread_func(std::shared_ptr<ADCDev> adc) {
    constexpr double mean_value = 3.3l / 2.0l;
    constexpr double value_error = 0.5l / 2.0l;
    constexpr double min_val = mean_value - value_error;
    constexpr double max_val = mean_value + value_error;

    std::map<size_t, uint8_t> sampling_info;
    sampling_info[0]=ADC_SampleTime_28Cycles5;
    std::vector<std::vector<double>> samples;


    while (!g_exit.load()) {
        adc->stop();
        adc->reset();
        adc->configure(0.001, 1, sampling_info);
        adc->start(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        adc->get(samples);
        adc->stop();

        // Check all values for reasonable value
        size_t n = samples.size();

        if (n==0) {
            print_result("ADCDev", n, 0.0L, 0.0L, 0.0L, 'U', "V");
        } else {
            double acc = 0.0l;
            double min_x = std::numeric_limits<double>::max();
            double max_x = 0.0l;

            for (auto v: samples) {
                double x = v[0];
                min_x = std::min(x, min_x);
                max_x = std::max(x, max_x);
                assert( x >= min_val);
                assert( x <= max_val);
                acc += x / (double)n;
            }

            //std::cout << "[ADCDev] Uₘₑₐₙ=" << acc << " N=" << n << " Uₘᵢₙ=" << min_x << " Uₘₐₓ=" << max_x << std::endl;
            print_result("ADCDev      ", n, acc, min_x, max_x, 'U', "V");
        }


        samples.clear();
    }
}

void time_tracker_thread_func(std::shared_ptr<TimeTrackerDev> tt) {

    constexpr double frequency = 1.0e4l; // 10 KHz
    constexpr double mean_value = 1.0l / (2 * frequency);
    constexpr double value_error = mean_value * 0.7l;
    constexpr double min_val = mean_value - value_error;
    constexpr double max_val = mean_value + value_error*5.0l;
    bool running;
    uint64_t first_ts = 0;

    std::vector<double> timestamps;
    while (!g_exit.load()) {
        tt->stop();
        tt->reset();
        tt->start();

        // SPWM is set to 10 KHZ, so we would have 200 events in 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds (100));

        uint64_t t;
        tt->get_status(running, t);
        assert(first_ts < t);
        first_ts = t;
        tt->read_all(timestamps, true);
        tt->stop();

        // Check all timestamps: all timestamp should go with reasonable distance between values
        double last_value;
        double acc = 0.0l;
        size_t n = timestamps.size();

        if (n > 1) {
            double min_hp = std::numeric_limits<double>::max();
            double max_hp = 0.0l;
            auto ts = timestamps.begin();
            last_value = *ts;

            for (++ts; ts!=timestamps.end(); ++ts) {
                double value = *ts;
                double half_period = value - last_value;
                max_hp = std::max(half_period, max_hp);
                min_hp = std::min(half_period, min_hp);

                if (half_period < min_val) {
                    std::cout << "[TimeTrackDev] Warning, received value is too small: half_period=" << half_period
                              << " , while minimum allowed value is " << min_val << std::endl;

                    assert(0);
                }

                if (half_period > max_val) {
                    std::cout << "[TimeTrackDev] Warning, received value is too long: half_period=" << half_period
                              << " , while maximum allowed value is " << max_val << std::endl;
                    assert(0);
                }

                acc += half_period / static_cast<double>(n-1);
                last_value = *ts;
            }

            print_result("TimeTrackDev", n, acc, min_hp, max_hp, 'T', "sec");
            //std::cout << "[TimeTrackDev: Nₛₐₘₚₗₑ=" << n << "] Uₘₑₐₙ=" << std::setw(10) << acc << " N=" << n << " Tₘᵢₙ=" << min_hp << " Tₘₐₓ=" << max_hp << std::endl;
        } else {
            print_result("TimeTrackDev", 0, 0.0L, 0.0L, 0.0L, 'T', "sec");
        }
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
        if (argc != 3 ) {
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
        info->check();
        std::cout << "InfoDev - [OK]" << std::endl;

        std::shared_ptr<ADCDev> adc(new ADCDev(fw, testbench::adc_adc_dma_config));
        adc->set_crc_callback([](){
            std::cout << "ADCDev: Overflow" << std::endl;
            return EKIT_OK;
        });

        adc->set_fail_callback([](){
            std::cout << "ADCDev: Failed command" << std::endl;
            return EKIT_COMMAND_FAILED;
        });


        std::shared_ptr<SPWMDev> spwm(new SPWMDev(fw, testbench::spwm_config_ptr));
        std::shared_ptr<TimeTrackerDev> tt(new TimeTrackerDev(fw, testbench::timetrackerdev_timetrackerdev_0_config_ptr));



        // Switch spwm at 10KHz
        spwm->reset();
        SPWM_STATE state;
        spwm->set_pwm_freq(1.0e4);
        state[testbench::SPWM_PWM] = 0xFFFF / 2;
        spwm->set(state);



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
