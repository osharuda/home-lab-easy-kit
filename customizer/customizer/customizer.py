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
import shutil

from LibhlekCustomizer import *
from HLEKIOCustomizer import *
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
from CanCustomizer import *
from SPIProxyCustomizer import *
from AD9850DevCustomizer import *
from SPIDACCustomizer import *
# -> IMPORT_CUSTOMIZER | HASH: C20ADA6B43BC43E19CC5E0A90FCC37A7F0068B21
from PaceMakerDevCustomizer import *
# -> IMPORT_CUSTOMIZER | HASH: C20ADA6B43BC43E19CC5E0A90FCC37A7F0068B21
# -> IMPORT_CUSTOMIZER | HASH: C20ADA6B43BC43E19CC5E0A90FCC37A7F0068B21
from TimeTrackerDevCustomizer import *
# -> IMPORT_CUSTOMIZER | HASH: C20ADA6B43BC43E19CC5E0A90FCC37A7F0068B21
# IMPORT_CUSTOMIZER

# Load configuration file specified as parameter
# fn: name of the json file
def load_json(fn: str) -> dict:
    with open(os.path.abspath(fn)) as f:
        try:
            result = json.load(f)
        except Exception as je:
            raise Exception('JSON syntax error:', je)

        return result, hash_dict_as_bytes(result).hex()


def is_outdated(key: str, digest: str, check_dir: str = None):
    if opt_ignore_cache:
        if opt_verbose:
            print("Ignoring hashes, customization will be performed.")
        return True

    if check_dir and not os.path.exists(check_dir):
        if opt_verbose:
            print("Check directory is specified and it is not found, customization will be performed.")
        return True

    if GLOBAL_HASH_KEY not in hashes or\
            global_hash != hashes[GLOBAL_HASH_KEY]:
        if opt_verbose:
            print(f"Customization will be performed because {GLOBAL_HASH_KEY} hash wasn't calculated yet or mismatch.")
        return True

    if key not in hashes or\
            digest != hashes[key]:
        if opt_verbose:
            print(f"Customization will be performed because {key} hash wasn't calculated yet or mismatch.")
        return True

    return False


def update_caches(key: str, digest):
    hashes[key] = digest
    hashes_text = json.dumps(hashes, indent=4, cls=Sorted_JSON_Encoder)
    write_text_file(HASHES_JSON, hashes_text)

def get_feature_define_dict():
    feature_detect_dict = dict()
    feature_detect_dict[AD9850DevCustomizer.__name__] = "AD9850DEV_DEVICE_ENABLED"
    feature_detect_dict[ADCDevCustomizer.__name__] = "ADCDEV_DEVICE_ENABLED"
    feature_detect_dict[CanCustomizer.__name__] = "CAN_DEVICE_ENABLED"
    feature_detect_dict[DeskDevCustomizer.__name__] = "DESKDEV_DEVICE_ENABLED"
    feature_detect_dict[GPIODevCustomizer.__name__] = "GPIODEV_DEVICE_ENABLED"
    feature_detect_dict[InfoCustomizer.__name__] = "INFO_DEVICE_ENABLED"
    feature_detect_dict[IRRCCustomizer.__name__] = "IRRC_DEVICE_ENABLED"
    feature_detect_dict[LCD1602aCustomizer.__name__] = "LCD1602a_DEVICE_ENABLED"
    feature_detect_dict[RTCCustomizer.__name__] = "RTC_DEVICE_ENABLED"
    feature_detect_dict[SPIDACCustomizer.__name__] = "SPIDAC_DEVICE_ENABLED"
    feature_detect_dict[SPIProxyCustomizer.__name__] = "SPIPROXY_DEVICE_ENABLED"
    feature_detect_dict[SPWMCustomizer.__name__] = "SPWM_DEVICE_ENABLED"
    feature_detect_dict[StepMotorDevCustomizer.__name__] = "STEP_MOTOR_DEVICE_ENABLED"
    feature_detect_dict[UartProxyCustomizer.__name__] = "UART_PROXY_DEVICE_ENABLED"
    feature_detect_dict[PaceMakerDevCustomizer.__name__] = "PACEMAKERDEV_DEVICE_ENABLED"
    feature_detect_dict[TimeTrackerDevCustomizer.__name__] = "TIMETRACKERDEV_DEVICE_ENABLED"
    feature_detect_dict[EXTIHubCustomizer.__name__] = "EXTIHUB_DEVICE_ENABLED"

    # ADD_FEATURE_DEFINE

    return feature_detect_dict



