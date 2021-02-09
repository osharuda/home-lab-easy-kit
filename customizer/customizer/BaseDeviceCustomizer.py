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

import os

class BaseDeviceCustomizer:
    def __init__(self, mcu_hw, dev_configs):
        self.mcu_hw = mcu_hw
        self.dev_configs = dev_configs
        self.device_name = "BASE_DEVICE"

        self.fw_inc_templ = "../templates/firmware/inc/"
        self.fw_src_templ = "../templates/firmware/src/"
        self.fw_inc_dest = "../../firmware/inc/"
        self.fw_src_dest = "../../firmware/src/"
        self.shared_templ = "../templates/shared/"

        self.sw_inc_templ = "../templates/software/inc/"
        self.sw_src_templ = "../templates/software/src/"
        self.sw_inc_dest = "../../software/inc/"
        self.sw_src_dest = "../../software/src/"
        self.sw_testtool_dest = "../../software/testtool/"

        self.fw_base_header = "fw.h"
        self.sw_base_header = "sw.h"
        self.fw_header = self.fw_base_header
        self.sw_header = self.sw_base_header

        self.template_list = dict()
        self.shared_code = dict()
        self.tab = "    "

        self.check_mcu_support()

    def check_instance_count(self, count: int):
        dev_cnt = len(self.dev_configs)
        if dev_cnt > count:
            raise RuntimeError(
                "Error: {0} doesn't support {1} devices per mcu. {2} devices are supported".format(self.device_name,
                                                                                                   dev_cnt,
                                                                                                   count))

    def customize(self) -> str:
        return ""

    def add_template(self, in_file: str, out_file_list: list):
        self.template_list[in_file] = out_file_list

    def add_shared_code(self, in_file: str, token: str):
        header_separator = "/* --------------------> END OF THE TEMPLATE HEADER <-------------------- */"
        with open(os.path.abspath(in_file), 'r') as f:
            template = f.read()

            # cut the file header
            indx = template.find(header_separator)

            self.shared_code[token] = template[indx+len(header_separator):]

    def patch_templates(self, vocabulary: dict):

        for in_file, out_file_list in self.template_list.items():

            # read in_file
            with open(os.path.abspath(in_file), 'r') as f:
                template = f.read()

            # patch shared code first
            for k,v in self.shared_code.items():
                template = template.replace("{"+k+"}", v)

            # patch tokens
            template = template.format(**vocabulary)

            for fn in out_file_list:
                with open(os.path.abspath(fn), 'w') as f:
                    f.write(template)

    def get_gpio(self, d: dict) -> str:
        if (d is None) or ("gpio" not in d):
            raise RuntimeError("gpio resource is not specified")
        gpio = d["gpio"]
        self.check_resource("gpio", gpio)
        return gpio

    def get_gpio_with_default(self, d: dict, default=None) -> str:
        if (d is None) or ("gpio" not in d):
            return default
        return self.get_gpio(d)

    def get_i2c(self, d: dict) -> str:
        i2c = d["i2c"]
        self.check_resource("i2c", i2c)
        return i2c

    def get_isr(self, d: dict) -> str:
        isr = d["irq_handler"]
        self.check_resource("irq_handler", isr)
        return isr

    def get_rtc(self, d: dict) -> str:
        rtc = d["rtc"]
        self.check_resource("rtc", rtc)
        return rtc

    def get_backup_reg(self, d: dict) -> str:
        br = d["bkp"]
        self.check_resource("bkp", br)
        return br

    def get_gpio_pin_type(self, t: str):
        if t not in self.mcu_hw.mcu_gpio_pin_types:
            raise RuntimeError("Device {0} uses invalid pin type description ({1}) for {2}".format(self.device_name, t,
                                                                                                   self.mcu_hw.mcu_name))
        return t

    def get_usart(self, d: dict) -> str:
        usart = d["usart"]
        self.check_resource("usart", usart)
        return usart

    def get_timer(self, d: dict, subtype=None) -> str:
        timer = d["timer"]
        self.check_resource("timer", timer)
        t = self.mcu_hw.mcu_resources[timer]
        if subtype and t["subtype"]!=subtype:
            raise RuntimeError("Device {0} uses invalid timer with wrong subtype ({1}). subtype={2} is expected".format(
                self.device_name,
                subtype,
                t[subtype]))
        return timer

    def get_resource(self, d: dict) -> tuple:
        if len(d) != 1:
            raise RuntimeError("There should be single element")

        rtype, res = next(iter(d.items()))

        self.check_resource(rtype, res)

        return rtype, res

    def get_resource_by_name(self, name: str) -> tuple:
        if name not in self.mcu_hw.mcu_resources:
            raise RuntimeError("Unknown resource {0}".format(name))

        return (self.mcu_hw.mcu_resources[name]["type"], name)

    def get_resources(self, rtype: str) -> dict:
        result = dict()
        for rname, rval in self.mcu_hw.mcu_resources.items():
            if rval["type"]==rtype:
                result[rname]=rval
        return result


    def check_resource(self, res_type: str, res_name: str):
        if res_name not in self.mcu_hw.mcu_resources:
            raise RuntimeError("Device {0} uses invalid resource ({1}) for {2}".format(self.device_name, res_name,
                                                                                       self.mcu_hw.mcu_name))

        if self.mcu_hw.mcu_resources[res_name]["type"] != res_type:
            raise RuntimeError("Device {0} uses resource with wrong type({1} : {2}) for {3}".format(self.device_name,
                                                                                                    res_name,
                                                                                                    res_type,
                                                                                                    self.mcu_hw.mcu_name))
        return

    def check_res_feature(self, dev : str, feature : str):
        f = self.mcu_hw.mcu_resources[dev]["features"]
        if feature not in f:
            raise RuntimeError("Resource {0} doesn't have feature {1}".format(dev, feature))

    def get_fw_header(self):
        return self.fw_header

    def get_sw_header(self):
        return self.sw_header

    def check_mcu_support(self):
        # check MCU support in self.mcu_hw.mcu_name
        return

    def get_dev_ids(self):
        res = list()
        for v in self.dev_configs.values():
            res.append(v["dev_id"])
        return res

    def get_int_value(self, periph: str, d: dict, value: str, values: set) -> int:

        if value not in d:
            raise RuntimeError(
                "Device '{0}' has no '{1}' value defined for '{2}'. Must be defined.".format(
                    self.device_name, value, periph))

        val = d[value]

        if type(val) is not int:
            raise RuntimeError(
                "Device '{0}' has invalid '{1}' value type for '{2}'. Must be integer.".format(
                    self.device_name, value, periph))

        if val not in values:
            raise RuntimeError(
                "Device '{0}' has invalid '{1}' value ({2}) for '{3}'. Must be integer one of {4}: ".format(
                    self.device_name, value, val, periph, str(values)))

        return val