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

from BaseCustomizer import *
from keywords import *
from tools import *

class HLEKIOCustomizer(BaseCustomizer):
    def __init__(self, hlekio_config: [dict]):
        super().__init__()
        self.hlekio_dir = os.path.join(get_project_root(), "hlekio")
        self.dts_source = os.path.join(self.hlekio_dir, "hlekio.dts")

        self.dts_template_dir = os.path.join(self.template_dir, "hlekio")
        self.dts_template = os.path.join(self.dts_template_dir, "hlekio.dts")

        self.hlekio_ioctl_header_src = os.path.join(self.libhlek_inc_dest_path, "hlekio_ioctl.h")
        self.hlekio_ioctl_header_dst = os.path.join(self.hlekio_dir, "hlekio_ioctl.h")

        self.config = hlekio_config

        self.init_hardware_info()

        self.add_template(self.dts_template, [self.dts_source])
        self.add_copy(self.hlekio_ioctl_header_src, [self.hlekio_ioctl_header_dst])
        self.device_names = set([str])
        self.pin_numbers = dict[str, str]() # key - pin_number(str), device name


    def init_hardware_info(self):
        self.board_map = {
        "Raspberry Pi Model B": {
                KW_INFO_URL: "https://www.pololu.com/product/2752",
                KW_CPU: "BCM2835",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Model B+": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-1-model-b-plus/",
                KW_CPU: "BCM2835",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 2 Model B": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-2-model-b/",
                KW_CPU: "BCM2836",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 3 Model B": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-3-model-b/",
                KW_CPU: "BCM2837",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 3 Model B+": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-3-model-b-plus/",
                KW_CPU: "BCM2837B0",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 3 Model A+": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-3-model-a-plus/",
                KW_CPU: "BCM2837B0",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Compute Module 3+": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/compute-module-3-plus/",
                KW_CPU: "BCM2837B0",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 4 Model B": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-4-model-b/",
                KW_CPU: "BCM2711",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 400": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-400/",
                KW_CPU: "BCM2711",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi 5": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-5/",
                KW_CPU: "BCM2712",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Zero": {
                KW_INFO_URL: "BCM2835",
                KW_CPU: "https://www.raspberrypi.com/products/raspberry-pi-zero/",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Zero v1.3": {
                KW_INFO_URL: "BCM2835",
                KW_CPU: "https://www.raspberrypi.com/products/raspberry-pi-zero/",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Zero W": {
                KW_INFO_URL: "BCM2835",
                KW_CPU: "https://www.raspberrypi.com/products/raspberry-pi-zero-w/",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Zero WH": {
                KW_INFO_URL: "BCM2835",
                KW_CPU: "https://www.raspberrypi.com/products/raspberry-pi-zero-w/",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "Raspberry Pi Zero 2 W": {
                KW_INFO_URL: "https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/",
                KW_CPU: "BCM2710A1",
                KW_DISTRO: {KW_RASPBERRY_PI_OS}
            },
        "ODROID-C4": {
                KW_INFO_URL: "https://wiki.odroid.com/odroid-c4/hardware/hardware",
                KW_CPU: "S905X3",
                KW_DISTRO: {KW_DIET_PI, KW_UBUNTU_20_04, KW_UBUNTU_22_04}
            },
        "ODROID-HC4": {
                KW_INFO_URL: "https://wiki.odroid.com/odroid-hc4/hardware/hardware",
                KW_CPU: "S905X3",
                KW_DISTRO: {KW_DIET_PI, KW_UBUNTU_20_04, KW_UBUNTU_22_04}
            }
        }

        self.linux_kernels = {
            KW_RASPBERRY_PI_OS: "",
            KW_DIET_PI: "",
            KW_UBUNTU_20_04: "",
            KW_UBUNTU_22_04: ""
        }

        self.cpu_map = {
            "BCM2835": "CPU_BCM2835",
            "BCM2836": "CPU_BCM2835",
            "BCM2837": "CPU_BCM2835",
            "BCM2837B0": "CPU_BCM2835",
            "BCM2710A1": "CPU_BCM2835",
            "RP3A0": "CPU_BCM2835",

            "BCM2711": "CPU_BCM2711",
            "BCM2712": "CPU_BCM2711",

            "S905X3": "CPU_S905X3"
        }

        self.trigger_to_value = {
            KW_RISE: 0,
            KW_FALL: 1,
            KW_EDGE: 2
        }

        self.pull_to_value = {
            KW_PULL_NONE: 0,
            KW_PULL_DOWN: 1,
            KW_PULL_UP: 2
        }

        self.logical_to_value = {
            KW_LOGICAL_HI: 1,
            KW_LOGICAL_LO: 0
        }

        self.pin_type_to_gpio_flags = {
            KW_PUSH_PULL: "GPIO_PUSH_PULL",
            KW_OPEN_DRAIN: "GPIO_LINE_OPEN_DRAIN"
        }

#        self.level_to_gpio_flags = {
#            KW_LOGICAL_HI: "GPIO_ACTIVE_HIGH",
#            KW_LOGICAL_LO: "GPIO_ACTIVE_LOW",
#        }

        self.gpio_pins_raspi = {
            "GPIO_2": 2,
            "GPIO_3": 3,
            "GPIO_4": 4,
            "GPIO_5": 5,
            "GPIO_6": 6,
            "GPIO_7": 7,
            "GPIO_8": 8,
            "GPIO_9": 9,
            "GPIO_10": 10,
            "GPIO_11": 11,
            "GPIO_12": 12,
            "GPIO_13": 13,
            "GPIO_14": 14,
            "GPIO_15": 15,
            "GPIO_16": 16,
            "GPIO_17": 17,
            "GPIO_18": 18,
            "GPIO_19": 19,
            "GPIO_20": 20,
            "GPIO_21": 21,
            "GPIO_22": 22,
            "GPIO_23": 23,
            "GPIO_24": 24,
            "GPIO_25": 25,
            "GPIO_26": 26,
            "GPIO_27": 27
        }

        self.gpio_pins = self.gpio_pins_raspi

        self.pull_to_input_gpio_state = {
            KW_PULL_NONE: "GPIO_PULL_DISABLE",
            KW_PULL_DOWN: "GPIO_PULL_DOWN",
            KW_PULL_UP: "GPIO_PULL_UP"
        }

        self.distro_map = {"raspi": {"https://github.com/raspberrypi/linux"}}

    def register_pin_and_device(self, name: str, pin: str):
        if name in self.device_names:
            raise RuntimeError(f'More than one declaration of device "{name}".')
        else:
            self.device_names.add(name)

        if pin in self.pin_numbers:
            raise RuntimeError(f'Pin {pin} ( "{name}" ) is already used in device {self.pin_numbers[pin]}.')
        else:
            self.pin_numbers[pin] = name

    def generate_input_pin_config_node(self, name: str, data: dict) -> str:
        pin = get_param(data, KW_PIN, f' for input {KW_HLEKIO} device "{name}"', self.gpio_pins)
        pull = get_param(data, KW_PULL, f' for input {KW_HLEKIO} device "{name}"', self.pull_to_value)

        result = f"""
        		{name}_pin: {name}_pin {{
				brcm,pins=<{pin}>;
				brcm,function=<0>;
				brcm,pull=<{pull}>;
			}};"""


        self.register_pin_and_device(name, pin)
        
        return result

    """
    edge values consist of the following flags:
          1 = low-to-high edge triggered.
          2 = high-to-low edge triggered.
          4 = active high level-sensitive.
          8 = active low level-sensitive.
        Valid combinations are 1, 2, 3, 4, 8.
    """

    def generate_input_device(self, name: str, data: dict) -> str:
        pin = get_param(data, KW_PIN, f' for input {KW_HLEKIO} device "{name}"', self.gpio_pins)
        trigger = get_param(data, KW_TRIGGER, f' for input {KW_HLEKIO} device "{name}"', self.trigger_to_value)
        pull = get_param(data, KW_PULL, f' for input {KW_HLEKIO} device "{name}"', self.pull_to_input_gpio_state)

        return f"""
                    {name} {{
                        compatible="hlek,io";
                        label="{name}";
                        default=<0>;
                        pinctrl-names = "default";
                        pinctrl-0=<&{name}_pin>;
        				{name}-gpio=<&gpio {pin} {pull}>;
        				trigger=<{trigger}>;
                    }};"""

    def generate_output_pin_config_node(self, name: str, data: dict) -> str:
        pin = get_param(data, KW_PIN, f' for output {KW_HLEKIO} device "{name}"', self.gpio_pins)
        self.register_pin_and_device(name, pin)

        return f"""
            {name}_pin: {name}_pin {{
                brcm,pins=<{pin}>;
                brcm,function=<1>;
                brcm,pull=<0>;
            }};"""



    def generate_output_device(self, name: str, data: dict) -> str:
        pin = get_param(data, KW_PIN, f' for output {KW_HLEKIO} device "{name}"', self.gpio_pins)
        default_level = get_param(data, KW_DEFAULT, f' for output {KW_HLEKIO} device "{name}"', self.logical_to_value)
        out_type = get_param(data, KW_PIN_TYPE, f' for output {KW_HLEKIO} device "{name}"', self.pin_type_to_gpio_flags)

        return f"""
            {name} {{
                compatible="hlek,io";
                label="{name}";
                init_state=<{default_level}>;
                default=<0>;
                pinctrl-0=<&{name}_pin>;
                {name}-gpio=<&gpio {pin} ({out_type}|GPIO_ACTIVE_HIGH)>;
            }};"""

    def customize(self):

        if KW_INPUTS not in self.config:
            raise RuntimeError(f'"{KW_INPUTS}" keyword is not specified in configuration. If inputs are not required, specify it to be empty.')

        if KW_OUTPUTS not in self.config:
            raise RuntimeError(f'"{KW_OUTPUTS}" keyword is not specified in configuration. If inputs are not required, specify it to be empty.')

        inputs = self.config[KW_INPUTS]
        outputs = self.config[KW_OUTPUTS]

        pin_groups = ""
        devices = ""
        for in_name, in_data in inputs.items():
            pin_groups += self.generate_input_pin_config_node(in_name, in_data)
            devices += self.generate_input_device(in_name, in_data)

        for out_name, out_data in outputs.items():
            pin_groups += self.generate_output_pin_config_node(out_name, out_data)
            devices += self.generate_output_device(out_name, out_data)

        self.vocabulary = self.vocabulary | {
            "__PIN_GROUPS__": pin_groups,
            "__HLEK_IO_DEVICES__": devices
        }

        self.patch_templates()
