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

from ExclusiveDeviceCustomizer import *
from tools import concat_lines


class GPIODevCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "CONTROLS")
        self.fw_header = "fw_gpiodev.h"
        self.sw_header = "sw_gpiodev.h"
        self.shared_header = "gpio_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__GPIO_SHARED_HEADER__")

    def customize(self):
        fw_pin_list = []
        sw_pin_list = []
        pin_defs = []
        gpio_requires = dict()
        self.dev_config["requires"] = gpio_requires
        index = 0
        for pin_name, pin_cfg in self.dev_config["description"].items():
            gpio = self.get_gpio(pin_cfg)
            pin_type = self.get_gpio_pin_type(pin_cfg["type"])
            if self.mcu_hw.is_GPIO_input(pin_type):
                in_def = "GPIO_DEV_INPUT"
                def_val = 0;
            else:
                in_def = "GPIO_DEV_OUTPUT"
                def_val = self.get_int_value(pin_name, pin_cfg, "default", {0, 1})

            pin_def_name = "GPIODEV_{0}_PIN".format(pin_name.upper())

            fw_pin_list.append("{{ {0}, {1}, {2}, {3} }}".format(pin_type,
                                                                 self.mcu_hw.GPIO_to_port(gpio),
                                                                 self.mcu_hw.GPIO_to_pin_number(gpio),
                                                                 def_val))

            sw_pin_list.append('{{ {0}, {1}, {2}, "{3}" }}'.format(pin_def_name,
                                                            in_def,
                                                            def_val,
                                                            pin_name))

            pin_defs.append("#define {0}  {1}".format(pin_def_name, index))
            gpio_requires[pin_def_name] = {"gpio": gpio}
            index += 1

        vocabulary = {"__DEVICE_ID__": self.dev_config["dev_id"],
                      "__GPIO_DEVICE_NAME__": self.device_name,
                      "__GPIO_DEV_PINS_DECLARATION__": concat_lines(pin_defs),
                      "__GPIO_FW_DEV_DESCRIPTION__": ", ".join(fw_pin_list),
                      "__GPIO_SW_DEV_DESCRIPTION__": ", ".join(sw_pin_list),
                      "__GPIO_PIN_COUNT__": index}

        self.patch_templates(vocabulary)
