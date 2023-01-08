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

constexpr int16_t x_off = -571;
constexpr int16_t y_off = -944;
constexpr int16_t z_off = 1852;

void
adxl_345_calibrate(std::shared_ptr<ADXL345> adxl,
                   ADXL345Data &avg_vals) {
    constexpr int32_t n_samples = 1000;
    ADXL345Sample sample;
    int32_t count = 0;

    int32_t x=0,y=0,z=0;

    std::cout << "Calibration: Rotate sensor along all three axis to capture all possible positions." << std::endl <<
              "Press ANY KEY when ready." << std::endl;
    wait_a_key();
    adxl->clear_fifo();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        uint8_t events = adxl->get_events();

        while (events & ADXL345::ADXLEvents::ADXL_EV_DATA_READY) {
            adxl->get_data(&sample);
            events = adxl->get_events();
            x +=  sample.data.x;
            y +=  sample.data.y;
            z +=  sample.data.z;

            count++;

            if (count >= n_samples) {
                goto done;
            }
        }
    }

done:

    avg_vals.x = static_cast<int16_t>(x / count);
    avg_vals.y = static_cast<int16_t>(y / count);
    avg_vals.z = static_cast<int16_t>(z / count);
}

void adxl345_monitor(std::shared_ptr<ADXL345> adxl, std::shared_ptr<EKitBus> spi) {
    ADXL345Sample data;
    ADXL345DataDbl dbl_data;
    const size_t conf_fifo_len = ADXL345::ADXL345Constants::FIFO_CTL_SAMPLES_DEFAULT;
    adxl->configure(ADXL345::ADXL345Constants::BW_RATE_100HZ,
                    conf_fifo_len,
                    ADXL345::ADXL345Constants::DATA_FORMAT_RANGE_16g);
    adxl->enable(true);
    adxl->clear_fifo();




    // ------------------- CALIBRATION -------------------
#ifdef CALIBRATION
    ADXL345Data data_avg;
    while (true) {
        adxl_345_calibrate(adxl, data_avg);

        data_avg.x -= x_off;
        data_avg.y -= y_off;
        data_avg.z -= z_off;

        std::cout << "Ax=" << data_avg.x << "\t\tAy=" << data_avg.y << "\t\tAz=" << data_avg.z << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;

    }
#else

    int32_t x=0,y=0,z=0;
    const int32_t n_samples = 32;
    int32_t count = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        uint8_t events = adxl->get_events();
        while (events & ADXL345::ADXLEvents::ADXL_EV_DATA_READY) {
            adxl->get_data(&data);

            x += (data.data.x - x_off);
            y += (data.data.y - y_off);
            z += (data.data.z - z_off);

            count++;

            if (count>=n_samples) {
                data.data.x = static_cast<int16_t>(x/count);
                data.data.y = static_cast<int16_t>(y/count);
                data.data.z = static_cast<int16_t>(z/count);

                adxl->to_double_data(data.data, dbl_data);

                std::cout << "Ax = " << dbl_data.x << " m/s^2 ; "
                          << "Ay = " << dbl_data.y << " m/s^2 ; "
                          << "Az = " << dbl_data.z << " m/s^2 ; "
                          << "T = " << data.timestamp.tv_sec << " s. " << data.timestamp.tv_nsec << " ns."
                          << std::endl;

                count = 0;
                x=0;
                y=0;
                z=0;
            }

            events = adxl->get_events();
        }
    }
#endif
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

