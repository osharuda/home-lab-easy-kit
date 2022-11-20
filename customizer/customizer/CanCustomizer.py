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
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "CAN")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "can_conf.hpp"
        self.sw_lib_source = "can_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])
        self.add_template(os.path.join(self.sw_inc_templ, self.hlek_lib_common_header),
                          [os.path.join(self.libhlek_inc_dest_path, self.hlek_lib_common_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])
        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)

    def sanity_checks(self, dev_config: dict, dev_requires: dict, dev_name : str):
        return

    def get_can_timings(self, bitrate: int):
        timings = {
            #baud rate       (prescaller, seg1, sample p., seg2)
            10:              (200, 15, 1, 2),
            #10:              (225, 13, 1, 2),
            20:              (100, 15, 1, 2),
            50:              (40, 15, 1, 2),
            #50:              (45, 13, 1, 2),
            83:              (24, 15, 1, 2),
            #83:              (27, 13, 1, 2),
            100:              (20, 15, 1, 2),
            125:              (16, 15, 1, 2),
            #125:              (18, 13, 1, 2),
            250:              (8, 15, 1, 2),
            #250:              (9, 13, 1 ,2),
            500:              (4, 15, 1, 2),
            800:              (3, 12, 1, 2),
            1000:             (2, 15, 1, 2)
        }

        if bitrate not in timings:
            raise RuntimeError(f'Bit rate for CAN is set incorrectly: ({timings}). Supported values are: {timings.keys()}')

        seg1_to_text = {
            1: 'CAN_BS1_1tq',
            2: 'CAN_BS1_2tq',
            3: 'CAN_BS1_3tq',
            4: 'CAN_BS1_4tq',
            5: 'CAN_BS1_5tq',
            6: 'CAN_BS1_6tq',
            7: 'CAN_BS1_7tq',
            8: 'CAN_BS1_8tq',
            9: 'CAN_BS1_9tq',
            10: 'CAN_BS1_10tq',
            11: 'CAN_BS1_11tq',
            12: 'CAN_BS1_12tq',
            13: 'CAN_BS1_13tq',
            14: 'CAN_BS1_14tq',
            15: 'CAN_BS1_15tq',
            16: 'CAN_BS1_16tq'
        }

        seg2_to_text = {
            1: 'CAN_BS2_1tq',
            2: 'CAN_BS2_2tq',
            3: 'CAN_BS2_3tq',
            4: 'CAN_BS2_4tq',
            5: 'CAN_BS2_5tq',
            6: 'CAN_BS2_6tq',
            7: 'CAN_BS2_7tq',
            8: 'CAN_BS2_8tq'
        }

        smpl_to_text = {
            1: 'CAN_SJW_1tq',
            2: 'CAN_SJW_2tq',
            3: 'CAN_SJW_3tq',
            4: 'CAN_SJW_4tq'
        }

        (pr, seg1, smpl, seg2) = timings[bitrate]
        return (pr, seg1_to_text[seg1], smpl_to_text[smpl], seg2_to_text[seg2])



    def customize(self):
        fw_device_descriptors = []      # these descriptors are used to configure each device on firmwire side
        sw_device_desсriptors = []      # these descriptors are used to configure each device on software side
        fw_device_buffers = []          # these buffers are used to be internal buffers for all configured devices on firmware side
        can_isr_list = []
        sw_config_array_name = "can_configs"
        sw_config_declarations = []
        sw_configs = []

        index = 0
        for dev_name, dev_config in self.device_list:

            self.require_feature("SYSTICK", dev_config)

            dev_requires = dev_config["requires"]
            dev_id       = dev_config["dev_id"]
            bitrate     = dev_config["bitrate"]
            buffer_size = "sizeof(CanRecvMessage)*{0}".format(dev_config["buffered_msg_count"])

            (can_prescaller, can_seg1, can_sample_point, can_seg2) = self.get_can_timings(bitrate)

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
            fw_device_descriptors.append("{{ {{0}}, {{0}}, {{ {{0}}, {{{{0}}}} }}, {2}, {3}, {5}, {7}, {1}, {13}, {9}, {10}, {11}, {12}, {14}, {15}, {16}, {4}, {6}, {8}, {0} }}".format(
                dev_id,                                         #0
                buffer_size,                                    #1
                fw_buffer_name,                                 #2
                can_name,                                       #3
                int(remap),                                     #4
                canrx_port,                                     #5
                canrx_pin,                                      #6
                cantx_port,                                     #7
                cantx_pin,                                      #8
                self.mcu_hw.ISRHandler_to_IRQn(tx_handler),     #9
                self.mcu_hw.ISRHandler_to_IRQn(rx0_handler),    #10
                self.mcu_hw.ISRHandler_to_IRQn(rx1_handler),    #11
                self.mcu_hw.ISRHandler_to_IRQn(sce_handler),    #12
                can_prescaller,                                 #13
                can_seg1,                                       #14
                can_sample_point,                               #15
                can_seg2))                                      #16

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

            sw_config_name = "can_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const CANConfig* {sw_config_name};")
            sw_configs.append(
                f"const CANConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        vocabulary = {"__NAMESPACE_NAME__": self.project_name,
                      "__CAN_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__CAN_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__CAN_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__CAN_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__CAN_FW_IRQ_HANDLERS__": concat_lines(can_isr_list)[:-1],
                      "__CAN_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__CAN_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__CAN_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
                      }

        self.patch_templates(vocabulary)
