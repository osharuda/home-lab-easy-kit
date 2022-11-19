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

from tools import hash_dict_as_c_array

from ExclusiveDeviceCustomizer import *


class InfoCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, configuration, common_config):
        self.configuration = configuration
        dev_config = {"info": {"dev_id": 0}}
        configuration["devices"]["InfoCustomizer"] = dev_config
        self.i2c_address = common_config["firmware"]["i2c_bus"]["address"]
        super().__init__(mcu_hw, dev_config, common_config, "INFO")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "info_conf.hpp"
        self.sw_lib_source = "info_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])
        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)
        self.devices = dict()

    def customize(self):

        # generate guid from complete configuration
        (h, hash_len) = hash_dict_as_c_array(self.configuration)

        # form array of devices
        self.devices[0] = ("INFO_DEV_TYPE_INFO", "INFO_DEV_HINT_NONE", "info")
        dev_ids = list(self.devices.keys())
        dev_ids.sort()
        info_desciption = list()

        for dev_id in range(0, self.mcu_hw.max_address + 1):
            dt, hint, name = self.devices.get(dev_id, ("INFO_DEV_TYPE_NONE", "INFO_DEV_HINT_NONE", ""))
            info_desciption.append('{{ {0}, {1}, (const char*)"{2}" }}'.format(dt, hint, name))

        vocabulary = {"__NAMESPACE_NAME__": self.project_name.lower(),
                      "__DEVICE_ID__": 0,
                      "__INFO_UUID__": h,
                      "__INFO_UUID_LEN__": hash_len,
                      "__INFO_DEVICES_NUMBER__": self.mcu_hw.max_address+1,
                      "__INFO_DEVICES__": ",\\\n".join(info_desciption),
                      "__INFO_PROJECT_NAME__": self.configuration["firmware"]["device_name"],
                      "__INFO_I2C_ADDRESS__": self.i2c_address}

        self.patch_templates(vocabulary)

    def add_devices(self, configs_dict: dict, dev_type: str):
        result = dict()
        for dev_name, dev_cfg in configs_dict.items():

            hint_map = {"": ("INFO_DEV_HINT_NONE", None),
                        "gsmmodem": ("INFO_DEV_HINT_GSM_MODEM", None)}
            dev_id = dev_cfg["dev_id"]
            hint = dev_cfg.get("hint", "")

            try:
                hint, warning = hint_map[hint]

                if warning is not None:
                    print(f"""
!!! WARNING !!!
!!! WARNING !!! {hint} may be used, however the following should be considered:
!!! WARNING !!! {warning}
!!! WARNING !!!
""")
            except KeyError:
                raise RuntimeError("Device {0} specifies unknown hint value ({1})".format(dev_name, hint))

            self.devices[dev_id] = (dev_type, hint, dev_name)