def get_shared_headers_dict():
    shared_headers = dict()

    # Customizer class name |libhlek common header  |    shared header    |  firmware header     | config header        | shared token
    shared_headers[AD9850DevCustomizer.__name__] = (
    "ad9850_common.hpp", "ad9850_shared.h", "ad9850_conf.h", "ad9850_conf.hpp", "__AD9850_SHARED_HEADER__")
    shared_headers[ADCDevCustomizer.__name__] = (
    "adc_common.hpp", "adc_shared.h", "adc_conf.h", "adc_conf.hpp", "__ADC_SHARED_HEADER__")
    shared_headers[CanCustomizer.__name__] = (
    "can_common.hpp", "can_shared.h", "can_conf.h", "can_conf.hpp", "__CAN_SHARED_HEADER__")
    shared_headers[DeskDevCustomizer.__name__] = (
    "desk_common.hpp", "desk_shared.h", "desk_conf.h", "desk_conf.hpp", "__DESKDEV_SHARED_HEADER__")
    shared_headers[GPIODevCustomizer.__name__] = (
    "gpio_common.hpp", "gpio_shared.h", "gpio_conf.h", "gpio_conf.hpp", "__GPIO_SHARED_HEADER__")
    shared_headers[InfoCustomizer.__name__] = (
    "info_common.hpp", "info_shared.h", "info_conf.h", "info_conf.hpp", "__INFO_SHARED_HEADER__")
    shared_headers[IRRCCustomizer.__name__] = (
    "irrc_common.hpp", "irrc_shared.h", "irrc_conf.h", "irrc_conf.hpp", "__CONTROLS_SHARED_HEADER__")
    shared_headers[LCD1602aCustomizer.__name__] = (
    "lcd1602a_common.hpp", "lcd1602a_shared.h", "lcd1602a_conf.h", "lcd1602a_conf.hpp", "__LCD1602A_SHARED_HEADER__")
    shared_headers[RTCCustomizer.__name__] = (
    "rtc_common.hpp", "rtc_shared.h", "rtc_conf.h", "rtc_conf.hpp", "__RTC_SHARED_HEADER__")
    shared_headers[SPIDACCustomizer.__name__] = (
    "spidac_common.hpp", "spidac_shared.h", "spidac_conf.h", "spidac_conf.hpp", "__SPIDAC_SHARED_HEADER__")
    shared_headers[SPIProxyCustomizer.__name__] = (
    "spiproxy_common.hpp", "spiproxy_shared.h", "spiproxy_conf.h", "spiproxy_conf.hpp", "__SPIPROXY_SHARED_HEADER__")
    shared_headers[SPWMCustomizer.__name__] = (
    "spwm_common.hpp", "spwm_shared.h", "spwm_conf.h", "spwm_conf.hpp", "__SPWM_SHARED_HEADER__")
    shared_headers[StepMotorDevCustomizer.__name__] = (
    "step_motor_common.hpp", "step_motor_shared.h", "step_motor_conf.h", "step_motor_conf.hpp",
    "__STEP_MOTOR_SHARED_HEADER__")
    shared_headers[UartProxyCustomizer.__name__] = (
    "uart_proxy_common.hpp", "uart_proxy_shared.h", "uart_proxy_conf.h", "uartproxy_conf.hpp",
    "__UART_PROTO_SHARED_HEADER__")
# -> ADD_SHARED_HEADER_RECORD | HASH: 251B2D30A0B3397A383906AC4ECADE7A75749EB9
    shared_headers[PaceMakerDevCustomizer.__name__] = ("pacemakerdev_common.hpp", "pacemakerdev_shared.h", "pacemakerdev_conf.h", "pacemakerdev_conf.hpp", "__PACEMAKERDEV_SHARED_HEADER__")
