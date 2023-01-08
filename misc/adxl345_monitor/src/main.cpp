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
#include <libhlek/ekit_spi_bus.hpp>
#include <libhlek/texttools.hpp>
#include <libhlek/adxl345.hpp>
#include <thread>
#include <chrono>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>


void help() {
    std::cout << "Usage: example </dev/spidevX.Y>" << std::endl;
}

bool is_key_pressed(bool consume = true) {
    struct termios termios_bak, termios_new;

    tcgetattr(STDIN_FILENO,&termios_bak);
    termios_new=termios_bak;
    termios_new.c_lflag &=(~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&termios_new);

    int char_count = 0;
    ioctl(STDIN_FILENO, FIONREAD, &char_count);

    if (char_count && consume) {
        getchar();
    }

    /* restore the former settings */
    tcsetattr(STDIN_FILENO,TCSANOW,&termios_bak);
    return char_count;
}

void wait_a_key() {
    while(!is_key_pressed()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

size_t
adxl_345_calibrate(std::shared_ptr<ADXL345> adxl,
                   ADXL345Data &max_vals,
                   ADXL345Data &min_vals,
                   uint32_t &mod_max,
                   uint32_t &mod_min) {
    ADXL345Sample sample;
    mod_min = UINT_MAX;
    mod_max = 0;
    size_t count = 0;

    max_vals.x = INT16_MIN;
    max_vals.y = INT16_MIN;
    max_vals.z = INT16_MIN;

    min_vals.x = INT16_MAX;
    min_vals.y = INT16_MAX;
    min_vals.z = INT16_MAX;

    std::cout << "Calibration: Rotate sensor along all three axis to capture all possible positions." << std::endl <<
              "Press ANY KEY when ready." << std::endl;
    wait_a_key();
    std::cout << "Press ANY KEY again when done..." << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        uint8_t events = adxl->get_events();

        while (events & ADXL345::ADXLEvents::ADXL_EV_DATA_READY) {
            adxl->get_data(&sample);
            events = adxl->get_events();

            // Calculate min and max
            max_vals.x = std::max(max_vals.x, sample.data.x);
            max_vals.y = std::max(max_vals.y, sample.data.y);
            max_vals.z = std::max(max_vals.z, sample.data.z);

            min_vals.x = std::min(min_vals.x, sample.data.x);
            min_vals.y = std::min(min_vals.y, sample.data.y);
            min_vals.z = std::min(min_vals.z, sample.data.z);

            uint32_t module = (sample.data.x * sample.data.x) +
                              (sample.data.y * sample.data.y) +
                              (sample.data.z * sample.data.z);

            mod_max = std::max(mod_max, module);
            mod_min = std::min(mod_max, module);

            count++;

            if (is_key_pressed()) {
                goto done;
            }
        }
    }

done:
    return count;
}

void adxl345_monitor(std::shared_ptr<ADXL345> adxl, std::shared_ptr<EKitBus> spi) {
    ADXL345Sample data;
    ADXL345DataDbl dbl_data;
    const size_t conf_fifo_len = ADXL345::ADXL345Constants::FIFO_CTL_SAMPLES_DEFAULT;
    adxl->configure(ADXL345::ADXL345Constants::BW_RATE_3_13HZ,
                    conf_fifo_len,
                    ADXL345::ADXL345Constants::DATA_FORMAT_RANGE_16g);
    adxl->enable(true);
    adxl->clear_fifo();

    // ------------------- CALIBRATION -------------------
    ADXL345Data data_max,data_min;
    uint32_t mod_max, mod_min;
    while (true) {
        size_t count = adxl_345_calibrate(adxl, data_max, data_min, mod_max, mod_min);

        std::cout << "[SAMPLES=" << count << "]" << std::endl;
        std::cout << "[MAX. VALUES] x=" << data_max.x << " y=" << data_max.y << " z=" << data_max.z << " mod^2="
                  << mod_max << std::endl;
        std::cout << "[MIN. VALUES] y=" << data_min.x << " y=" << data_min.y << " z=" << data_min.z << " mod^2="
                  << mod_min << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;

    }

    ADXL345OffsetData offset;
    adxl->get_offset_data(offset);
    std::cout << "OFSX=" << (int) (offset.ofs_x) << " OFSY=" << (int) (offset.ofs_y) << " OFSZ="
              << (int) (offset.ofs_z)
              << std::endl;


    /*
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        uint8_t events = adxl->get_events();
        while (events & ADXL345::ADXLEvents::ADXL_EV_DATA_READY) {
            adxl->get_data(&data);
            adxl->to_double_data(data.data, dbl_data);

            std::cout << "Ax = " << dbl_data.x << " m/s^2 ; "
                      << "Ay = " << dbl_data.y << " m/s^2 ; "
                      << "Az = " << dbl_data.z << " m/s^2 ; "
                      << "T = " << data.timestamp.tv_sec << " s. " << data.timestamp.tv_nsec << " ns."
                      << std::endl;

            events = adxl->get_events();
        }
    }
     */
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cout << "Wrong number of arguments" << std::endl;
        help();
        return 1;
    }

    try {
        // Print information about available devices
        std::cout << "*** Welcome to Home Lab Easy Kit ***" << std::endl;

        // Open SPI bus
        const char *spi_dev = argv[1];
        std::shared_ptr<EKitBus> spibus(new EKitSPIBus(spi_dev));
        EKitTimeout to(5);
        {
            BusLocker block(spibus, to);

            EKIT_ERROR err = spibus->open(to);
            if (err != EKIT_OK) {
                throw EKitException(__FUNCTION__, err, tools::format_string("Failed to open %s", spi_dev));
            }

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_CLOCK_PHASE, 1, to);
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_CLOCK_PHASE\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_CLOCK_POLARITY, 1, to);
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_CLOCK_POLARITY\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_CS_HIGH, 0, to);
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_CS_HIGH\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_LSB_FIRST, 0, to);
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_LSB_FIRST\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_NO_CS, 0, to);
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_NO_CS\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_CLOCK_FREQUENCY, 4000000, to); // 1 MHz
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_CLOCK_FREQUENCY\n";

            // err = spibus->set_opt(EKitSPIBus::SPI_OPT_WORD_SIZE, 16); // 16 bits
            // if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_WORD_SIZE\n";

            err = spibus->set_opt(EKitSPIBus::SPI_OPT_CS_CHANGE, 0, to); // Deselect when done
            if (err != EKIT_OK) std::cout << "Failed set SPI_OPT_CS_CHANGE\n";
        }

        std::shared_ptr<ADXL345> adxl345(new ADXL345(spibus, 1000, "adxl345"));
        adxl345_monitor(adxl345, spibus);

    } catch (EKitException e) {
        std::cout << e.what() << std::endl;
    };
}

