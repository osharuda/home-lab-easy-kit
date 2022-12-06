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
from tools import *
from shutil import copy2

class BaseDeviceCustomizer:
    def __init__(self, mcu_hw, dev_configs, common_config):
        self.mcu_hw = mcu_hw
        self.dev_configs = dev_configs
        self.device_name = "BASE_DEVICE"
        self.config_name = common_config['firmware']['device_name'] #<!CHECKIT!> device name should become config name

        self.fw_templ = "../templates/firmware/"
        self.fw_inc_templ = "../templates/firmware/inc/"
        self.fw_src_templ = "../templates/firmware/src/"
        self.fw_path = "../../firmware/"
        self.fw_inc_source_path = "../../firmware/inc/"
        self.fw_src_source_path = "../../firmware/src/"
        self.fw_inc_dest = common_config['firmware']['firmware_inc_path']
        self.fw_dest = common_config['firmware']['firmware_path']
        self.shared_templ = "../templates/shared/"

        self.sw_templ = "../templates/monitor/"
        self.sw_example_templ = "../templates/example/"
        self.sw_example_inc_templ = "../templates/example/inc/"
        self.sw_example_src_templ = "../templates/example/src/"
        self.sw_monitor_dest = common_config["firmware"]["monitor_path"] + os.path.sep

        self.sw_example_dest = common_config["firmware"]["example_path"] + os.path.sep
        self.sw_example_inc_dest = common_config["firmware"]["example_inc_path"] + os.path.sep
        self.sw_example_src_dest = common_config["firmware"]["example_src_path"] + os.path.sep

        self.sw_inc_templ = "../templates/libhlek/inc/"
        self.sw_src_templ = "../templates/software/src/"


        self.sw_inc_dest = "../../software/inc/"
        self.sw_src_dest = "../../software/src/"
        self.sw_testtool_templ = "../templates/software/testtool/"
        self.sw_testtool_dest = "../../software/testtool/"

        self.sw_lib_path = common_config['firmware']['libconfig_path']
        self.sw_lib_inc_dest = common_config['firmware']['libconfig_inc_path'] + os.path.sep
        self.sw_lib_src_dest = common_config['firmware']['libconfig_src_path'] + os.path.sep

        self.sw_lib_templ_path = os.path.abspath("../templates/libconfig/") + os.path.sep
        self.sw_lib_inc_templ_path = os.path.join(self.sw_lib_templ_path, "inc/") + os.path.sep
        self.sw_lib_src_templ_path = os.path.join(self.sw_lib_templ_path, "src/") + os.path.sep
        self.flash_script = "flash.sh"
        self.fw_base_header = "fw.h"
        self.fw_header = self.fw_base_header

        self.libhlek_templ_path = os.path.abspath("../templates/libhlek/") + os.path.sep
        self.libhlek_dest_path = os.path.abspath("../../libhlek") + os.path.sep
        self.libhlek_inc_dest_path = os.path.join(self.libhlek_dest_path, "inc") + os.path.sep
        self.libhlek_src_dest_path = os.path.join(self.libhlek_dest_path, "src") + os.path.sep

        self.template_list = dict()
        self.file_copy_list = dict()
        self.shared_code = dict()
        self.tab = "    "
        self.newline = "\n"
        self.project_name = common_config['firmware']['device_name']

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

    def add_copy(self, in_file: str, out_file_list: list):
        self.file_copy_list[in_file] = out_file_list

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

        # Process file copy
        for in_file, out_file_list in self.file_copy_list.items():
            for fn in out_file_list:
                copy2(in_file, fn)



    def get_gpio(self, d: dict) -> str:
        if (d is None) or ("gpio" not in d):
            raise RuntimeError("gpio resource is not specified")
        gpio = d["gpio"]
        self.check_resource(gpio, "gpio")
        return gpio

    def get_gpio_with_default(self, d: dict, default=None) -> str:
        if (d is None) or ("gpio" not in d):
            return default
        return self.get_gpio(d)

    def get_i2c(self, d: dict) -> str:
        i2c = d["i2c"]
        self.check_resource(i2c, "i2c")
        return i2c

    def get_isr(self, d: dict) -> str:
        isr = d["irq_handler"]
        self.check_resource(isr, "irq_handler")
        return isr

    def get_rtc(self, d: dict) -> str:
        rtc = d["rtc"]
        self.check_resource(rtc, "rtc")
        return rtc

    def get_spi(self, d: dict) -> str:
        spi = d["spi"]
        self.check_resource(spi, "spi")
        return spi

    def get_backup_reg(self, d: dict) -> str:
        br = d["bkp"]
        self.check_resource(br, "bkp")
        return br

    def get_gpio_pin_type(self, t: str):
        if t not in self.mcu_hw.mcu_gpio_pin_types:
            raise RuntimeError("Device {0} uses invalid pin type description ({1}) for {2}".format(self.device_name, t,
                                                                                                   self.mcu_hw.mcu_name))
        return t

    def get_usart(self, d: dict) -> str:
        usart = d["usart"]
        self.check_resource(usart, "usart")
        return usart

    def get_timer(self, d: dict, subtype=None) -> str:
        timer = d["timer"]
        self.check_resource(timer, "timer")
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

        self.check_resource(res, rtype)

        return rtype, res

    def get_resource_by_name(self, name: str) -> tuple:
        if name not in self.mcu_hw.mcu_resources:
            raise RuntimeError("Unknown resource {0}".format(name))

        return (self.mcu_hw.mcu_resources[name]["type"], name)

    def get_required_resource(self, res: str, dep_res: str, dep_res_type: str) -> tuple:
        if res not in self.mcu_hw.mcu_resources:
            raise RuntimeError("Unknown resource {0}".format(res))

        req = self.mcu_hw.mcu_resources[res]['requires']
        if dep_res not in req:
            raise RuntimeError("Not a dependent resource {0}".format(dep_res))

        dr = req[dep_res]
        if len(dr)!=1:
            raise RuntimeError("Invalid dependent resource specification {0}. Must be single resource in dict.".format(dr))

        if dep_res_type not in dr.keys():
            raise RuntimeError("Wrong dependent resource type {0}".format(dep_res_type))

        return dr[dep_res_type]

    def get_resources_by_type(self, rtype: str) -> dict:
        result = dict()
        for rname, rval in self.mcu_hw.mcu_resources.items():
            if rval["type"]==rtype:
                result[rname]=rval
        return result


    def check_resource(self, res_name: str, res_type: str = ""):
        if res_name not in self.mcu_hw.mcu_resources.keys():
            raise RuntimeError("Device {0} uses invalid resource ({1}) for {2}".format(self.device_name, res_name,
                                                                                       self.mcu_hw.mcu_name))

        if res_type and self.mcu_hw.mcu_resources[res_name]["type"] != res_type:
            raise RuntimeError("Device {0} uses resource with wrong type({1} : {2}) for {3}".format(self.device_name,
                                                                                                    res_name,
                                                                                                    res_type,
                                                                                                    self.mcu_hw.mcu_name))
        return self.mcu_hw.mcu_resources[res_name]

    def check_res_feature(self, dev : str, feature : str):
        f = self.mcu_hw.mcu_resources[dev]["features"]
        if feature not in f:
            raise RuntimeError("Resource {0} doesn't have feature {1}".format(dev, feature))

    def get_fw_header(self):
        return self.fw_header

    def check_mcu_support(self):
        # check MCU support in self.mcu_hw.mcu_name
        return

    def check_requirements(self, obj: str, dev_requires: dict, dev_suffix: str, exclude: set = {}):
        required = self.check_resource(obj)['requires']

        if not dev_suffix:
            raise RuntimeError("dev_suffix may not be empty")

        self.check_requirements_dict(required, dev_requires, dev_suffix, exclude)
        return

    def check_requirements_dict(self, required: dict, dev_requires: dict, dev_suffix: str, exclude: set):
        for k, v in required.items():
            if isinstance(k, str) and isinstance(v, str):
                # both k is type, v is resource
                dev_requires[k+dev_suffix] = v
                self.check_resource(v)
                self.check_requirements(v, dev_requires, dev_suffix, exclude)
            elif isinstance(k, str) and isinstance(v, dict):
                if k not in exclude:
                    self.check_requirements_dict(v, dev_requires, "{0}_{1}".format(k, dev_suffix), exclude)
            else:
                raise RuntimeError("Bad requirement")
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

    def require_feature(self, feature: str, dev_config: dict):
        if feature not in self.mcu_hw.FW_FEATURES:
            raise RuntimeError(f"Unknown feature '{feature}' is required by device {self.device_name}")

        feature_dict = dev_config.setdefault("required_features", set())
        feature_dict.add(feature)