# -> ADD_SHARED_HEADER_RECORD | HASH: 251B2D30A0B3397A383906AC4ECADE7A75749EB9
# -> ADD_SHARED_HEADER_RECORD | HASH: 251B2D30A0B3397A383906AC4ECADE7A75749EB9
    shared_headers[TimeTrackerDevCustomizer.__name__] = ("timetrackerdev_common.hpp", "timetrackerdev_shared.h", "timetrackerdev_conf.h", "timetrackerdev_conf.hpp", "__TIMETRACKERDEV_SHARED_HEADER__")
# -> ADD_SHARED_HEADER_RECORD | HASH: 251B2D30A0B3397A383906AC4ECADE7A75749EB9
    # ADD_SHARED_HEADER_RECORD

    return shared_headers


def customize_firmware(json_file_name: str):
    json_base_name = os.path.basename(json_file_name)
    hash_key = f'{json_base_name}_hash'
    customizer_path = os.path.join(project_dir, "customizer/customizer")
    configuration, configuration_hash = load_json(json_file_name)


    configuration["global"] = global_config

    # Check configuration
    configuration[FW_FIRMWARE][KW_DEV_NAME], json_ext = os.path.splitext(os.path.basename(json_file_name))
    if 'firmware' not in configuration:
        raise RuntimeError(f'"{FW_FIRMWARE}" section is not specified.')
    firmware_conf_section = configuration['firmware']

    if 'device_name' not in firmware_conf_section:
        raise RuntimeError(f'device_name is not specified in "{FW_FIRMWARE}" section')
    devname = firmware_conf_section['device_name']

    if 'mcu_model' not in firmware_conf_section:
        raise RuntimeError(f'mcu_model is not specified in "{FW_FIRMWARE}" section')

    # Generate file and directories paths
    libconfig_name = f'lib{devname}'
    dev_target_path = os.path.join(get_project_root(), devname)
    libconfig_path = os.path.join(dev_target_path, libconfig_name)
    libconfig_inc_path = os.path.join(libconfig_path, 'inc')
    libconfig_src_path = os.path.join(libconfig_path, 'src')
    monitor_path = os.path.join(dev_target_path, 'monitor')
    example_path = os.path.join(dev_target_path, 'example')
    example_inc_path = os.path.join(example_path, 'inc')
    example_src_path = os.path.join(example_path, 'src')
    firmware_path = os.path.join(dev_target_path, 'firmware')
    firmware_inc_path = os.path.join(firmware_path, 'inc')
    configuration[FW_FIRMWARE]["libname"] = libconfig_name
    configuration[FW_FIRMWARE]["libconfig_path"] = libconfig_path
    configuration[FW_FIRMWARE]["libconfig_inc_path"] = libconfig_inc_path
    configuration[FW_FIRMWARE]["libconfig_src_path"] = libconfig_src_path
    configuration[FW_FIRMWARE]["monitor_path"] = monitor_path
    configuration[FW_FIRMWARE]["example_path"] = example_path
    configuration[FW_FIRMWARE]["example_inc_path"] = example_inc_path
    configuration[FW_FIRMWARE]["example_src_path"] = example_src_path
    configuration[FW_FIRMWARE]["firmware_path"] = firmware_path
    configuration[FW_FIRMWARE]["firmware_inc_path"] = firmware_inc_path

    configuration["generation"] = dict()
    configuration["generation"]["shared"] = get_shared_headers_dict()
    configuration["generation"][KW_FEATURE_DEFINES] = get_feature_define_dict()

    # Check if customization is actually required
    if not is_outdated(hash_key, configuration_hash):
        print(f'No changes for {json_file_name} detected, exiting ...')
        return

    # Create directories
    if os.path.exists(dev_target_path):
        shutil.rmtree(dev_target_path)
    os.mkdir(dev_target_path)
    os.mkdir(libconfig_path)
    os.mkdir(libconfig_inc_path)
    os.mkdir(libconfig_src_path)
    os.mkdir(monitor_path)
    os.mkdir(example_path)
    os.mkdir(example_inc_path)
    os.mkdir(example_src_path)
    os.mkdir(firmware_path)
    os.mkdir(firmware_inc_path)

    # MCU model
    mcu_module = os.path.join(customizer_path, f"{configuration['firmware']['mcu_model']}.py")
    spec = importlib.util.spec_from_file_location(mcu_module, mcu_module)
    mcu = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mcu)

    required_resources = list()
    required_features = set()
    fw_customizer = FirmwareCustomizer(mcu, configuration[FW_FIRMWARE], configuration, required_resources,
                                       required_features)

    # Add headers to libhlek (all must be added)
    fw_customizer.add_shared_header(configuration, AD9850DevCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, ADCDevCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, CanCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, DeskDevCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, GPIODevCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, InfoCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, IRRCCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, LCD1602aCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, RTCCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, SPIDACCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, SPIProxyCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, SPWMCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, StepMotorDevCustomizer.__name__)
    fw_customizer.add_shared_header(configuration, UartProxyCustomizer.__name__)
