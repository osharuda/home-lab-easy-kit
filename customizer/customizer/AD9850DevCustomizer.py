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


class AD9850DevCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "AD9850DEV")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "ad9850_conf.hpp"
        self.sw_lib_source = "ad9850_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])
        self.add_template(os.path.join(self.sw_inc_templ, self.hlek_lib_common_header),
                          [os.path.join(self.libhlek_inc_dest_path, self.hlek_lib_common_header)])

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
        fw_set_gpio_functions = []
        fw_set_gpio_headers = []
        sw_config_array_name = "ad9850_configs"
        sw_config_declarations = []
        sw_configs = []

        index = 0
        for dev_name, dev_config in self.device_list:

            dev_requires = dev_config["requires"]
            dev_id       = dev_config["dev_id"]
            clock_freq = frequency_to_int(dev_config["clock_frequency"])

            self.sanity_checks(dev_config, dev_requires, dev_name)

            rtype, d0 = self.get_resource(dev_requires["D0"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d0} specified in {dev_name}. Must be gpio.")

            rtype, d1 = self.get_resource(dev_requires["D1"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d1} specified in {dev_name}. Must be gpio.")


            rtype, d2 = self.get_resource(dev_requires["D2"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d2} specified in {dev_name}. Must be gpio.")


            rtype, d3 = self.get_resource(dev_requires["D3"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d3} specified in {dev_name}. Must be gpio.")


            rtype, d4 = self.get_resource(dev_requires["D4"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d4} specified in {dev_name}. Must be gpio.")


            rtype, d5 = self.get_resource(dev_requires["D5"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d5} specified in {dev_name}. Must be gpio.")


            rtype, d6 = self.get_resource(dev_requires["D6"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d6} specified in {dev_name}. Must be gpio.")


            rtype, d7 = self.get_resource(dev_requires["D7"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {d7} specified in {dev_name}. Must be gpio.")


            rtype, w_clk = self.get_resource(dev_requires["W_CLK"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {w_clk} specified in {dev_name}. Must be gpio.")

            rtype, fq_ud = self.get_resource(dev_requires["FQ_UD"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {fq_ud} specified in {dev_name}. Must be gpio.")

            rtype, reset = self.get_resource(dev_requires["RESET"])
            if rtype != "gpio":
                raise ValueError(f"Invalid resource {reset} specified in {dev_name}. Must be gpio.")

            # generate gpio update function for device
            set_gpio_data_func_name = f"ad9850_set_data_gpio_{index}"
            data_to_pin = { 0: (self.mcu_hw.GPIO_to_port(d0), self.mcu_hw.GPIO_to_pin_number(d0)),
                            1: (self.mcu_hw.GPIO_to_port(d1), self.mcu_hw.GPIO_to_pin_number(d1)),
                            2: (self.mcu_hw.GPIO_to_port(d2), self.mcu_hw.GPIO_to_pin_number(d2)),
                            3: (self.mcu_hw.GPIO_to_port(d3), self.mcu_hw.GPIO_to_pin_number(d3)),
                            4: (self.mcu_hw.GPIO_to_port(d4), self.mcu_hw.GPIO_to_pin_number(d4)),
                            5: (self.mcu_hw.GPIO_to_port(d5), self.mcu_hw.GPIO_to_pin_number(d5)),
                            6: (self.mcu_hw.GPIO_to_port(d6), self.mcu_hw.GPIO_to_pin_number(d6)),
                            7: (self.mcu_hw.GPIO_to_port(d7), self.mcu_hw.GPIO_to_pin_number(d7))}
            header, macro = self.mcu_hw.generate_set_gpio_bus_function(f"{set_gpio_data_func_name}", "uint8_t", "uint16_t", data_to_pin)

            fw_set_gpio_headers.append(header)
            fw_set_gpio_functions.append(macro)

            fw_device_descriptors.append(f"""{{\\
    {{0}},\\
    {{ {{ .W0=0, 0, 0, 0, 0 }} }},\\
    {set_gpio_data_func_name},\\
    {self.mcu_hw.GPIO_to_port(d0)},\\
    {self.mcu_hw.GPIO_to_port(d1)},\\
    {self.mcu_hw.GPIO_to_port(d2)},\\
    {self.mcu_hw.GPIO_to_port(d3)},\\
    {self.mcu_hw.GPIO_to_port(d4)},\\
    {self.mcu_hw.GPIO_to_port(d5)},\\
    {self.mcu_hw.GPIO_to_port(d6)},\\
    {self.mcu_hw.GPIO_to_port(d7)},\\
    {self.mcu_hw.GPIO_to_port(reset)},\\
    {self.mcu_hw.GPIO_to_port(fq_ud)},\\
    {self.mcu_hw.GPIO_to_port(w_clk)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d0)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d1)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d2)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d3)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d4)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d5)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d6)},\\
    {self.mcu_hw.GPIO_to_pin_mask(d7)},\\
    {self.mcu_hw.GPIO_to_pin_mask(reset)},\\
    {self.mcu_hw.GPIO_to_pin_mask(fq_ud)},\\
    {self.mcu_hw.GPIO_to_pin_mask(w_clk)},\\
     {dev_id} }}""")

            sw_device_desсriptors.append(f"""{{\\
    {dev_id},\\
    "{dev_name}",\\
    {clock_freq} \\
    }}""")

            sw_config_name = "ad9850_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const AD9850Config* {sw_config_name};")
            sw_configs.append(
                f"const AD9850Config* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        vocabulary = {"__NAMESPACE_NAME__": self.project_name.lower(),
                      "__AD9850_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__AD9850_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__AD9850_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__AD9850_FW_SET_DATA_HEADERS__": ("\\" + self.newline).join(fw_set_gpio_headers),
                      "__AD9850_FW_SET_DATA_FUNCTIONS__": ("\\" + self.newline).join(fw_set_gpio_functions),
                      "__AD9850_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__AD9850_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__AD9850_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
                      }

        self.patch_templates(vocabulary)
