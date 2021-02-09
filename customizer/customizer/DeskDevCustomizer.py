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

class DeskDevCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "DESKDEV")
        self.fw_header = "fw_deskdev.h"
        self.sw_header = "sw_deskdev.h"
        self.shared_header = "deskdev_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__DESKDEV_SHARED_HEADER__")

    def customize(self):
        requires = self.dev_config["requires"]

        btn_up = self.get_gpio(requires["up"])
        btn_down = self.get_gpio(requires["down"])
        btn_left = self.get_gpio(requires["left"])
        btn_right = self.get_gpio(requires["right"])
        enc_a = self.get_gpio(requires["encoder"]["A"])
        enc_b = self.get_gpio(requires["encoder"]["B"])

        params = {"__DEVICE_ID__": self.dev_config["dev_id"],
                  "__DESKDEV_DEVICE_NAME__": self.device_name,

                  "__BUTTON_UP_PORT__": self.mcu_hw.GPIO_to_port(btn_up),
                  "__BUTTON_UP_PIN__": self.mcu_hw.GPIO_to_pin_number(btn_up),
                  "__BUTTON_UP_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(btn_up),
                  "__BUTTON_UP_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(btn_up),

                  "__BUTTON_DOWN_PORT__": self.mcu_hw.GPIO_to_port(btn_down),
                  "__BUTTON_DOWN_PIN__": self.mcu_hw.GPIO_to_pin_number(btn_down),
                  "__BUTTON_DOWN_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(btn_down),
                  "__BUTTON_DOWN_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(btn_down),

                  "__BUTTON_LEFT_PORT__": self.mcu_hw.GPIO_to_port(btn_left),
                  "__BUTTON_LEFT_PIN__": self.mcu_hw.GPIO_to_pin_number(btn_left),
                  "__BUTTON_LEFT_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(btn_left),
                  "__BUTTON_LEFT_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(btn_left),

                  "__BUTTON_RIGHT_PORT__": self.mcu_hw.GPIO_to_port(btn_right),
                  "__BUTTON_RIGHT_PIN__": self.mcu_hw.GPIO_to_pin_number(btn_right),
                  "__BUTTON_RIGHT_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(btn_right),
                  "__BUTTON_RIGHT_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(btn_right),

                  "__ENCODER_A_PORT__": self.mcu_hw.GPIO_to_port(enc_a),
                  "__ENCODER_A_PIN__": self.mcu_hw.GPIO_to_pin_number(enc_a),
                  "__ENCODER_A_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(enc_a),
                  "__ENCODER_A_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(enc_a),

                  "__ENCODER_B_PORT__": self.mcu_hw.GPIO_to_port(enc_b),
                  "__ENCODER_B_PIN__": self.mcu_hw.GPIO_to_pin_number(enc_b),
                  "__ENCODER_B_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(enc_b),
                  "__ENCODER_B_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(enc_b),
                  }

        self.patch_templates(params)

        requires["exti_line_btn_up"] = {"exti_line" : self.mcu_hw.GPIO_to_EXTI_line(btn_up)}
        requires["exti_line_btn_down"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(btn_down)}
        requires["exti_line_btn_left"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(btn_left)}
        requires["exti_line_btn_right"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(btn_right)}
        requires["exti_line_enc_a"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(enc_a)}
        requires["exti_line_enc_b"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(enc_b)}