# -> ADD_LIBHLEK_COMMON_HEADER | HASH: EE88CF0A25ABEECCB614BD607A19CCF91781A85C
    fw_customizer.add_shared_header(configuration, PaceMakerDevCustomizer.__name__)
# -> ADD_LIBHLEK_COMMON_HEADER | HASH: EE88CF0A25ABEECCB614BD607A19CCF91781A85C
# -> ADD_LIBHLEK_COMMON_HEADER | HASH: EE88CF0A25ABEECCB614BD607A19CCF91781A85C
    fw_customizer.add_shared_header(configuration, TimeTrackerDevCustomizer.__name__)
# -> ADD_LIBHLEK_COMMON_HEADER | HASH: EE88CF0A25ABEECCB614BD607A19CCF91781A85C
    # ADD_LIBHLEK_COMMON_HEADER

    # InfoCustomizer - device containing all information about build
    info_customizer = InfoCustomizer(mcu, configuration, configuration)

    # EXTIHub is a part of firmware detected to use as much EXTI lines as possible
    extihub_customizer = EXTIHubCustomizer(mcu, configuration)

    for customizer_name in configuration["devices"].keys():
        config_dict = configuration["devices"][customizer_name]

        if customizer_name == "DeskDevCustomizer":
            dt = "INFO_DEV_TYPE_DESKDEV"
            customizer = DeskDevCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "IRRCCustomizer":
            dt = "INFO_DEV_TYPE_IRRC"
            customizer = IRRCCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "RTCCustomizer":
            dt = "INFO_DEV_TYPE_RTC"
            customizer = RTCCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "UartProxyCustomizer":
            dt = "INFO_DEV_TYPE_UART_PROXY"
            customizer = UartProxyCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "LCD1602aCustomizer":
            dt = "INFO_DEV_TYPE_LCD1602a"
            customizer = LCD1602aCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "GPIODevCustomizer":
            dt = "INFO_DEV_TYPE_GPIO"
            customizer = GPIODevCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "SPWMCustomizer":
            dt = "INFO_DEV_TYPE_SPWM"
            customizer = SPWMCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "ADCDevCustomizer":
            dt = "INFO_DEV_TYPE_ADC"
            customizer = ADCDevCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "StepMotorDevCustomizer":
            dt = "INFO_DEV_TYPE_STEP_MOTOR"
            customizer = StepMotorDevCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "CanCustomizer":
            dt = "INFO_DEV_TYPE_CAN"
            customizer = CanCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "SPIProxyCustomizer":
            dt = "INFO_DEV_TYPE_SPIPROXY"
            customizer = SPIProxyCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "AD9850DevCustomizer":
            dt = "INFO_DEV_TYPE_AD9850DEV"
            customizer = AD9850DevCustomizer(mcu, config_dict, configuration)
        elif customizer_name == "SPIDACCustomizer":
            dt = "INFO_DEV_TYPE_SPIDAC"
            customizer = SPIDACCustomizer(mcu, config_dict, configuration)
