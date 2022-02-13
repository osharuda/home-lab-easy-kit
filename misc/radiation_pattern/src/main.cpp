/**
 *   Copyright 2022 Oleh Sharuda <oleh.sharuda@gmail.com>
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

#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include "main.hpp"
#include "ekit_i2c_bus.hpp"
#include "ekit_firmware.hpp"
#include "info_dev.hpp"
#include "step_motor.hpp"
#include "adcdev.hpp"

const uint8_t ADC_DEV_ID = 1;
const uint8_t STEP_MOT_DEV_ID = 2;
const uint8_t MOT_ID = 0;
const uint8_t MOT_USTEP = 32;
const size_t FULL_REVOLUTION = 200;
const size_t SCAN_STEPS = 100;
const double angle_step = 360.0L / FULL_REVOLUTION;
const double rpm = 1.0;


void make_mot_step(std::shared_ptr<StepMotorDev>& sm, uint64_t n, bool cw, double rpm) {
    sm->dir(MOT_ID, cw);
    sm->speed(MOT_ID, rpm, true);
    sm->move(MOT_ID, n*MOT_USTEP);
    sm->feed(); // feed commands to device
    sm->start(); // execute them

    // wait for command execution
    uint8_t status = STEP_MOTOR_DEV_STATUS_RUN;
    std::vector<StepMotorStatus> mstatus;
    while (status!=STEP_MOTOR_DEV_STATUS_IDLE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mstatus.clear();
        status = sm->status(mstatus);
    }
}

double read_adc(std::shared_ptr<ADCDev>& adc) {
    static std::once_flag once;
    static size_t input_count = 0;
    static size_t samples_count = 0;

    std::call_once(once, [&]() {
        input_count = adc->get_input_count();
        samples_count = adc->get_descriptor(0)->dev_buffer_len / (input_count*sizeof(uint16_t));
    });

    std::vector<std::vector<double>> values;
    bool ovf = false;
    adc->stop(true);
    adc->start(samples_count, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    adc->get(values, ovf);

    double result = 0.0L;
    for (auto x : values[0]) result += x;
    return result / samples_count;
}

int main(int argc, char* argv[]) {
    std::shared_ptr<EKitBus>        i2cbus(new EKitI2CBus("/dev/i2c-1"));
    i2cbus->open();
    std::shared_ptr<EKitBus>        firmware (new EKitFirmware(i2cbus, I2C_FIRMWARE_ADDRESS));
    std::shared_ptr<StepMotorDev>   sm( new StepMotorDev(firmware, STEP_MOT_DEV_ID));
    std::shared_ptr<ADCDev>         adc(new ADCDev(firmware, ADC_DEV_ID));

    // Reset motor
    sm->stop();
    sm->reset(MOT_ID);
    sm->enable(MOT_ID, true);
    sm->configure(MOT_ID, STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE | STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE);

    make_mot_step(sm, SCAN_STEPS/2, false, rpm); // Move to start position.

    std::cout << "\"Angle\",\"Value\"" << std::endl; // Write CSV header.

    // Scan radiation pattern
    double angle = -90.0L;
    for (size_t i=0; i<SCAN_STEPS; i++) {
        make_mot_step(sm, 1, true, rpm);
        angle += angle_step;
        std::cout << angle << ", " << read_adc(adc) << std::endl; // Write measurement.
    }

    make_mot_step(sm, SCAN_STEPS/2, false, rpm); // Return stepper motor back.
    sm->stop(); // Stop stepper motor
}
