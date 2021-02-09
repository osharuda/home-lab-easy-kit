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


class RTCCustomizer(ExclusiveDeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs):
        super().__init__(mcu_hw, dev_configs, "RTC")
        self.fw_header = "fw_rtc.h"
        self.sw_header = "sw_rtc.h"
        self.shared_header = "rtc_proto.h"

        self.add_template(self.fw_inc_templ + self.fw_header, [self.fw_inc_dest + self.fw_header])
        self.add_template(self.sw_inc_templ + self.sw_header, [self.sw_inc_dest + self.sw_header])
        self.add_shared_code(self.shared_templ + self.shared_header, "__RTC_SHARED_HEADER__")

    def customize(self):
        rtc_requires = self.dev_config["requires"]
        rtc = self.get_rtc(rtc_requires)
        br = self.get_backup_reg(rtc_requires)
        vocabulary = {"__DEVICE_ID__": self.dev_config["dev_id"],
                      "__RTC_DEVICE_NAME__": self.device_name,
                      "__RTC_BACKUP_REG__": br}
        self.patch_templates(vocabulary)

