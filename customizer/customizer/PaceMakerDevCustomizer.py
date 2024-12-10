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
from keywords import *
from tools import *


class PaceMakerDevCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, common_config):
        super().__init__(mcu_hw, dev_configs, common_config, "PACEMAKERDEV")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "pacemakerdev_conf.hpp"
        self.sw_lib_source = "pacemakerdev_conf.cpp"

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
        sw_config_array_name = "pacemakerdev_configs"
        fw_set_gpio_functions = []
        fw_set_gpio_headers = []
        fw_init_gpio_headers = []
        fw_init_gpio_functions = []
        sw_dev_namespace_declarations = []
        sw_dev_namespace_values = []
        main_timer_irq_handler_list = []
        internal_timer_irq_handler_list = []

        index = 0
        for dev_name, dev_config in self.device_list:
            check_items(dev_config,
                       {KW_REQUIRES: dict, KW_DEV_ID: int, KW_MAX_SAMPLES: int, KW_SIGNALS: dict})

            dev_requires = dev_config[KW_REQUIRES]
            dev_id       = dev_config[KW_DEV_ID]
            max_samples = dev_config[KW_MAX_SAMPLES]
            signals      = dev_config[KW_SIGNALS]

            sw_dev_namespace_declarations.append(f"{self.tab}namespace {dev_name} {{")
            sw_dev_namespace_values.append(f"{self.tab}namespace {dev_name} {{")

            sw_signal_descriptors = []

            check_items(dev_requires, {KW_MAIN_TIMER: dict, KW_INT_TIMER: dict})
            rtype, main_timer = self.unpack_resource(self.get_requirement(dev_requires, KW_MAIN_TIMER, RT_TIMER))
            main_time_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(main_timer)
            main_timer_irq_handler_list.append(
                "MAKE_ISR_WITH_INDEX({0}, PACEMAKER_MAIN_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(main_time_irq_handler, index))
            dev_requires["MAIN_TIMER_IRQ"] = {"irq_handlers": main_time_irq_handler}

            rtype, internal_timer = self.unpack_resource(self.get_requirement(dev_requires, KW_INT_TIMER, RT_TIMER))
            internal_time_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(internal_timer)
            internal_timer_irq_handler_list.append(
                "MAKE_ISR_WITH_INDEX({0}, PACEMAKER_INTERNAL_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(internal_time_irq_handler, index))
            dev_requires["INTERNAL_TIMER_IRQ"] = {"irq_handlers": internal_time_irq_handler}

            signal_index = 0
            default_pin_state = 0;
            data_to_pin = dict()
            for s in signals.items():
                sig_name, sig_pin, sig_pin_type, sig_pin_default = self.get_out_signal(s)
                sig_port = self.mcu_hw.GPIO_to_port(sig_pin)
                sig_pin_num = self.mcu_hw.GPIO_to_pin_number(sig_pin)

                # Software signal descriptions
                sw_signal_descriptors.append(f'{{ signal_{sig_name}_mask, signal_{sig_name}_default, signal_{sig_name}_name }}')
                sw_dev_namespace_declarations.append(f'{self.tab*2}constexpr size_t signal_{sig_name}_index = {signal_index};')
                sw_dev_namespace_declarations.append(f'{self.tab*2}constexpr size_t signal_{sig_name}_mask = {1 << signal_index};')
                sw_dev_namespace_declarations.append(f'{self.tab*2}constexpr int signal_{sig_name}_default = {sig_pin_default};')
                sw_dev_namespace_declarations.append(f'{self.tab*2}extern const char* signal_{sig_name}_name;')
                sw_dev_namespace_values.append(f'{self.tab*2}const char* signal_{sig_name}_name = "{sig_name}";')
                default_pin_state |= (sig_pin_default << signal_index)

                # Add pin to set pin generation dictionary used to generate set gpio function
                data_to_pin[signal_index] = (sig_port, sig_pin_num, sig_pin_type, sig_pin_default)

                # Add pin requirement and increment pin index
                self.add_requirement(dev_requires, {sig_name: {RT_GPIO: sig_pin}})
                signal_index += 1

            # Add number of the signals and default state
            sw_dev_namespace_declarations.append(f'{self.tab*2}constexpr size_t signals_number = {signal_index};')
            sw_dev_namespace_declarations.append(f'{self.tab * 2}constexpr size_t signals_default_state = {default_pin_state};')

            # Generate functions to set gpio
            set_gpio_func_name = f"pacemaker_set_gpio_{index}"
            header, macro = self.mcu_hw.generate_set_gpio_bus_function(f"{set_gpio_func_name}",
                                                                       "uint32_t",
                                                                       "uint16_t",
                                                                       data_to_pin)
            fw_set_gpio_headers.append(header)
            fw_set_gpio_functions.append(macro)

            init_gpio_func_name = f"pacemaker_init_gpio_{index}"
            header, macro = self.mcu_hw.generate_init_gpio_bus_function(f"{init_gpio_func_name}",
                                                                       "uint16_t",
                                                                       data_to_pin)
            fw_init_gpio_headers.append(header)
            fw_init_gpio_functions.append(macro)


            fw_buffer_name = "g_{0}_buffer".format(dev_name)

            buffer_size  = f"( sizeof(struct PaceMakerStatus) + {max_samples}*sizeof(struct PaceMakerTransition) )"
            fw_device_descriptors.append(f"""{{    {{0}}, \\
    {{ {{0}}, 0, 0, 0, 0, 0, 0 }}, \\
    {self.mcu_hw.get_TIMER_definition(main_timer)}, \\
    {self.mcu_hw.get_TIMER_definition(internal_timer)}, \\
    {init_gpio_func_name}, \\
    {set_gpio_func_name}, \\
    {fw_buffer_name}, \\
    {default_pin_state}, \\
    {buffer_size}, \\
    {dev_id} }} \\
""")

            fw_device_buffers.append("uint8_t {0}[{1} + sizeof(struct PaceMakerStatus)];\\".format(fw_buffer_name, buffer_size))

            sw_device_desсriptors.append(f'{{ {dev_id}, "{dev_name}", {buffer_size}, {self.mcu_hw.get_TIMER_freq(main_timer)}, {self.mcu_hw.get_TIMER_freq(internal_timer)}, {signal_index}, {default_pin_state}, {max_samples} }}')
            sw_config_name = f'pacemaker_{dev_name}_config'
            sw_config_declarations.append(f"{self.tab}extern const struct PaceMakerDevConfig* {sw_config_name};")
            sw_configs.append(
                f"const struct PaceMakerDevConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            sw_dev_namespace_declarations.append(f"{self.tab}}}")
            sw_dev_namespace_values.append(f"{self.tab}}}")
            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__PACEMAKERDEV_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__PACEMAKERDEV_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__PACEMAKERDEV_SW_DEV_NAMESPACE_DECLARATIONS__": self.newline.join(sw_dev_namespace_declarations),
                      "__PACEMAKERDEV_SW_DEV_NAMESPACE_VALUES__": self.newline.join(sw_dev_namespace_values),
                      "__PACEMAKERDEV_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__PACEMAKERDEV_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__PACEMAKERDEV_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__PACEMAKERDEV_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__PACEMAKERDEV_CONFIGURATION_ARRAY_NAME__": sw_config_array_name,
                      "__PACEMAKERDEV_FW_SET_GPIO_HEADERS__": ("\\" + self.newline).join(fw_set_gpio_headers),
                      "__PACEMAKERDEV_FW_SET_GPIO_FUNCTIONS__": ("\\" + self.newline).join(fw_set_gpio_functions),
                      "__PACEMAKERDEV_FW_INIT_GPIO_HEADERS__": ("\\" + self.newline).join(fw_init_gpio_headers),
                      "__PACEMAKERDEV_FW_INIT_GPIO_FUNCTIONS__": ("\\" + self.newline).join(fw_init_gpio_functions),
                      "__PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS__": concat_lines(main_timer_irq_handler_list)[:-1],
                      "__PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS__": concat_lines(internal_timer_irq_handler_list)[:-1]
        }

        self.patch_templates()
