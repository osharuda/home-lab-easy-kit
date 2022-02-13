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
    def __init__(self, mcu_hw, dev_config):
        super().__init__(mcu_hw, dev_config, "ADCDEV")
        self.fw_header = "fw_adcdev.h"
        self.sw_header = "sw_adcdev.h"
        self.shared_header = "adc_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__ADC_SHARED_HEADER__")

    def sanity_checks(self, dev_config: dict, adcdev_requires: dict, dev_name : str):
        if "sample_time" not in dev_config.keys():
            raise RuntimeError("sample_time is not specified for device {0}".format(dev_name))

        adc_count = 0
        analog_inputs_names = set()
        analog_inputs = set()
        for rname, ritem in adcdev_requires.items():
            rtype, res = self.get_resource(ritem)
            if rtype == "adc_input":
                analog_inputs_names.add(rname)
                analog_inputs.add(res)
            elif rtype == "adc":
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
        sw_device_descrs = []
        buffer_defs = []
        buffer_size_defs = []
        fw_device_analog_inputs = []
        sw_device_analog_inputs = []
        adc_isr_list = []
        dma_isr_list = []
        timer_irq_handler_list = []
        index = 0
        adc_maxval = self.mcu_hw.get_ADC_MAXVAL();


        for dev_name, dev_config in self.device_list:
            adcdev_requires = dev_config["requires"]
            dev_id = dev_config["dev_id"]
            buffer_size = dev_config["buffer_size"]
            use_dma = dev_config["use_dma"] != 0
            adc_input_count = 0
            timer_count = 0
            dma_channel = "0"
            dma = "0"
            dma_it = "0"
            dr_address = "0"
            fw_analog_inputs = []
            sw_analog_inputs = []

            vref = dev_config["vref"]

            self.sanity_checks(dev_config, adcdev_requires, dev_name)

            for rname, ritem in adcdev_requires.items():

                rtype, res = self.get_resource(ritem)
                if rtype == "adc_input":
                    adc_input_count += 1
                    try:
                        gpio = self.mcu_hw.ADCChannel_to_GPIO(res)
                        channel_port = self.mcu_hw.GPIO_to_port(gpio)
                        channel_pin = self.mcu_hw.GPIO_to_pin_mask(gpio)
                        sample_time = self.get_sample_time(dev_config, dev_name, rname)

                        # Add requirements for GPIO pin
                        ritem["gpio"] = gpio
                    except KeyError:
                        channel_port = "0"
                        channel_pin = "0"
                        sample_time = "0"

                    fw_analog_inputs.append("{{ {0}, {1}, {2}, {3} }}".format(channel_port, channel_pin, res, sample_time))
                    sw_analog_inputs.append('{{ "{0}", "{1}" }}'.format(rname, res))

                elif rtype == "adc":
                    adc = res
                elif rtype == "timer":
                    timer_count += 1
                    timer = self.get_timer(ritem)
                else:
                    raise RuntimeError("Wrong device specified in {0} requirements".format(dev_name))

            if adc_input_count == 0:
                raise RuntimeError("Device {0} must have at least one adc_input".format(dev_name))
            sample_block_size = adc_input_count*2

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
                adcdev_requires["DMA_IRQ"] = {"irq_handlers": irq_handler}
                adcdev_requires["DMA_CHANNEL"] = {"dma_channel": dma_channel}
            else:
                irq_handler = self.mcu_hw.ADC_to_ADCHandler(adc)
                adc_isr_list.append("MAKE_ISR_WITH_INDEX({0}, ADC_COMMON_ADC_IRQ_HANDLER, {1}) \\".format(irq_handler, index))
                adcdev_requires["ADC_IRQ"] = {"irq_handlers": irq_handler}
                print("Warning: device {0} will work in interrupt mode. DMA will not be used!".format(dev_name))


            scan_complete_irqn = self.mcu_hw.ISRHandler_to_IRQn(irq_handler)

            if not check_buffer_size_multiplicity(buffer_size, adc_input_count*2):
                print("Warning: device {0} has buffer size ({1}) not multiply to the number of inputs ({2}) * sizeof("
                      "uint16_t)".format(dev_name,
                                         buffer_size,
                                         adc_input_count))
            fw_buffer_name = "g_{0}_buffer".format(dev_name)
            fw_inputs_name = "g_{0}_inputs".format(dev_name)
            fw_device_descrs.append("{{ {{0}}, {{0}}, {{0}}, {13}, {10}, {2}, {12}, {3}, {4}, {5}, {6}, {9}, {11}, {7}, {8}, {0}, {1} }}".format(
                                                                                dev_id,             #0
                                                                                adc_input_count,    #1
                                                                                adc,                #2
                                                                                dr_address,         #3
                                                                                dma_channel,        #4
                                                                                dma,                #5
                                                                                dma_it,             #6
                                                                                self.mcu_hw.ISRHandler_to_IRQn(time_irq_handler),   #7
                                                                                scan_complete_irqn, #8
                                                                                buffer_size,        #9
                                                                                fw_buffer_name,     #10
                                                                                sample_block_size,  #11
                                                                                timer,              #12
                                                                                fw_inputs_name))    #13

            sw_device_descrs.append('{{ {0}, "{1}", {2}, {3}, {4}, {5}, {6}, {7} }}'.format(dev_id, dev_name, buffer_size, adc_input_count, self.mcu_hw.get_TIMER_freq(timer), vref, adc_maxval, fw_inputs_name))
            fw_device_buffers.append("volatile uint8_t {0}[{1}];\\".format(fw_buffer_name, buffer_size))

            fw_device_analog_inputs.append("volatile ADCDevFwChannel {0}[]= {{ {1} }};\\".format(fw_inputs_name,
                                                                                                 ", ".join(fw_analog_inputs)))

            sw_device_analog_inputs.append("const ADCInput {0}[]= {{ {1} }};\\".format(fw_inputs_name,
                                                                                                 ", ".join(sw_analog_inputs)))


            index += 1

        vocabulary = {"__ADCDEV_BUFFERS__": concat_lines(buffer_defs),
                      "__ADCDEV_DEVICE_COUNT__": len(fw_device_descrs),
                      "__ADCDEV_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descrs),
                      "__ADCDEV_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_descrs),
                      "__ADCDEV_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__ADCDEV_FW_CHANNELS__": concat_lines(fw_device_analog_inputs)[:-1],
                      "__ADCDEV_SW_CHANNELS__": concat_lines(sw_device_analog_inputs)[:-1],
                      "__ADCDEV_FW_ADC_IRQ_HANDLERS__": concat_lines(adc_isr_list)[:-1],
                      "__ADCDEV_FW_TIMER_IRQ_HANDLERS__": concat_lines(timer_irq_handler_list)[:-1],
                      "__ADCDEV_FW_DMA_IRQ_HANDLERS__": concat_lines(dma_isr_list)[:-1]}

        self.patch_templates(vocabulary)