# -> REGISTER_CUSTOMIZER | HASH: D85113CE1E662E848F94E1C716B0F918DCD14FEC
        elif customizer_name == "PaceMakerDevCustomizer":
            dt = "INFO_DEV_TYPE_PACEMAKERDEV"
            customizer = PaceMakerDevCustomizer(mcu, config_dict, configuration)
# -> REGISTER_CUSTOMIZER | HASH: D85113CE1E662E848F94E1C716B0F918DCD14FEC
# -> REGISTER_CUSTOMIZER | HASH: D85113CE1E662E848F94E1C716B0F918DCD14FEC
        elif customizer_name == "TimeTrackerDevCustomizer":
            dt = "INFO_DEV_TYPE_TIMETRACKERDEV"
            customizer = TimeTrackerDevCustomizer(mcu, config_dict, configuration)
# -> REGISTER_CUSTOMIZER | HASH: D85113CE1E662E848F94E1C716B0F918DCD14FEC
        # REGISTER_CUSTOMIZER
        elif customizer_name == "InfoCustomizer":
            continue
        else:
            raise RuntimeError("{0} is not implemented/supported now.".format(customizer_name))

        customizer.customize()

        info_customizer.add_devices(config_dict, dt)

        feature_define = configuration["generation"][KW_FEATURE_DEFINES][customizer_name]
        fw_customizer.add_fw_feature_define(feature_define)
        #fw_customizer.add_fw_header(customizer.fw_header)
        fw_customizer.add_sw_header(customizer.sw_header)
        fw_customizer.add_allocated_dev_ids(customizer.get_dev_ids())

        for dev_name, dev_config in config_dict.items():
            required_resources.extend(get_leaf_values(dev_config[KW_REQUIRES]))
            required_features.update(dev_config.get("required_features", set()))
            extihub_customizer.register_exti(dev_name, dev_config[KW_REQUIRES])

    # Customize EXTI hub
    if extihub_customizer.enabled():
        dev_requires = extihub_customizer.customize(configuration)
        required_resources.extend(get_leaf_values(dev_requires))
        required_features.update(configuration["devices"]["EXTIHubCustomizer"].get("required_features", set()))
        #fw_customizer.add_fw_header(extihub_customizer.fw_header)
        feature_define = configuration["generation"][KW_FEATURE_DEFINES][EXTIHubCustomizer.__name__]
        fw_customizer.add_fw_feature_define(feature_define)

    # Customize Info device
    info_customizer.customize()
    fw_customizer.add_sw_header(info_customizer.sw_header)
    #fw_customizer.add_fw_header(info_customizer.fw_header)
    feature_define = configuration["generation"][KW_FEATURE_DEFINES][InfoCustomizer.__name__]
    fw_customizer.add_fw_feature_define(feature_define)
    fw_customizer.add_allocated_dev_ids(info_customizer.get_dev_ids())

    fw_customizer.customize()
    fw_customizer.detect_conflicting_resources()
    fw_customizer.check_dev_ids()
    fw_customizer.detect_conflicting_dev_ids()

    config_text = json.dumps(configuration, indent=4, cls=Sorted_JSON_Encoder)
    mcu.check_errata(config_text)

    update_caches(hash_key, configuration_hash)

    if opt_verbose:
        print(config_text)
        print(required_resources)

def customize_libhlek():
    LIBHLEK_HASH_KEY = "libhlek"
    configuration = dict()
    configuration["global"] = global_config
    configuration["generation"] = dict()
    configuration["generation"]["shared"] = get_shared_headers_dict()

    customizer = LibhlekCustomizer(configuration)
    cmakelists_path = customizer.get_cmakelists_path()
    libhlek_hash = hash_text_file(cmakelists_path)
    if is_outdated(LIBHLEK_HASH_KEY, libhlek_hash, cmakelists_path):
        customizer.customize()
        libhlek_hash = hash_text_file(cmakelists_path)
        update_caches(LIBHLEK_HASH_KEY, libhlek_hash)
        print("libhlek customization done.")
    else:
        print("libhlek didn't change.")

