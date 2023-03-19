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
 *   \brief AD9850Dev device software implementation
 *   \author Oleh Sharuda
 */

#include "ad9850dev.hpp"

#include <math.h>

#include "ad9850_common.hpp"
#include "ekit_firmware.hpp"

AD9850Dev::AD9850Dev(std::shared_ptr<EKitBus>& ebus, const AD9850Config* cfg)
    : super(ebus, cfg->dev_id, cfg->dev_name), config(cfg) {}

AD9850Dev::~AD9850Dev() {}

void AD9850Dev::reset() {
    static const char* const func_name = "AD9850Dev::reset";
    EKIT_ERROR               err;
    AD9850Command            cmd;
    cmd.freq_b31_b24 = 0;
    cmd.freq_b23_b16 = 0;
    cmd.freq_b15_b8  = 0;
    cmd.freq_b7_b0   = 0;
    cmd.W0           = 0;
    cmd.power_down   = 1;

    EKitTimeout to(get_timeout());
    BusLocker   bl(bus, get_addr(), to);
    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, AD9850DEV_RESET, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to set reset flag");
    }

    err = bus->write(&cmd, sizeof(cmd), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to write bus");
    }

    err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, 0, to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to clear reset flag");
    }
}

void AD9850Dev::update(double frequency, double phase) {
    static const char* const func_name = "AD9850Dev::update";
    EKIT_ERROR               err;
    AD9850Command            cmd;
    static_assert(sizeof(cmd) == 5,
                  "AD9850Command size must be 5 bytes, check structure padding "
                  "and alignment.");  // Check structure size, must be 5
    cmd.W0            = 0;
    double clock_freq = static_cast<double>(config->clock_frequency);

    // Convert frequency to uint32_t
    if (frequency < 0 || frequency >= clock_freq) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Bad frequency value");
    }
    uint32_t f       = static_cast<uint32_t>((frequency / clock_freq)
                                       * static_cast<double>(UINT32_MAX));
    cmd.freq_b31_b24 = (f >> 24) & 0xff;
    cmd.freq_b23_b16 = (f >> 16) & 0xff;
    cmd.freq_b15_b8  = (f >> 8) & 0xff;
    cmd.freq_b7_b0   = f & 0xff;

    // Convert phase 5 bit value
    if (phase < 0 || phase > 2 * M_PI) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "Bad frequency value");
    }

    uint8_t p = static_cast<uint8_t>((phase / (2.0 * M_PI))
                                     * static_cast<double>(0b00011111));
    assert(p < 0b00100000);
    cmd.phase = p;

    EKitTimeout to(get_timeout());
    BusLocker   bl(bus, get_addr(), to);
    err = bus->write(&cmd, sizeof(cmd), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "Failed to write bus");
    }
}
