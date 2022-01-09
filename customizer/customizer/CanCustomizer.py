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


class CanCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "CAN")
        self.fw_header = "fw_can.h"
        self.sw_header = "sw_can.h"
        self.shared_header = "can_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__CAN_SHARED_HEADER__")

    def sanity_checks(self, dev_config: dict, dev_requires: dict, dev_name : str):
        return


    def customize(self):
        fw_device_descriptors = []      # these descriptors are used to configure each device on firmwire side
        sw_device_desсriptors = []      # these descriptors are used to configure each device on software side
        fw_device_buffers = []          # these buffers are used to be internal buffers for all configured devices on firmware side
        can_isr_list = []

        index = 0
        for dev_name, dev_config in self.device_list:

            dev_requires = dev_config["requires"]
            dev_id       = dev_config["dev_id"]
            buffer_size = "sizeof(CanRecvMessage)*{0}".format(dev_config["buffered_msg_count"])

            self.sanity_checks(dev_config, dev_requires, dev_name)
            can = dev_requires["can"]
            can_name, remap = self.mcu_hw.is_remaped(can)
            self.check_requirements(can, dev_requires, "dev_{0}".format(dev_name))

            canrx = self.get_required_resource(can, "CANRX", "gpio")
            cantx = self.get_required_resource(can, "CANTX", "gpio")


            canrx_port = self.mcu_hw.GPIO_to_port(canrx)
            canrx_pin = self.mcu_hw.GPIO_to_pin_number(canrx)

            cantx_port = self.mcu_hw.GPIO_to_port(cantx)
            cantx_pin = self.mcu_hw.GPIO_to_pin_number(cantx)

            tx_handler = self.mcu_hw.mcu_resources[can]["irq_tx_handler"]["irq_handler"]
            rx0_handler = self.mcu_hw.mcu_resources[can]["irq_rx0_handler"]["irq_handler"]
            rx1_handler = self.mcu_hw.mcu_resources[can]["irq_rx1_handler"]["irq_handler"]
            sce_handler = self.mcu_hw.mcu_resources[can]["irq_sce_handler"]["irq_handler"]

            fw_buffer_name = "g_{0}_buffer".format(dev_name)
            fw_device_descriptors.append("{{ {0}, {1}, {2}, {{0}}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12}, {{0}}, {{ {{0}}, {{{{0}}}} }} }}".format(
                dev_id, buffer_size, fw_buffer_name,
                can_name,
                int(remap),
                canrx_port, canrx_pin,
                cantx_port, cantx_pin,
                self.mcu_hw.ISRHandler_to_IRQn(tx_handler),
                self.mcu_hw.ISRHandler_to_IRQn(rx0_handler),
                self.mcu_hw.ISRHandler_to_IRQn(rx1_handler),
                self.mcu_hw.ISRHandler_to_IRQn(sce_handler)))

            can_isr_list.append("MAKE_ISR_WITH_INDEX({0}, CAN_COMMON_TX_IRQ_HANDLER, {1}) \\".format(tx_handler, index))
            can_isr_list.append("MAKE_ISR_WITH_INDEX({0}, CAN_COMMON_RX0_IRQ_HANDLER, {1}) \\".format(rx0_handler, index))
            can_isr_list.append("MAKE_ISR_WITH_INDEX({0}, CAN_COMMON_RX1_IRQ_HANDLER, {1}) \\".format(rx1_handler, index))
            can_isr_list.append("MAKE_ISR_WITH_INDEX({0}, CAN_COMMON_SCE_IRQ_HANDLER, {1}) \\".format(sce_handler, index))

            dev_requires["TX_IRQ"] = {"irq_handlers": tx_handler}
            dev_requires["RX0_IRQ"] = {"irq_handlers": rx0_handler}
            dev_requires["RX1_IRQ"] = {"irq_handlers": rx1_handler}
            dev_requires["SCE_IRQ"] = {"irq_handlers": sce_handler}

            sw_device_desсriptors.append('{{ {0}, "{1}", {2} }}'.format(
                dev_id, dev_name, buffer_size))

            fw_device_buffers.append("volatile uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))

            index += 1

        vocabulary = {"__CAN_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__CAN_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__CAN_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__CAN_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__CAN_FW_IRQ_HANDLERS__": concat_lines(can_isr_list)[:-1]}

        self.patch_templates(vocabulary)
