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


class {DevName}Customizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, common_config):
        super().__init__(mcu_hw, dev_configs, common_config, "{DEVNAME}")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "{devname}_conf.hpp"
        self.sw_lib_source = "{devname}_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header), self.shared_token)

    def sanity_checks(self, dev_config: dict, dev_requires: dict, dev_name : str):
        return


    def customize(self):
        fw_device_descriptors = []      # these descriptors are used to configure each device on firmwire side
        sw_device_desсriptors = []      # these descriptors are used to configure each device on software side
        fw_device_buffers = []          # these buffers are used to be internal buffers for all configured devices on firmware side
        sw_configs = []
        sw_config_declarations = []
        sw_config_array_name = "{devname}_configs"

        index = 0
        for dev_name, dev_config in self.device_list:

            dev_requires = dev_config["requires"]
            dev_id       = dev_config["dev_id"]
            buffer_size  = dev_config["buffer_size"]

            self.sanity_checks(dev_config, dev_requires, dev_name)

            for rdecl, ritem in dev_requires.items():
                rtype, rname = self.get_resource(ritem)

                # Custom resource handling
                if rtype == "gpio":
                    # process gpio resources here
                    pass
                elif rtype == "irq_handler":
                    # process IRQ handlers here
                    pass
                elif rtype == "exti_line":
                    # process EXTI lines
                    pass
                elif rtype == "bkp":
                    # process backup registers
                    pass
                elif rtype == "timer":
                    # process timers
                    pass
                elif rtype == "usart":
                    # process usart
                    pass
                elif rtype == "i2c":
                    # process i2c
                    pass
                elif rtype == "dma":
                    # process dma
                    pass
                elif rtype == "dma_channel":
                    # process dma_channel
                    pass
                elif rtype == "adc":
                    # process ADC
                    pass
                elif rtype == "adc_input":
                    # process ADC input
                    pass
                elif rtype == "spi":
                    # process SPI
                    pass
                else:
                    raise RuntimeError('Wrong resource specified in {0} requirements: "{1}"'.format(dev_name, rdecl))

            # Do not forget to add IRQs, DMA and other related resources
            if "__DEVICE_BUFFER_TYPE__" == "DEV_NO_BUFFER":
                # No buffers
                fw_device_descriptors.append("{{ {{0}}, {{0}}, {0} }}".format(
                    dev_id))

                sw_device_desсriptors.append('{{ {0}, "{1}", 0}}'.format(
                    dev_id, dev_name))

                fw_device_buffers = []
            elif "__DEVICE_BUFFER_TYPE__" == "DEV_LINIAR_BUFFER":
                # Buffer is present
                fw_buffer_name = "g_{0}_buffer".format(dev_name)
                fw_device_descriptors.append("{{ {{0}}, {{0}}, {2}, {1}, {0} }}".format(
                    dev_id,
                    buffer_size,
                    fw_buffer_name))

                sw_device_desсriptors.append('{{ {0}, "{1}", {2} }}'.format(
                    dev_id, dev_name, buffer_size))

                fw_device_buffers.append("volatile uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))
            elif "__DEVICE_BUFFER_TYPE__" == "DEV_CIRCULAR_BUFFER":
                # Buffer is present
                fw_buffer_name = "g_{0}_buffer".format(dev_name)
                fw_device_descriptors.append("{{ {{0}}, {{0}}, {{0}}, {2}, {1}, {0} }}".format(
                    dev_id,
                    buffer_size,
                    fw_buffer_name))

                sw_device_desсriptors.append('{{ {0}, "{1}", {2} }}'.format(
                    dev_id, dev_name, buffer_size))

                fw_device_buffers.append("volatile uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))

            sw_config_name = "{devname}_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const {DevName}Config* {sw_config_name};")
            sw_configs.append(
                f"const {DevName}Config* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__{DEVNAME}_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__{DEVNAME}_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__{DEVNAME}_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__{DEVNAME}_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__{DEVNAME}_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__{DEVNAME}_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__{DEVNAME}_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
        }

        self.patch_templates()
