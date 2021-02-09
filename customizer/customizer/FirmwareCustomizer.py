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
    def __init__(self, mcu_hw, dev_config, required_resources):
        super().__init__(mcu_hw, dev_config)
        self.device_name = "FIRMWARE"
        self.dev_config = dev_config
        self.fw_header = "fw.h"
        self.sw_header = "sw.h"
        self.proto_header = "i2c_proto.h"
        self.fw_dev_headers = []
        self.sw_dev_headers = []
        self.allocated_dev_ids = []
        self.required_resources = required_resources

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_template(self.fw_inc_templ + self.proto_header, [self.fw_inc_dest + self.proto_header,
                                                                     self.sw_inc_dest + self.proto_header])

    def customize(self):
        buffer_size = self.dev_config["i2c_bus"]["buffer_size"]
        address_0 = self.dev_config["i2c_bus"]["address"]
        clock_speed = self.dev_config["i2c_bus"]["clock_speed"]

        extender_requires = self.dev_config["i2c_bus"]["requires"]
        i2c_periph = self.get_i2c(extender_requires)

        # Update requirements for I2C
        extender_requires = {**extender_requires, **self.mcu_hw.mcu_resources[i2c_periph]["requires"]}
        sda = self.get_gpio(extender_requires["SDA"])
        scl = self.get_gpio(extender_requires["SCL"])
        ev_isr = self.get_isr(extender_requires["ev_irq_handler"])
        er_isr = self.get_isr(extender_requires["er_irq_handler"])

        self.required_resources.extend(get_leaf_values(extender_requires))

        vocabulary = {"__I2C_BUS_PERIPH__": i2c_periph,
                      "__I2C_BUS_CLOCK_SPEED__": clock_speed,
                      "__I2C_BUS_SDA_PORT__": self.mcu_hw.GPIO_to_port(sda),
                      "__I2C_BUS_SDA_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(sda),
                      "__I2C_BUS_SCL_PORT__": self.mcu_hw.GPIO_to_port(scl),
                      "__I2C_BUS_SCL_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(scl),
                      "__I2C_BUS_EV_ISR__": ev_isr,
                      "__I2C_BUS_EV_IRQ__": self.mcu_hw.ISRHandler_to_IRQn(ev_isr),
                      "__I2C_BUS_ER_ISR__": er_isr,
                      "__I2C_BUS_ER_IRQ__": self.mcu_hw.ISRHandler_to_IRQn(er_isr),
                      "__FW_HEADERS__": concat_lines(self.fw_dev_headers),
                      "__SW_HEADERS__": concat_lines(self.sw_dev_headers),
                      "__COMM_BUFFER_LENGTH__": buffer_size,
                      "__I2C_FIRMWARE_ADDRESS__": address_0,
                      "__APB_CLOCK_ENABLE__": self.mcu_hw.ENABLE_CLOCK_on_APB(self.required_resources),

                      "__MCU_FREQUENCY_MHZ__": self.mcu_hw.system_clock // 1000000,
                      "__MCU_FREQUENCY__": self.mcu_hw.system_clock,
                      "__MCU_MAXIMUM_TIMER_US__":  (0xFFFF+1)*(0xFFFF+1)*1000000//self.mcu_hw.system_clock
                      }

        self.patch_templates(vocabulary)
        self.copy_files_for_tests()

    def add_fw_header(self, header : str):
        self.fw_dev_headers.append("#include \"{0}\"".format(header))
        return

    def add_sw_header(self, header : str):
        self.sw_dev_headers.append("#include \"{0}\"".format(header))
        return

    def clean(self):
        fl = list()
        fl.extend(glob.glob(self.fw_inc_dest+"fw*.h"))
        fl.extend(glob.glob(self.fw_inc_dest + self.proto_header))
        fl.extend(glob.glob(self.fw_src_dest + "fw*.c"))
        fl.extend(glob.glob(self.sw_inc_dest+"sw*.h"))
        fl.extend(glob.glob(self.sw_inc_dest + self.proto_header))
        fl.extend(glob.glob(self.sw_src_dest + "sw*.c"))

        for f in fl:
            os.remove(f)

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

    def copy_files_for_tests(self):
        file_list = [(self.fw_inc_dest + "circbuffer.h", self.sw_testtool_dest),
                     (self.fw_src_dest + "circbuffer.c", self.sw_testtool_dest),
                     (self.fw_inc_dest + "utools.h", self.sw_testtool_dest),
                     (self.fw_src_dest + "utools.c", self.sw_testtool_dest)]

        for src, dst in file_list:
            copy(src, dst)
