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
#include <librowland_scan/info_conf.hpp>
#include <librowland_scan/step_motor_conf.hpp>
#include <librowland_scan/adc_conf.hpp>
#include <libhlek/info_dev.hpp>
#include <libhlek/step_motor.hpp>
#include <libhlek/adcdev.hpp>
#include <libhlek/ekit_i2c_bus.hpp>
#include <libhlek/ekit_firmware.hpp>
#include <libhlek/texttools.hpp>
#include <thread>
#include <chrono>

void help() {
    std::cout << "Usage: example </dev/i2c-X>" << std::endl;
}

void wait_goniometr(std::shared_ptr<StepMotorDev>& goniometr, uint64_t expected_wait) {
    const char* func_name = __FUNCTION__;

    std::vector<StepMotorStatus> mstatus;
    uint8_t dev_status;

    if (expected_wait) {
        std::this_thread::sleep_for(std::chrono::microseconds (expected_wait + expected_wait/2));
    }

    do {
        dev_status = goniometr->status(mstatus);
        if (dev_status==STEP_MOTOR_DEV_STATUS_RUN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } else {
            break;
        }
    } while (true);

    if (dev_status==STEP_MOTOR_DEV_STATUS_ERROR) {
        throw EKitException(func_name, EKIT_FAIL, "Motor homing has failed.");
    }
}

int main(int argc, char* argv[]) {
    const char* func_name = __FUNCTION__;
    try {
        if (argc != 2) {
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Wrong number of arguments");
        }

        // Open I2C bus
        const char* i2c_dev = argv[1];
        std::shared_ptr<EKitBus> i2cbus(new EKitI2CBus(i2c_dev));
        EKitTimeout time_out(0);
    	EKIT_ERROR err = i2cbus->open(time_out);
        if (err != EKIT_OK) {
            throw EKitException(__FUNCTION__, EKIT_FAIL, tools::format_string("Failed to open %s", i2c_dev));
        }

        // Open firmware (via I2C)
        std::shared_ptr<EKitBus> firmware (new EKitFirmware(i2cbus, rowland_scan::INFO_I2C_ADDRESS));


        // Open goniometr
        std::shared_ptr<StepMotorDev> goniometr(new StepMotorDev(firmware, rowland_scan::step_motor_goniometr_config_ptr));

        // Open adc
        std::shared_ptr<ADCDev> adc(new ADCDev(firmware, rowland_scan::adc_adc_config));

        // Open info device
        std::shared_ptr<INFODev> info_dev(new INFODev(firmware, rowland_scan::info_config_ptr));
        info_dev->check();

        // Home detector
        goniometr->enable(0, true);
        goniometr->speed(0, 0.002L, false);
        goniometr->dir(0, false);
        goniometr->move(0);
        goniometr->feed();
        goniometr->start();

        // Wait until idle
        wait_goniometr(goniometr,0);

        size_t step_count = 1000;
        size_t step = rowland_scan::step_motor_goniometr_config_ptr->motor_descriptor[0]->cw_sft_limit / step_count;
        size_t channel_index = 2;
        std::string adc_channel_name = adc->get_input_name(channel_index, true);
        std::cout << "n," << adc_channel_name << std::endl;

        // reset goniometr position by calling stop
        goniometr->stop();
        goniometr->enable(0, true);
        goniometr->dir(0,true);
        goniometr->speed(0, 0.002L, false);

        for (size_t i=0; i<step_count; i++) {
            uint64_t wait = goniometr->move(0, step);
	    goniometr->feed();
	    goniometr->start();

            wait_goniometr(goniometr, wait);

            // Measure
            std::vector<double> av_values;
            bool ovf = false;
            adc->start(100, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds (100));
            adc->get(av_values, ovf);

            // Print data
            std::cout << i << "," << av_values[channel_index] << std::endl;
        }
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
