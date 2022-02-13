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

from DeviceCustomizer import *
from tools import *

class UartProxyCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs):
        super().__init__(mcu_hw, dev_configs, "UART_PROXY")
        self.fw_header = "fw_uart_proxy.h"
        self.sw_header = "sw_uart_proxy.h"
        self.shared_header = "uart_proxy_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__UART_PROTO_SHARED_HEADER__")

    def customize(self):
        indx = 0
        buffer_size_defs = []
        buffer_defs = []
        fw_device_descrs = []
        sw_device_descrs = []
        isr_list = []

        for dev_name, dev_config in self.device_list:
            uart_proxy_requires = dev_config["requires"]

            uart_port = self.get_usart(uart_proxy_requires)
            baud_rate = dev_config["baud_rate"];
            (irq_handler, rx, tx) = self.mcu_hw.USART_to_resources(uart_port)
            buffer_size = dev_config["buffer_size"]
            dev_id = dev_config["dev_id"]
            buf_size_def = "{0}_BUFFER_LEN".format(dev_name.upper())
            buffer_size_defs.append("#define {0} {1}".format(buf_size_def, buffer_size))
            buffer_name = "g_{0}_buffer".format(dev_name)
            buffer_defs.append("uint8_t {0}[{1}]={{0}}; \\".format(buffer_name, buf_size_def))
            fw_device_descrs.append("{{ {{0}}, {{0}}, {3}, {4}, {6}, {2}, {9}, {1}, {5}, {7}, {8}, {0} }}".format(
                dev_id,                                     #0
                buf_size_def,                               #1
                buffer_name,                                #2
                uart_port,                                  #3
                self.mcu_hw.GPIO_to_port(rx),               #4
                self.mcu_hw.GPIO_to_pin_mask(rx),           #5
                self.mcu_hw.GPIO_to_port(tx),               #6
                self.mcu_hw.GPIO_to_pin_mask(tx),           #7
                self.mcu_hw.ISRHandler_to_IRQn(irq_handler),#8
                baud_rate))                                 #9

            sw_device_descrs.append("{{ {0}, {1}, \"{2}\", {3} }}".format(dev_id,
                                                                          buf_size_def,
                                                                          dev_name,
                                                                          baud_rate))

            isr_list.append("MAKE_ISR_WITH_INDEX({0}, UART_PROXY_COMMON_IRQ_HANDLER, {1}) \\".format(irq_handler, indx))

            # update requirements
            uart_proxy_requires["irq_handler"] = irq_handler
            uart_proxy_requires["rx"] = {"gpio": rx}
            uart_proxy_requires["tx"] = {"gpio": tx}

            indx+=1


        vocabulary = {"__UART_PROXY_BUFFERS__": concat_lines(buffer_defs),
                      "__UART_PROXY_DEVICE_NUMBER__": len(self.dev_configs),
                      "__UART_PROXY_BUFFER_LENGTHS__": concat_lines(buffer_size_defs),
                      "__UART_FW_PROXY_DEV_DESCRIPTOR__": ", ".join(fw_device_descrs),
                      "__UART_SW_PROXY_DEV_DESCRIPTOR__": ", ".join(sw_device_descrs),
                      "__UART_PROXY_ISR_HANDLERS__" : concat_lines(isr_list)[:-1]}

        self.patch_templates(vocabulary)

        return
