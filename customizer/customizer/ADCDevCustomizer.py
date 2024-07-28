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


class ADCDevCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "ADCDEV")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "adc_conf.hpp"
        self.sw_lib_source = "adc_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header), self.shared_token)

    def sanity_checks(self, dev_config: dict, adcdev_requires: dict, dev_name : str):
        if "sample_time" not in dev_config.keys():
            raise RuntimeError("sample_time is not specified for device {0}".format(dev_name))

        adc_count = 0
        analog_inputs_names = set()
        analog_inputs = set()
        for rname, ritem in adcdev_requires.items():
            rtype, res = self.unpack_resource(ritem)
            if rtype == RT_ADC_INPUT:
                analog_inputs_names.add(rname)
                analog_inputs.add(res)
            elif rtype == RT_ADC:
                adc_count += 1
                adc = res

        if adc_count != 1:
            raise RuntimeError("Device {0} must have one and only adc".format(dev_name))

        for i in analog_inputs:
            if "use_adc" in self.mcu_hw.mcu_resources[i]:
                use_adc = self.mcu_hw.mcu_resources[i]["use_adc"]
                if use_adc != adc:
                    raise RuntimeError("Channel {0} must be used with {1}. Change ADC in device {2}".format(i,use_adc,dev_name))

        sample_time_dict = dev_config["sample_time"]

        if len(sample_time_dict)!=2 or ("default" not in sample_time_dict) or ("override" not in sample_time_dict):
            raise RuntimeError("sample_time definition for {0} must have two items specified: 'default' and 'override'".format(dev_name))

        tmp = sample_time_dict["default"]
        if not self.mcu_hw.check_ADC_sample_time(tmp):
            raise RuntimeError("Wrong sample time {0} specified for device {1} in default section".format(tmp, dev_name))

        for input_name, sample_time in sample_time_dict["override"].items():
            if not self.mcu_hw.check_ADC_sample_time(sample_time):
                raise RuntimeError("Wrong sample time {0} specified for device {1} in override section for {2}".format(sample_time, dev_name, input_name))

            if input_name not in analog_inputs_names:
                raise RuntimeError("Input {0} listed in sample_times.override section is not listed among inputs for device {1}".format(input_name, dev_name))

    def get_sample_time(self, dev_config, dev_name, input_name) -> str:
        res = dev_config["sample_time"]["default"]
        if input_name in dev_config["sample_time"]["override"]:
            res = dev_config["sample_time"]["override"][input_name]

        if not self.mcu_hw.check_ADC_sample_time(res):
            raise RuntimeError("Wrong sample time {0} specified for input {1} in device {2}".format(res, input_name, dev_name))

        return res


    def customize(self):
        fw_device_descrs = []
        fw_device_buffers = []      # these buffers are used to buffer all samples
        fw_device_measurement_buffers = []
        fw_device_sample_time_buffers = []
        fw_device_accumulator_buffers = []
        sw_device_descrs = []
        buffer_defs = []
        fw_device_analog_inputs = []
        sw_device_analog_inputs = []
        sw_config_declarations = []
        sw_config_array_name = "adc_configs"
        sw_configs = []
        adc_isr_list = []
        dma_isr_list = []
        timer_irq_handler_list = []
        index = 0
        adc_maxval = self.mcu_hw.get_ADC_MAXVAL();
        sample_size = 2         # result of averaging will be uint16_t

        for dev_name, dev_config in self.device_list:
            adcdev_requires = dev_config[KW_REQUIRES]
            dev_id = dev_config[KW_DEV_ID]
            measurements_per_sample = dev_config["measurements_per_sample"]
            use_dma = dev_config["use_dma"] != 0
            adc_input_number = 0
            timer_count = 0
            dma_channel = "0"
            dma = "0"
            dma_it = "0"
            dr_address = "0"
            fw_analog_inputs = []
            sw_analog_inputs = []

            self.sanity_checks(dev_config, adcdev_requires, dev_name)

            for rname, ritem in adcdev_requires.items():

                rtype, res = self.unpack_resource(ritem)
                if rtype == RT_ADC_INPUT:
                    adc_input_number += 1
                    try:
                        gpio = self.mcu_hw.ADCChannel_to_GPIO(res)
                        channel_port = self.mcu_hw.GPIO_to_port(gpio)
                        channel_pin = self.mcu_hw.GPIO_to_pin_mask(gpio)
                        sample_time = self.get_sample_time(dev_config, dev_name, rname)

                        # Add requirements for GPIO pin
                        ritem[RT_GPIO] = gpio
                    except KeyError:
                        channel_port = "0"
                        channel_pin = "0"
                        sample_time = "0"

                    fw_analog_inputs.append("{{ {0}, {1}, {2}, {3} }}".format(channel_port, channel_pin, res, sample_time))
                    sw_analog_inputs.append('{{ "{0}", "{1}", {2}    }}'.format(rname, res, sample_time))

                elif rtype == RT_ADC:
                    adc = res
                elif rtype == RT_TIMER:
                    timer_count += 1
                    timer = self.get_timer(ritem)
                else:
                    raise RuntimeError("Wrong device specified in {0} requirements".format(dev_name))

            if adc_input_number == 0:
                raise RuntimeError("Device {0} must have at least one adc_input".format(dev_name))
            sample_block_size = adc_input_number*sample_size
            buffer_size = dev_config[KW_BUFFER_SIZE] * sample_block_size;

            if timer_count == 0:
                raise RuntimeError("Device {0} must have one timer assigned".format(dev_name))

            if timer_count > 1:
                raise RuntimeError("Device {0} must have only one timer assigned".format(dev_name))

            time_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(timer)
            timer_irq_handler_list.append("MAKE_ISR_WITH_INDEX({0}, ADC_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(time_irq_handler, index))
            adcdev_requires["TIMER_IRQ"] = {"irq_handlers": time_irq_handler}

            try:
                self.check_res_feature(adc, "dma_support")
            except RuntimeError:
                if use_dma:
                    print("Warning: device {0} is instructed to use DMA, but {1} doesn't support DMA.".format(dev_name, adc))
                use_dma = False

            if use_dma:
                dma_channel = self.mcu_hw.get_DMA_Channel(adc)
                dma = self.mcu_hw.DMA_from_DMA_Channel(dma_channel)
                dma_it = self.mcu_hw.get_DMA_IT_Flag(dma_channel, "TC")
                dr_address = self.mcu_hw.get_DMA_DR_for_resource(adc)
                irq_handler = self.mcu_hw.DMA_Channel_to_IRQHandler(dma_channel)
                dma_isr_list.append("MAKE_ISR_WITH_INDEX({0}, ADC_COMMON_DMA_IRQ_HANDLER, {1}) \\".format(irq_handler, index))
                adcdev_requires["DMA_IRQ"] = {RT_IRQ_HANDLER: irq_handler}
                adcdev_requires["DMA_CHANNEL"] = {RT_DMA_CHANNEL: dma_channel}
            else:
                irq_handler = self.mcu_hw.ADC_to_ADCHandler(adc)
                adc_isr_list.append("MAKE_ISR_WITH_INDEX({0}, ADC_COMMON_ADC_IRQ_HANDLER, {1}) \\".format(irq_handler, index))
                adcdev_requires["ADC_IRQ"] = {RT_IRQ_HANDLER: irq_handler}
                print("Warning: device {0} will work in interrupt mode!".format(dev_name))


            scan_complete_irqn = self.mcu_hw.ISRHandler_to_IRQn(irq_handler)

            if not check_buffer_size_multiplicity(buffer_size, sample_block_size):
                samples_per_channel = ( buffer_size // sample_block_size ) + 1
                new_buffer_size = samples_per_channel * sample_block_size
                print(f'Warning: device {dev_name} has buffer size ({buffer_size}) not multiply to the number of inputs ({adc_input_number}) * sizeof('
                      'uint16_t)')
                print(f'Buffer size is increased to {new_buffer_size}')
                buffer_size = new_buffer_size
            fw_buffer_name = "g_{0}_buffer".format(dev_name)
            fw_inputs_name = "g_{0}_inputs".format(dev_name)

            fw_measurement_buffer_size = measurements_per_sample * adc_input_number
            fw_measurement_buffer_name = "g_{0}_measurement_buffer".format(dev_name)
            fw_sample_time_buffer_name = "g_{0}_sample_time_buffer".format(dev_name)
            fw_accumulator_buffer_name = "g_{0}_accumulator_buffer".format(dev_name)

            fw_device_descrs.append(f"""{{\\
    {self.device_context_initializer},\\
    {self.circular_buffer_initializer},\\
    {{0}},\\
    {fw_inputs_name},\\
    {fw_measurement_buffer_name},\\
    {fw_sample_time_buffer_name},\\
    {fw_accumulator_buffer_name},\\
    {fw_buffer_name},\\
    {adc},\\
    {timer},\\
    {dr_address},\\
    {dma_channel},\\
    {dma},\\
    {dma_it},\\
    {buffer_size},\\
    {sample_block_size},\\
    {measurements_per_sample}, \\
    {self.mcu_hw.ISRHandler_to_IRQn(time_irq_handler)},\\
    {scan_complete_irqn},\\
    {dev_id},\\
    {adc_input_number} }}""")


            sw_device_descrs.append(f"""{{  \\
    {dev_id},                              /* Device Id */ \\
    "{dev_name}",                          /* Device name */ \\
    {buffer_size},                         /* Buffer size */ \\
    {adc_input_number},                    /* Number of ADC inputs*/ \\
    {measurements_per_sample},             /* Number of measurements to be averaged per sample */
    {self.mcu_hw.get_TIMER_freq(timer)},   /* Timer frequency */ \\
    {adc_maxval},                          /* ADC maximum value */ \\
    {fw_inputs_name}                       /* Inputs */ }}""")

            # Device data arrays
            fw_device_buffers.append("uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))
            fw_device_measurement_buffers.append("uint16_t {0}[{1}];\\".format(fw_measurement_buffer_name, fw_measurement_buffer_size))
            fw_device_analog_inputs.append("struct ADCDevFwChannel {0}[]= {{ {1} }};\\".format(fw_inputs_name,
                                                                                                 ", ".join(fw_analog_inputs)))
            fw_device_sample_time_buffers.append("uint8_t {0}[{1}];\\".format(fw_sample_time_buffer_name, adc_input_number))
            fw_device_accumulator_buffers.append("uint32_t {0}[{1}];\\".format(fw_accumulator_buffer_name, adc_input_number))


            # Software data arrays
            sw_device_analog_inputs.append("const struct ADCInput {0}[]= {{ {1} }};\\".format(fw_inputs_name,
                                                                                                 ", ".join(sw_analog_inputs)))
            sw_config_name = "adc_{0}_config".format(dev_name)
            sw_config_declarations.append(f"extern const struct ADCConfig* {sw_config_name};")
            sw_configs.append(f"const struct ADCConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__ADCDEV_BUFFERS__": concat_lines(buffer_defs),
                      "__ADCDEV_DEVICE_COUNT__": len(fw_device_descrs),
                      "__ADCDEV_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descrs),
                      "__ADCDEV_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_descrs),
                      "__ADCDEV_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__ADCDEV_FW_MEASUREMENT_BUFFERS__": concat_lines(fw_device_measurement_buffers)[:-1],
                      "__ADCDEV_FW_ACCUMULATOR_BUFFERS__": concat_lines(fw_device_accumulator_buffers)[:-1],
                      "__ADCDEV_FW_SAMPLE_TIME_BUFFERS__": concat_lines(fw_device_sample_time_buffers)[:-1],
                      "__ADCDEV_FW_CHANNELS__": concat_lines(fw_device_analog_inputs)[:-1],
                      "__ADCDEV_SW_CHANNELS__": concat_lines(sw_device_analog_inputs)[:-1],
                      "__ADCDEV_FW_ADC_IRQ_HANDLERS__": concat_lines(adc_isr_list)[:-1],
                      "__ADCDEV_FW_TIMER_IRQ_HANDLERS__": concat_lines(timer_irq_handler_list)[:-1],
                      "__ADCDEV_FW_DMA_IRQ_HANDLERS__": concat_lines(dma_isr_list)[:-1],

                      "__ADCDEV_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__ADCDEV_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__ADCDEV_CONFIGURATION_ARRAY_NAME__": sw_config_array_name}

        self.patch_templates()
