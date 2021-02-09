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


class LCD1602aCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "LCD1602a")
        self.fw_header = "fw_lcd1602a.h"
        self.sw_header = "sw_lcd1602a.h"
        self.shared_header = "lcd1602a_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__LCD1602A_SHARED_HEADER__")

    def customize(self):
        lcd_requires = self.dev_config["requires"]
        enable = self.get_gpio(lcd_requires["enable"])
        reg_sel = self.get_gpio(lcd_requires["reg_sel"])
        data4 = self.get_gpio(lcd_requires["data4"])
        data5 = self.get_gpio(lcd_requires["data5"])
        data6 = self.get_gpio(lcd_requires["data6"])
        data7 = self.get_gpio(lcd_requires["data7"])
        light = self.get_gpio(lcd_requires["light"])
        wline1, wline2 = self.get_wellcome()


        vocabulary = {"__DEVICE_ID__": self.dev_config["dev_id"],
                      "__LCD1602a_DEVICE_NAME__": self.device_name,

                      "__LCD1602a_ENABLE_PORT__": self.mcu_hw.GPIO_to_port(enable),
                      "__LCD1602a_ENABLE_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(enable),

                      "__LCD1602a_REGISTER_SELECT_PORT__": self.mcu_hw.GPIO_to_port(reg_sel),
                      "__LCD1602a_REGISTER_SELECT_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(reg_sel),

                      "__LCD1602a_DATA4_PORT__": self.mcu_hw.GPIO_to_port(data4),
                      "__LCD1602a_DATA4_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(data4),

                      "__LCD1602a_DATA5_PORT__": self.mcu_hw.GPIO_to_port(data5),
                      "__LCD1602a_DATA5_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(data5),

                      "__LCD1602a_DATA6_PORT__": self.mcu_hw.GPIO_to_port(data6),
                      "__LCD1602a_DATA6_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(data6),

                      "__LCD1602a_DATA7_PORT__": self.mcu_hw.GPIO_to_port(data7),
                      "__LCD1602a_DATA7_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(data7),

                      "__LCD1602a_LIGHT_PORT__": self.mcu_hw.GPIO_to_port(light),
                      "__LCD1602a_LIGHT_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(light),

                      "__LCD1602a_WELLCOME_1__": '{0}'.format(wline1),
                      "__LCD1602a_WELLCOME_2__": '{0}'.format(wline2)}

        self.patch_templates(vocabulary)

    def get_wellcome(self) -> tuple:
        wlines = ["",""]
        if "wellcome" in self.dev_config.keys():
            wls = self.dev_config["wellcome"]
            n = min(len(wls), 2)
            for i in range(0, n):
                s = wls[i]
                if len(s) > 16:
                    raise RuntimeError("Wellcome line {0} '{1}' is longer than 16 characters ({2} characters)".format(i, s, len(s)))
                else:
                    wlines[i]=s

        return tuple(wlines)