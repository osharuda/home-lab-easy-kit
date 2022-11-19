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


class IRRCCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, common_config):
        super().__init__(mcu_hw, dev_configs, common_config, "IRRC")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "irrc_conf.hpp"
        self.sw_lib_source = "irrc_conf.cpp"

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

    def customize(self):
        self.require_feature("SYSTICK", self.dev_config)
        irrc_requires = self.dev_config["requires"]
        data_pin = self.get_gpio(irrc_requires["data"])
        irrc_requires["exti_line_irrc"] = {"exti_line": self.mcu_hw.GPIO_to_EXTI_line(data_pin)}
        data_pin_number = self.mcu_hw.GPIO_to_pin_number(data_pin)

        vocabulary = {"__NAMESPACE_NAME__": self.project_name.lower(),
                      "__DEVICE_ID__": self.dev_config["dev_id"],
                      "__IRRC_DEVICE_NAME__": self.device_name,
                      "__IRRC_BUF_LEN__": self.dev_config["buffer_size"],

                      "__IRRC_OUT_PORT__": self.mcu_hw.GPIO_to_port(data_pin),
                      "__IRRC_OUT_PIN__": data_pin_number,
                      "__IRRC_OUT_PIN_MASK__": self.mcu_hw.GPIO_to_pin_mask(data_pin),
                      "__IRRC_EXTICR__": self.mcu_hw.GPIO_to_AFIO_EXTICR(data_pin)}

        self.patch_templates(vocabulary)
