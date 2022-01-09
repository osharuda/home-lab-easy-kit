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

from BaseDeviceCustomizer import *


class DeviceCustomizer(BaseDeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, device_name):
        super().__init__(mcu_hw, dev_configs)
        self.device_count = len(dev_configs)
        self.device_list = list(self.dev_configs.items())
        self.device_name = device_name
        return


