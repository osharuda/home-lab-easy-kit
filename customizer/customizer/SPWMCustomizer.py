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
from tools import *


class SPWMCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "CONTROLS")
        self.fw_header = "fw_spwm.h"
        self.sw_header = "sw_spwm.h"
        self.shared_header = "spwm_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__SPWM_SHARED_HEADER__")

    def customize(self):
        fw_pin_list = []
        sw_pin_list = []
        used_ports = dict()
        spwm_requires = self.dev_config["requires"]
        timer = self.get_timer(spwm_requires)
        pin_to_name = dict()
        spwm_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(timer)
        self.check_resource("irq_handler", spwm_irq_handler)
        spwm_requires["irq_handler"] = spwm_irq_handler
        prescaler = self.dev_config["prescaler"]

        index = 0
        for pin_name, pin_cfg in self.dev_config["description"].items():
            pin_type = self.get_gpio_pin_type(pin_cfg["type"])
            if self.mcu_hw.is_GPIO_input(pin_type):
                raise RuntimeError(
                    "Pin {0} should be output ( {1} was specified ) for SPWMCustomizer device".format(pin_name,
                                                                                                      pin_type))

            gpio = self.get_gpio(pin_cfg)
            pin_to_name[gpio] = pin_name
            pin_number = self.mcu_hw.GPIO_to_pin_number(gpio)
            pin_mask = 1 << pin_number
            port = self.mcu_hw.GPIO_to_port(gpio)
            def_val = pin_cfg["default"]

            if port not in used_ports.keys():
                used_ports[port] = {"open_drain_bits": 0, "n_bits": 0, "bitmask": 0, "def_vals": 0};

            if self.mcu_hw.is_GPIO_open_drain(pin_type):
                used_ports[port]["open_drain_bits"] = used_ports[port]["open_drain_bits"] | pin_mask

            if def_val != 0:
                used_ports[port]["def_vals"] = used_ports[port]["def_vals"] | pin_mask

            used_ports[port]["bitmask"] = used_ports[port]["bitmask"] | pin_mask
            used_ports[port]["n_bits"] = used_ports[port]["n_bits"] + 1

            spwm_requires[pin_name] = {"gpio": gpio}
            index += 1

        pin_gpio_descriptors = []
        pin_name_defines = []
        pin_sw_defines = []

        index = 0
        port_index = 0;
        for port, val in used_ports.items():
            bitmask = val["bitmask"]
            def_vals = val["def_vals"]
            descr = "{{ {0}, {1}, {2}, {3}, {4} }}".format(port,
                                                           bitmask,
                                                           val["n_bits"],
                                                           val["open_drain_bits"],
                                                           def_vals)
            pin_gpio_descriptors.append(descr)

            # walk through the established pins
            for i in range(0, self.mcu_hw.GPIO_PORT_LEN):
                t = 1 << i
                if t & bitmask != 0:
                    p = self.mcu_hw.get_GPIO(port, i)
                    name = "SPWM_" + pin_to_name[p].upper()
                    pin_name_defines.append("#define {0} {1}".format(name, index))
                    descr = '{{ {0}, {1}, {2}, "{3}" }} '.format(port_index, i, int_to_bool(def_vals & t), pin_to_name[p])
                    pin_sw_defines.append(descr)
                    index += 1

            port_index += 1

        vocabulary = {"__DEVICE_ID__": self.dev_config["dev_id"],
                      "__SPWM_DEVICE_NAME__": self.device_name,
                      "__SPWM_PORT_COUNT__": len(used_ports),
                      "__SPWM_TIMER__": timer,
                      "__SPWM_GPIO_DESCRIPTION__": ", ".join(pin_gpio_descriptors),
                      "__SPWM_CHANNEL_INDEXES__": concat_lines(pin_name_defines),
                      "__SPWM_TIM_IRQ_HANDLER__": spwm_irq_handler,
                      "__SPWM_TIM_IRQn__": self.mcu_hw.ISRHandler_to_IRQn(spwm_irq_handler),
                      "__SPWM_CHANNEL_COUNT__": index,
                      "__SPWM_MAX_PWM_ENTRIES_COUNT__": index+1,
                      "__SPWM_PRESCALE_VALUE__": prescaler,
                      "__SPWM_SW_DESCRIPTION__": ", ".join(pin_sw_defines)}

        self.patch_templates(vocabulary)
