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
#include <libtb_ad9850dev/info_conf.hpp>
#include <libtb_timetrackerdev/info_conf.hpp>

#include <libhlek/info_dev.hpp>
#include <libhlek/ekit_i2c_bus.hpp>
#include <libhlek/ekit_firmware.hpp>
#include <libhlek/texttools.hpp>

#include <libhlek/ad9850dev.hpp>
#include <libtb_ad9850dev/ad9850_conf.hpp>

#include <libhlek/timetrackerdev.hpp>
#include <libtb_timetrackerdev/timetrackerdev_conf.hpp>

#include <libhlek/hlekio.hpp>

void help() {
    std::cout << "Usage: example </dev/i2c-X> <test freq, hz>" << std::endl;
}
void wait_buffer(HLEKIOInput* hi, EKitTimeout* to) {
    hi->wait(*to, nullptr);
}

int main(int argc, char* argv[]) {
    const char* func_name = __FUNCTION__;
    std::vector<double> ts;
    try {
        if (argc != 3) {
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Wrong number of arguments");
        }

        size_t pos = 0;

        double test_freq = std::stod(argv[2], &pos);
        if (argv[2][pos] != 0) {
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Bad frequency value");
        }

        // std::cout << "*** Welcome to Home Lab Easy Kit ***" << std::endl;

        // Open I2C bus
        const char* i2c_dev = argv[1];
        std::shared_ptr<EKitBus> i2cbus(new EKitI2CBus(i2c_dev));
        EKitTimeout time_out(0);
        EKIT_ERROR err = i2cbus->open(time_out);
        if (err != EKIT_OK) {
            throw EKitException(__FUNCTION__, EKIT_FAIL, tools::format_string("Failed to open %s", i2c_dev));
        }

        // Open firmware protocol for AD9850 (via I2C) and create devices
        std::shared_ptr<EKitBus> fw_ad9850 (new EKitFirmware(i2cbus, tb_ad9850dev::INFO_I2C_ADDRESS));
        std::shared_ptr<INFODev> ad9850_info_dev(new INFODev(fw_ad9850, tb_ad9850dev::info_config_ptr));
        std::shared_ptr<AD9850Dev> ad9850(new AD9850Dev(fw_ad9850, tb_ad9850dev::ad9850_gen_0_config_ptr));

        // Open firmware protocol for TimeTrackerDev (via I2C) and create devices
        std::shared_ptr<EKitBus> fw_timetackdev (new EKitFirmware(i2cbus, tb_timetrackerdev::INFO_I2C_ADDRESS));
        std::shared_ptr<INFODev> timetrackdev_info_dev(new INFODev(fw_timetackdev, tb_timetrackerdev::info_config_ptr));
        std::shared_ptr<TimeTrackerDev> ttdev(new TimeTrackerDev(fw_timetackdev, tb_timetrackerdev::timetrackerdev_timetrackerdev_0_config_ptr));

        // Open buffer overflow interrup line
        HLEKIOInput ttdev_warn("/dev/ttdev_warn");


        // Print information about available devices
        auto ad9850_name = ad9850_info_dev->get_dev_name();
        ad9850_info_dev->check();
        std::cout << ad9850_name.c_str() << " connected successfully." << std::endl;

        auto timetrackdev_name = timetrackdev_info_dev->get_dev_name();
        timetrackdev_info_dev->check();
        std::cout << timetrackdev_name.c_str() << " connected successfully." << std::endl;

        // Set ad9850
        ad9850->reset();
        ad9850->update(test_freq, 0);
        std::cout << "Frequency is set." << std::endl;

        // Check if ttdev_warn already indicates full buffer
        if (ttdev_warn.get(nullptr)) {
            std::cout << "Buffer is already full by some data." << std::endl;
        }

        // Catch events using time tracker
        ttdev->stop();
        EKitTimeout to(10000);
        std::thread wt(wait_buffer, &ttdev_warn, &to);
        ttdev->start(true);

        std::cout << "Waiting buffer overrun." << std::endl;
        wt.join();

        std::cout << "N,Timestamp" << std::endl;

        bool running;
        uint64_t first_ts;
        size_t n = ttdev->get_status(running, first_ts);
        // assert(first_ts==0);


        ttdev->read_all(ts, true);
        n=ts.size();
        for(size_t i=1; i<n; i++) {
            std::cout << i << "," << ts[i] - ts[i-1] <<std::endl;
        }

        //ad9850->reset();
        ttdev->stop();

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
