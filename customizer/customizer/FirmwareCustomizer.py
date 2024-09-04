#   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
#
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

from BaseDeviceCustomizer import *
from tools import *
import os
import glob
from shutil import copy


class FirmwareCustomizer(BaseDeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config, required_resources, required_features):
        super().__init__(mcu_hw, dev_config, common_config)
        self.device_name = "FIRMWARE"
        self.dev_config = dev_config
        self.fw_header = "fw.h"
        self.example_main_hpp = "main.hpp"
        self.example_main_cpp = "main.cpp"
        self.fw_dev_headers = []
        self.feature_defines = []
        self.sw_dev_headers = []
        self.allocated_dev_ids = []
        self.required_resources = required_resources
        self.required_features = required_features
        self.hlek_name = "hlek"
        self.install_path = common_config["global"]["install_path"]
        self.cmsis_path = common_config["global"]["cmsis_path"]
        self.libhlek_name = "lib" + self.hlek_name
        self.libhlek_install_path = os.path.join(self.install_path, self.libhlek_name)

        self.config_name = common_config[FW_FIRMWARE][KW_DEV_NAME]
        self.libconfig_name = "lib" + self.config_name
        self.libconfig_install_path = os.path.join(self.install_path, self.libconfig_name)

        self.sw_libconfig_name = "libconfig.hpp"
        self.sw_lib_inc_templ = os.path.join(self.sw_lib_inc_templ_path, self.sw_libconfig_name)

        # Get error codes constants from libhlek (ekit_error.hpp) to the firmware header (fw.h)
        self.libhlek_error_header = "ekit_error.hpp"
        self.error_codes = self.extracted_from_file(os.path.join(self.project_dir, self.libhlek_name, "inc", self.libhlek_error_header),
                                                    "// <__LIBHLEK_ERROR_DEFINES__>",
                                                    "// <\__LIBHLEK_ERROR_DEFINES__>")

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        # CMakeLists.txt
        self.add_template(os.path.join(self.fw_templ, self.cmake_script),
                          [os.path.join(self.fw_dest, self.cmake_script)])

        # toolchain.cmake
        self.add_template(os.path.join(self.fw_templ, self.toolchain_name),
                          [os.path.join(self.fw_dest, self.toolchain_name)])

        self.add_copy(os.path.join(self.fw_path, self.flash_script),
                      [os.path.join(self.fw_dest, self.flash_script)])

        self.add_template(os.path.join(self.fw_inc_templ, self.proto_header),
                          [os.path.join(self.fw_inc_dest, self.proto_header)])

        self.add_template(self.sw_lib_inc_templ, [os.path.join(self.sw_lib_inc_dest, self.sw_libconfig_name)])
        self.add_template(os.path.join(self.sw_lib_templ_path, self.cmake_script),
                          [os.path.join(self.sw_lib_path, self.cmake_script)])
        self.add_template(os.path.join(self.sw_templ, self.cmake_script),
                          [os.path.join(self.sw_monitor_dest, self.cmake_script)])

        self.add_template(os.path.join(self.sw_example_templ, self.cmake_script),
                          [os.path.join(self.sw_example_dest, self.cmake_script)])
        self.add_template(os.path.join(self.sw_example_inc_templ, self.example_main_hpp),
                          [os.path.join(self.sw_example_inc_dest, self.example_main_hpp)])
        self.add_template(os.path.join(self.sw_example_src_templ, self.example_main_cpp),
                          [os.path.join(self.sw_example_src_dest, self.example_main_cpp)])

        self.add_template(os.path.join(self.sw_testtool_templ, self.cmake_script),
                          [os.path.join(self.sw_testtool_dest, self.cmake_script)])


    def customize(self):

        buffer_size = self.dev_config[FW_I2C][KW_BUFFER_SIZE]
        address_0 = self.dev_config[FW_I2C][KW_ADDRESS]
        clock_speed = self.dev_config[FW_I2C][KW_CLOCK_SPEED]
        device_name = self.dev_config[KW_DEV_NAME]

        extender_requires = self.dev_config[FW_I2C][KW_REQUIRES]
        i2c_periph = self.get_i2c(extender_requires)

        self.mcu_hw.check_i2c_clock_speed(i2c_periph, clock_speed)

        # Update requirements for I2C
        extender_requires = {**extender_requires, **self.mcu_hw.mcu_resources[i2c_periph][KW_REQUIRES]}
        sda = self.get_gpio(extender_requires[KW_SDA_LINE])
        scl = self.get_gpio(extender_requires[KW_SCL_LINE])
        ev_isr = self.get_isr(extender_requires[KW_EV_IRQ_HLR])
        er_isr = self.get_isr(extender_requires[KW_ER_IRQ_HLR])

        # Get real I2C periph name without remap
        i2c_periph, remap = self.mcu_hw.is_remaped(i2c_periph)

        timer_requires = self.dev_config[FW_SYS_TICK][KW_REQUIRES]
        systick_timer_periph = self.get_timer(timer_requires)
        timer_requires = {**timer_requires, **self.mcu_hw.mcu_resources[systick_timer_periph][KW_REQUIRES]}

        systick_isr = self.mcu_hw.TIMER_to_IRQHandler(systick_timer_periph)
        systick_irqn = self.mcu_hw.ISRHandler_to_IRQn(systick_isr)

        self.required_resources.extend(get_leaf_values(extender_requires))
        self.required_resources.extend(get_leaf_values(timer_requires))

        self.vocabulary = self.vocabulary | {
                      "__I2C_BUS_PERIPH__": i2c_periph,
                      "__I2C_BUS_PINS_REMAP__": int(remap),
                      "__I2C_BUS_CLOCK_SPEED__": clock_speed,
                      "__I2C_BUS_SDA_PORT__": self.mcu_hw.GPIO_to_port(sda),
                      "__I2C_BUS_SDA_PIN__": self.mcu_hw.GPIO_to_pin_number(sda),
                      "__I2C_BUS_SDA_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(sda),
                      "__I2C_BUS_SCL_PORT__": self.mcu_hw.GPIO_to_port(scl),
                      "__I2C_BUS_SCL_PIN__": self.mcu_hw.GPIO_to_pin_number(scl),
                      "__I2C_BUS_SCL_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(scl),
                      "__I2C_BUS_EV_ISR__": ev_isr,
                      "__I2C_BUS_EV_IRQ__": self.mcu_hw.ISRHandler_to_IRQn(ev_isr),
                      "__I2C_BUS_ER_ISR__": er_isr,
                      "__I2C_BUS_ER_IRQ__": self.mcu_hw.ISRHandler_to_IRQn(er_isr),

                      "__SYS_TICK_PERIPH__": systick_timer_periph,
                      "__SYS_TICK_ISR__": systick_isr,
                      "__SYS_TICK_IRQ__": systick_irqn,

                      "__FW_HEADERS__": concat_lines(self.fw_dev_headers),
                      "__FW_FEATURE_DEFINES__": concat_lines(self.feature_defines),
                      "__SW_HEADERS__": concat_lines(self.sw_dev_headers),
                      "__COMM_BUFFER_LENGTH__": buffer_size,
                      "__I2C_FIRMWARE_ADDRESS__": address_0,
                      "__APB_CLOCK_ENABLE__": self.mcu_hw.ENABLE_CLOCK_on_APB(self.required_resources),
                      "__MCU_FREQUENCY_MHZ__": self.mcu_hw.system_clock // 1000000,
                      "__MCU_FREQUENCY__": self.mcu_hw.system_clock,
                      "__MCU_MAXIMUM_TIMER_US__": (0xFFFF + 1) * (0xFFFF + 1) * 1000000 // self.mcu_hw.system_clock,
                      "__DEVICE_NAME__": device_name,
                      "__NAMESPACE_NAME__": self.project_name,
                      "__HLEK_NAME__": self.hlek_name,
                      "__LIBHLEK_NAME__": self.libhlek_name,
                      "__LIBHLEK_INSTALL_PATH__": self.libhlek_install_path,
                      "__LIBCONFIG_NAME__": self.config_name,
                      "__LIBCONFIG_INSTALL_PATH__": self.libconfig_install_path,
                      "__STDPERIF_PATH__": self.cmsis_path,
                      "__LIBHLEK_ERROR_DEFINES__": self.error_codes
                      }

        self.vocabulary.update(self.make_feature_macroses())
        # print(vocabulary)

        self.patch_templates()



    def add_fw_feature_define(self, feature_define: str):
        define = f"""#define {feature_define} 1"""
        self.feature_defines.append(define)

    def add_fw_header(self, header: str):
        self.fw_dev_headers.append("#include \"{0}\"".format(header))
        return

    def add_sw_header(self, header: str):
        self.sw_dev_headers.append(f'#include \"{header}\"')
        return

    def add_shared_header(self, config: dict, customizer: str):
        hlek_lib_common_header, shared_header, fw_header, sw_header, shared_token = config["generation"]["shared"][customizer]
        self.add_shared_code(os.path.join(self.shared_templ, shared_header), shared_token)
        return

    def detect_conflicting_resources(self):
        conflicts = get_duplicates(self.required_resources)
        if conflicts:
            raise RuntimeError("Duplicate resources are detected: {0}".format(", ".join(conflicts)))

    def add_allocated_dev_ids(self, dev_ids: list):
        self.allocated_dev_ids.extend(dev_ids)
        return

    def detect_conflicting_dev_ids(self):
        conflicts = get_duplicates(self.allocated_dev_ids)
        if conflicts:
            raise RuntimeError("Duplicate dev_id(s) are detected: {0}".format(", ".join(map(str, conflicts))))

    def check_dev_ids(self):
        for dev_id in self.allocated_dev_ids:
            if isinstance(dev_id, int) and 0 <= dev_id <= 15:
                return
            else:
                raise RuntimeError("Malformed dev_id is detected: {0}".format(dev_id))

    def make_feature_macroses(self) -> dict:
        result = dict()
        for feature in self.required_features:
            if feature not in self.mcu_hw.FW_FEATURES:
                raise RuntimeError(
                    f"Unknown feature {feature} required by device {self.device_name}. Check FW_FEATURES list...")

        for feature in self.mcu_hw.FW_FEATURES:
            value = 1 if feature in self.required_features else 0
            key = f"__ENABLE_{feature}__"
            result[key] = value

        return result
