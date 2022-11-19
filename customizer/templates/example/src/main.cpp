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
#include <lib{__DEVICE_NAME__}/info_conf.hpp>
#include <libhlek/info_dev.hpp>
#include <libhlek/ekit_i2c_bus.hpp>
#include <libhlek/ekit_firmware.hpp>
#include <libhlek/texttools.hpp>

void help() {{
    std::cout << "Usage: example </dev/i2c-X>" << std::endl;
}}

int main(int argc, char* argv[]) {{
    const char* func_name = __FUNCTION__;
    try {{
        if (argc != 2) {{
            throw EKitException(__FUNCTION__, EKIT_BAD_PARAM, "Wrong number of arguments");
        }}

        // Open I2C bus
        const char* i2c_dev = argv[1];
        std::shared_ptr<EKitBus> i2cbus(new EKitI2CBus(i2c_dev));
        EKIT_ERROR err = i2cbus->open();
        if (err != EKIT_OK) {{
            throw EKitException(__FUNCTION__, tools::format_string("Failed to open %s", i2c_dev));
        }}

        // Open firmware (via I2C)
        std::shared_ptr<EKitBus> firmware (new EKitFirmware(i2cbus, {__NAMESPACE_NAME__}::INFO_I2C_ADDRESS));

        // Create INFO device
        std::shared_ptr<INFODev> info_dev(new INFODev(firmware, {__NAMESPACE_NAME__}::info_config_ptr));

        // Print information about available devices
        auto dev_name = info_dev->get_dev_name();
        std::cout << "*** Welcome to Home Lab Easy Kit ***" << std::endl;
        std::cout << "Configuration name: " << info_dev->get_dev_name() << std::endl;
    }} catch (EKitException e) {{
        std::cout << e.what() << std::endl;

        // Specific error processing
        switch (e.ekit_error) {{
            case EKIT_BAD_PARAM:
                help();
                break;
        }};
    }}
}}