def customize_hlekio(json_file_name: str):
    json_base_name = os.path.basename(json_file_name)
    hash_key = f'{json_base_name}_hash'
    customizer_path = os.path.join(project_dir, "customizer/customizer")
    configuration, configuration_hash = load_json(json_file_name)

    configuration["global"] = global_config

    # region CHECK_CONFIG
    if KW_BOARD not in configuration:
        raise RuntimeError(f'"{KW_BOARD}" section is not specified.')
    if KW_DISTRO not in configuration:
        raise RuntimeError(f'"{KW_DISTRO}" section is not specified.')
    if KW_INPUTS not in configuration:
        raise RuntimeError(f'"{KW_INPUTS}" section is not specified.')
    if KW_OUTPUTS not in configuration:
        raise RuntimeError(f'"{KW_OUTPUTS}" section is not specified.')
    # endregion

    # Check if customization is actually required
    if not is_outdated(hash_key, configuration_hash):
        print(f'No changes for {json_file_name} detected, exiting ...')
        return

    hlekio_customizer = HLEKIOCustomizer(configuration)
    hlekio_customizer.customize()

    config_text = json.dumps(configuration, indent=4, cls=Sorted_JSON_Encoder)

    update_caches(hash_key, configuration_hash)

    if opt_verbose:
        print(config_text)

#try:
KEY_VERBOSE = "--verbose"
KEY_IGNORE_CACHE = "--ignore-cache"
KEY_LIBHLEK = "--libhlek"
KEY_HLEKIO = "--hlekio"
KEY_UPDATE_GLOBAL_CACHE = "--update-global"
KEY_JSON_FILENAME = "--json"

defaults = {
    # KEY                       VALUE   INCOMPATIBLE ARGUMENT LIST
    KEY_VERBOSE:                [False, []],
    KEY_IGNORE_CACHE:           [False, [KEY_UPDATE_GLOBAL_CACHE]],
    KEY_LIBHLEK:                [False, [KEY_JSON_FILENAME, KEY_UPDATE_GLOBAL_CACHE]],
    KEY_HLEKIO:                 ["",    [KEY_LIBHLEK, KEY_JSON_FILENAME, KEY_UPDATE_GLOBAL_CACHE]],
    KEY_UPDATE_GLOBAL_CACHE:    [False, [KEY_IGNORE_CACHE, KEY_LIBHLEK, KEY_JSON_FILENAME]],
    KEY_JSON_FILENAME:          ["",    [KEY_LIBHLEK, KEY_UPDATE_GLOBAL_CACHE]]
}

parsed_args = parse_cmd_line(sys.argv[1:], defaults)
opt_verbose = parsed_args[KEY_VERBOSE][0]
opt_ignore_cache = parsed_args[KEY_IGNORE_CACHE][0]
opt_update_global_cache = parsed_args[KEY_UPDATE_GLOBAL_CACHE][0]
opt_libhlek = parsed_args[KEY_LIBHLEK][0]
json_hlekio = parsed_args[KEY_HLEKIO][0]
json_file_name = parsed_args[KEY_JSON_FILENAME][0]

project_dir = os.path.abspath(get_env_var("PROJECTDIR"))

GLOBAL_HASH_KEY = "global"
GLOBAL_JSON = os.path.join(project_dir, "global.json")
global_config, global_hash = load_json(GLOBAL_JSON)

HASHES_JSON = os.path.join(project_dir, "hashes")
if not os.path.isfile(HASHES_JSON):
    write_text_file(HASHES_JSON, """{
}""")
hashes, hashes_hash = load_json(HASHES_JSON)

if opt_libhlek:
    customize_libhlek()
elif json_hlekio:
    customize_hlekio(json_hlekio)
elif json_file_name:
    customize_firmware(json_file_name)

if opt_update_global_cache:
    update_caches(GLOBAL_HASH_KEY, global_hash)

#except Exception as ex:
#    print(
#f"""Error occured: {str(ex)}
#{sys.exc_info()[2].tb_frame.f_code}""")
#    quit(1)

