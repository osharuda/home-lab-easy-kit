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

import json
import sys
import os
import importlib.util
from tools import *

from InfoCustomizer import *
from DeskDevCustomizer import *
from IRRCCustomizer import *
from RTCCustomizer import *
from UartProxyCustomizer import *
from LCD1602aCustomizer import *
from GPIODevCustomizer import *
from FirmwareCustomizer import *
from SPWMCustomizer import *
from ADCDevCustomizer import *
from EXTIHubCustomizer import *
from StepMotorDevCustomizer import *

# Load configuration file specified as parameter
with open(os.path.abspath(sys.argv[1])) as f:
    configuration = json.load(f)

# MCU model
mcu_module = "{0}.py".format(configuration["firmware"]["mcu_model"])
spec = importlib.util.spec_from_file_location(mcu_module, mcu_module)
mcu = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mcu)

required_resources = list()
fw_customizer = FirmwareCustomizer(mcu, configuration["firmware"], required_resources)
fw_customizer.clean()

# InfoCustomizer - device containing all information about build
info_customizer = InfoCustomizer(mcu, configuration)

# EXTIHub is a part of firmware detected to use as much EXTI lines as possible
extihub_customizer = EXTIHubCustomizer(mcu)

for customizer_name in configuration["devices"].keys():
    config_dict = configuration["devices"][customizer_name]

    if customizer_name == "DeskDevCustomizer":
        dt = "INFO_DEV_TYPE_DESKDEV"
        customizer = DeskDevCustomizer(mcu, config_dict)
    elif customizer_name == "IRRCCustomizer":
        dt = "INFO_DEV_TYPE_IRRC"
        customizer = IRRCCustomizer(mcu, config_dict)
    elif customizer_name == "RTCCustomizer":
        dt = "INFO_DEV_TYPE_RTC"
        customizer = RTCCustomizer(mcu, config_dict)
    elif customizer_name == "UartProxyCustomizer":
        dt = "INFO_DEV_TYPE_UART_PROXY"
        customizer = UartProxyCustomizer(mcu, config_dict)
    elif customizer_name == "LCD1602aCustomizer":
        dt = "INFO_DEV_TYPE_LCD1602a"
        customizer = LCD1602aCustomizer(mcu, config_dict)
    elif customizer_name == "GPIODevCustomizer":
        dt = "INFO_DEV_TYPE_GPIO"
        customizer = GPIODevCustomizer(mcu, config_dict)
    elif customizer_name == "SPWMCustomizer":
        dt = "INFO_DEV_TYPE_SPWM"
        customizer = SPWMCustomizer(mcu, config_dict)
    elif customizer_name == "ADCDevCustomizer":
        dt = "INFO_DEV_TYPE_ADC"
        customizer = ADCDevCustomizer(mcu, config_dict)
    elif customizer_name == "StepMotorDevCustomizer":
        dt = "INFO_DEV_TYPE_STEP_MOTOR"
        customizer = StepMotorDevCustomizer(mcu, config_dict)
    elif customizer_name == "InfoCustomizer":
        continue
    else:
        raise RuntimeError("{0} is not implemented/supported now.".format(customizer_name))

    customizer.customize()

    info_customizer.add_devices(config_dict, dt)

    fw_customizer.add_fw_header(customizer.fw_header)
    fw_customizer.add_sw_header(customizer.sw_header)
    fw_customizer.add_allocated_dev_ids(customizer.get_dev_ids())

    for dev_name, dev_config in config_dict.items():
        required_resources.extend(get_leaf_values(dev_config["requires"]))
        extihub_customizer.register_exti(dev_name, dev_config["requires"])

# Customize EXTI hub
if extihub_customizer.enabled():
    dev_requires = extihub_customizer.customize(configuration)
    required_resources.extend(get_leaf_values(dev_requires))
    fw_customizer.add_fw_header(extihub_customizer.fw_header)

# Customize Info device
info_customizer.customize()
fw_customizer.add_fw_header(info_customizer.fw_header)
fw_customizer.add_sw_header(info_customizer.sw_header)
fw_customizer.add_allocated_dev_ids(info_customizer.get_dev_ids())

fw_customizer.customize()
fw_customizer.detect_conflicting_resources()
fw_customizer.check_dev_ids()
fw_customizer.detect_conflicting_dev_ids()

print(json.dumps(configuration, indent=4))