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
from keywords import *


class TimeTrackerDevCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, common_config):
        super().__init__(mcu_hw, dev_configs, common_config, "TIMETRACKERDEV")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "timetrackerdev_conf.hpp"
        self.sw_lib_source = "timetrackerdev_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header), self.shared_token)


    def customize(self):
        fw_device_descriptors = []      # these descriptors are used to configure each device on firmwire side
        sw_device_desсriptors = []      # these descriptors are used to configure each device on software side
        fw_device_buffers = []          # these buffers are used to be internal buffers for all configured devices on firmware side
        sw_configs = []
        sw_config_declarations = []
        sw_config_array_name = "timetrackerdev_configs"

        index = 0
        for dev_name, dev_config in self.device_list:

            self.require_feature("SYSTICK", dev_config)

            dev_requires = dev_config[KW_REQUIRES]
            dev_id       = dev_config[KW_DEV_ID]
            buffer_size  = dev_config[KW_BUFFER_SIZE]


            interrupt_pin = self.get_gpio(get_param(dev_requires, KW_INTERRUPT, f' for device "{dev_name}"'))
            exti_line = self.mcu_hw.GPIO_to_EXTI_line(interrupt_pin)
            dev_requires[f"exti_line_{dev_name}"] = {RT_EXTI_LINE: exti_line}
            exti_control_reg = self.mcu_hw.GPIO_to_AFIO_EXTICR(interrupt_pin)

            exti_type = get_param(dev_config, KW_PULL, f' for device "{dev_name}"', self.mcu_hw.pull_type_map)
            exti_raise, exti_fall = get_param(dev_config, KW_TRIGGER, f' for device "{dev_name}"',
                                              self.mcu_hw.trigger_type_map)

            near_full_pin = self.get_gpio(get_param(dev_requires, KW_NEAR_FULL, f' for device "{dev_name}"'))
            near_full_pin_type = self.mcu_hw.out_type_map[KW_PUSH_PULL]


            # Configure buffer
            fw_buffer_name = "g_{0}_buffer".format(dev_name)
            exti_pin_descr = self.mcu_hw.GPIO_to_GPIO_Descr(interrupt_pin, exti_type, 0)
            near_full_pin_descr = self.mcu_hw.GPIO_to_GPIO_Descr(near_full_pin, near_full_pin_type, 0)
            fw_device_descriptors.append(f"""{{ {{0}}, {{0}}, {{ {{0}}, 0}}, {exti_pin_descr}, {near_full_pin_descr}, {fw_buffer_name}, {buffer_size}, {exti_control_reg}, {dev_id}, {exti_raise}, {exti_fall} }}""")
            sw_device_desсriptors.append(f"""{{ {dev_id}, "{dev_name}", {buffer_size}, {self.mcu_hw.systick_frequency()} }}""")
            fw_device_buffers.append("volatile uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))

            # libconfig
            sw_config_name = "timetrackerdev_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const TimeTrackerDevConfig* {sw_config_name};")
            sw_configs.append(
                f"const TimeTrackerDevConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__TIMETRACKERDEV_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__TIMETRACKERDEV_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__TIMETRACKERDEV_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__TIMETRACKERDEV_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__TIMETRACKERDEV_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__TIMETRACKERDEV_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__TIMETRACKERDEV_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
        }

        self.patch_templates()
