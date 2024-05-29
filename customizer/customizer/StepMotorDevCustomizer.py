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


class StepMotorDevCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_configs, common_config):
        super().__init__(mcu_hw, dev_configs, common_config, "STEP_MOTOR")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "step_motor_conf.hpp"
        self.sw_lib_source = "step_motor_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])
        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)

    def customize(self):
        indx = 0
        fw_device_descriptors = ["#define STEP_MOTOR_DEVICE_DESCRIPTORS \\"]
        fw_motor_buffers = ["#define STEP_MOTORS_BUFFERS \\"]
        fw_motor_dev_counts = []
        fw_dev_status_buffers = ["#define STEP_MOTOR_DEV_STATUS_BUFFER \\"]
        fw_dev_status_buffer_names = []
        timer_irq_handler_list = []
        fw_motor_descriptors = ["#define STEP_MOTOR_MOTOR_DESCRIPTORS \\"]
        fw_motor_descriptor_arrays = ["#define STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS \\"]
        fw_motor_context_arrays = ["#define STEP_MOTOR_MOTOR_CONTEXT_ARRAYS \\"]
        fw_motor_status_arrays = ["#define STEP_MOTOR_MOTOR_STATUS_ARRAYS \\"]
        fw_devices_array = []
        sw_motor_descriptors = []
        sw_motor_descriptor_arrays = []
        sw_devices_descriptors = []
        sw_config_array_name = "step_motor_configs"
        sw_config_declarations = []
        sw_configs = []

        for dev_name, dev_config in self.device_list:
            self.require_feature("SYSTICK", dev_config)
            dev_id = dev_config["dev_id"]
            motors_cfg = dev_config["motors"]
            dev_requires = dev_config[KW_REQUIRES]
            motors_count = len(motors_cfg)
            timer = self.get_timer(dev_requires)
            timer_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(timer)
            timer_irqn = self.mcu_hw.ISRHandler_to_IRQn(timer_irq_handler)
            dev_descriptor_name = "g_" + dev_name.lower() + "_devices"
            dev_motor_descr_list = []
            sw_dev_motor_descr_list = []

            timer_irq_handler_list.append(
                "MAKE_ISR_WITH_INDEX({0}, STEP_MOTOR_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(timer_irq_handler, indx))
            dev_requires[dev_name + "_irq_handler"] = {"irq_handler": timer_irq_handler}

            # motor count defines
            dev_mcount_def = "STEP_MOTOR_" + dev_name.upper() + "_MOTOR_COUNT"
            dev_mstatus_size = "STEP_MOTOR_" + dev_name.upper() + "_STATUS_SIZE"
            fw_motor_dev_counts.append("#define {0} ({1})".format(dev_mcount_def, motors_count))
            fw_motor_dev_counts.append(
                "#define {0} (sizeof(StepMotorDevStatus) + {1}*sizeof(StepMotorStatus))".format(dev_mstatus_size,
                                                                                                dev_mcount_def))

            # motor status structures
            dev_status_name = "g_" + dev_name.lower() + "_status_buffer"
            fw_dev_status_buffers.append(
                "volatile uint8_t {0}[{1}] = {{0}}; \\".format(dev_status_name, dev_mstatus_size))
            fw_dev_status_buffer_names = []

            for mot_name, mot_cfg in motors_cfg.items():
                mot_var_prefix = "g_" + dev_name.lower() + "_" + mot_name.lower()

                # Drive type
                drive_type = self.get_drive_type(mot_name, mot_cfg, dev_name)

                used_pins = self.get_driver_gpio(mot_name, mot_cfg, dev_name, dev_requires)
                # Motor buffers
                mot_buf_size = int(mot_cfg["buffer_size"])
                mot_buffer_name = mot_var_prefix + "_buffer"
                fw_motor_buffers.append(self.tab + "uint8_t {0}[{1}] = {{0}}; \\".format(mot_buffer_name, mot_buf_size))

                # Motor default speed
                default_speed = int(mot_cfg.get("default_speed", 1000000))

                # Motor descriptors
                mot_descriptor_name = mot_var_prefix + "_descriptor"
                mot_descriptor = self.get_motor_description(True, used_pins, mot_name, mot_cfg, dev_name, dev_requires,
                                                            mot_buffer_name, mot_buf_size, default_speed, drive_type)
                fw_motor_descriptors.append(
                    "volatile StepMotorDescriptor {0} = {{ {1} }}; \\".format(mot_descriptor_name, mot_descriptor))
                dev_motor_descr_list.append("&" + mot_descriptor_name)
                sw_dev_motor_descr_list.append("&" + mot_descriptor_name)

                mot_descriptor = self.get_motor_description(False, used_pins, mot_name, mot_cfg, dev_name, dev_requires,
                                                            mot_buffer_name, mot_buf_size, default_speed, drive_type)
                sw_motor_descriptors.append(
                    "const StepMotorDescriptor {0} = {{ {1} }};".format(mot_descriptor_name, mot_descriptor))

            # Motor descriptors
            motor_descriptors_array_name = "g_" + dev_name.lower() + "_motor_descriptors"
            fw_motor_descriptor_arrays.append(
                "volatile StepMotorDescriptor* {0}[] = {{ {1} }};\\".format(motor_descriptors_array_name,
                                                                            ", ".join(dev_motor_descr_list)))
            sw_motor_descriptor_arrays.append(
                "const StepMotorDescriptor* {0}[] = {{ {1} }};".format(motor_descriptors_array_name,
                                                                           ", ".join(sw_dev_motor_descr_list)))

            # Motor contexts
            motor_context_arrays_name = "g_" + dev_name.lower() + "_motor_contexts"
            fw_motor_context_arrays.append(
                "volatile StepMotorContext {0}[{1}] = {{0}}; \\".format(motor_context_arrays_name, motors_count))

            # Motor statuses
            motor_status_arrays_name = "g_" + dev_name.lower() + "_motor_statuses"
            fw_motor_status_arrays.append(
                "volatile StepMotorStatus {0}[{1}] = {{0}}; \\".format(motor_status_arrays_name, motors_count))

            # Device descriptors
            dev_motor_descr_list.append(dev_descriptor_name)
            fw_device_descriptors.append(
                "volatile StepMotorDevice {0} = {{ {{0}}, {{0}}, {4}, (volatile PStepMotorContext){6}, (volatile PStepMotorDevStatus){7}, {8}, {5}, (volatile StepMotorDescriptor**){3}, {2}, {1} }}; \\".format(
                    dev_descriptor_name,                #0
                    dev_id,                             #1
                    motors_count,                       #2
                    motor_descriptors_array_name,       #3
                    timer,                              #4
                    timer_irqn,                         #5
                    motor_context_arrays_name,          #6
                    dev_status_name,                    #7
                    dev_mstatus_size))                  #8

            sw_devices_descriptors.append(
                '{{ "{3}", {2}, {1}, {0} }}'.format(dev_id, motors_count, motor_descriptors_array_name, dev_name))

            fw_devices_array.append("(volatile PStepMotorDevice)&" + dev_descriptor_name)

            sw_config_name = "step_motor_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const StepMotorConfig* {sw_config_name};")
            sw_configs.append(
                f"const StepMotorConfig* {sw_config_name} = {sw_config_array_name} + {indx};")

            indx += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__STEP_MOTORS_BUFFERS__": concat_lines(fw_motor_buffers)[:-1],
                      "__STEP_MOTOR_MOTOR_DESCRIPTORS__": concat_lines(fw_motor_descriptors)[:-1],
                      "__STEP_MOTOR_DEVICE_DESCRIPTORS__": concat_lines(fw_device_descriptors)[:-1],
                      "__STEP_MOTOR_MOTOR_DESCRIPTORS_ARRAYS__": concat_lines(fw_motor_descriptor_arrays)[:-1],
                      "__STEP_MOTOR_MOTOR_CONTEXT_ARRAYS__": concat_lines(fw_motor_context_arrays)[:-1],
                      "__STEP_MOTOR_MOTOR_STATUS_ARRAYS__": concat_lines(fw_motor_status_arrays)[:-1],
                      "__STEP_MOTOR_DEVICES__": ", ".join(fw_devices_array),
                      "__STEP_MOTOR_DEVICE_COUNT__": len(fw_devices_array),
                      "__STEP_MOTOR_FW_TIMER_IRQ_HANDLERS__": concat_lines(timer_irq_handler_list)[:-1],
                      "__STEP_MOTORS_MOTOR_COUNTS__": concat_lines(fw_motor_dev_counts),
                      "__STEP_MOTORS_DEV_STATUSES__": concat_lines(fw_dev_status_buffers)[:-1],
                      "__SW_STEP_MOTOR_DESCRIPTORS__": concat_lines(sw_motor_descriptors),
                      "__SW_STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS__": concat_lines(sw_motor_descriptor_arrays),
                      "__SW_STEP_MOTOR_DEVICE_DESCRIPTORS__": ", ".join(sw_devices_descriptors),
                      "__STEP_MOTOR_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__STEP_MOTOR_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__STEP_MOTOR_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
                      }

        self.patch_templates()

        return

    def get_driver_gpio(self, mname: str, mcfg: dict, dev_name: str, dev_requires: dict) -> tuple:
        if "step" not in mcfg:
            raise RuntimeError(
                "step pin is not specified in device {0} for the motor definition for {1}".format(dev_name, mname))

        step_pin = self.get_gpio_with_default(mcfg.get("step"))
        dir_pin = self.get_gpio_with_default(mcfg.get("dir"))
        ms1_pin = self.get_gpio_with_default(mcfg.get("m1"))
        ms2_pin = self.get_gpio_with_default(mcfg.get("m2"))
        ms3_pin = self.get_gpio_with_default(mcfg.get("m3"))
        enable_pin = self.get_gpio_with_default(mcfg.get("enable"))
        reset_pin = self.get_gpio_with_default(mcfg.get("reset"))
        sleep_pin = self.get_gpio_with_default(mcfg.get("sleep"))
        fault_pin = self.get_gpio_with_default(mcfg.get("fault"))
        cw_endstop_pin = self.get_gpio_with_default(mcfg.get("cw_endstop"))
        ccw_endstop_pin = self.get_gpio_with_default(mcfg.get("ccw_endstop"))

        dev_requires[mname + "_step_gpio"] = {"gpio": step_pin}
        if dir_pin is not None:
            dev_requires[mname + "_dir_gpio"] = {"gpio": dir_pin}

        if ms1_pin is not None:
            dev_requires[mname + "_ms1_gpio"] = {"gpio": ms1_pin}

        if ms2_pin is not None:
            dev_requires[mname + "_ms2_gpio"] = {"gpio": ms2_pin}

        if ms3_pin is not None:
            dev_requires[mname + "_ms3_gpio"] = {"gpio": ms3_pin}

        if enable_pin is not None:
            dev_requires[mname + "_enable_gpio"] = {"gpio": enable_pin}

        if reset_pin is not None:
            dev_requires[mname + "_reset_gpio"] = {"gpio": reset_pin}

        if sleep_pin is not None:
            dev_requires[mname + "_sleep_gpio"] = {"gpio": sleep_pin}

        if fault_pin is not None:
            dev_requires[mname + "_fault_gpio"] = {"gpio": fault_pin}

        if cw_endstop_pin is not None:
            dev_requires[mname + "_cw_endstop_gpio"] = {"gpio": cw_endstop_pin}

        if ccw_endstop_pin is not None:
            dev_requires[mname + "_ccw_endstop_gpio"] = {"gpio": ccw_endstop_pin}

        return (
        step_pin, dir_pin, ms1_pin, ms2_pin, ms3_pin, enable_pin, reset_pin, sleep_pin, fault_pin, cw_endstop_pin,
        ccw_endstop_pin)

    def get_motor_description(self, for_firmware: bool, used_pins: tuple, mname: str, mcfg: dict, dev_name: str,
                              dev_requires: dict, buf_name: str, buf_size: int, default_speed: int, driver_type: str) -> str:
        step_pin, dir_pin, m1_pin, m2_pin, m3_pin, enable_pin, reset_pin, sleep_pin, fault_pin, cw_endstop_pin, ccw_endstop_pin = used_pins
        config_flags = []
        indention = self.tab
        drive_type_dict = {"unknown" : "STEP_MOTOR_DRIVER_UNKNOWN",
                           "a4998"   : "STEP_MOTOR_DRIVER_A4998",
                           "drv8825" : "STEP_MOTOR_DRIVER_DRV8825"}
        template = """ \\
{__config_flags__},\\
{__buffer_size__},\\
{__default_speed__},\\
{__motor_driver__},\\
{__cw_sft_limit__},\\
{__ccw_sft_limit__},\\"""

        fw_template = """
{__buffer__},\\
{{ {{ {__step_port__}, {__step_pin__} }},\\
  {{ {__dir_port__}, {__dir_pin__} }},\\
  {{ {__m1_port__}, {__m1_pin__} }},\\
  {{ {__m2_port__}, {__m2_pin__} }},\\
  {{ {__m3_port__}, {__m3_pin__} }},\\
  {{ {__enable_port__}, {__enable_pin__} }},\\
  {{ {__reset_port__}, {__reset_pin__} }},\\
  {{ {__sleep_port__}, {__sleep_pin__} }},\\
  {{ {__fault_port__}, {__fault_pin__} }},\\
  {{ {__cw_endstop_port__}, {__cw_endstop_pin__} }},\\
  {{ {__ccw_endstop_port__}, {__ccw_endstop_pin__} }} }},\\
{__fault_exticr__},\\
{__cw_endstop_exticr__},\\
{__ccw_endstop_exticr__}"""

        sw_template = """
{__motor_name__},\\
{__steps_per_revolution__}"""

        template += fw_template if for_firmware else sw_template
        steps_per_revolution = int(
            self.get_config_option(None, "steps_per_revolution", mname, mcfg, dev_name, None, None))

        self.vocabulary = self.vocabulary | {
                      "__step_port__": indention + self.mcu_hw.GPIO_to_port(step_pin),
                      "__step_pin__": indention + str(self.mcu_hw.GPIO_to_pin_number(step_pin)),
                      "__default_speed__": indention + str(default_speed),
                      "__motor_driver__" : indention + drive_type_dict[driver_type],
                      "__motor_name__": indention + '"' + mname + '"',
                      "__steps_per_revolution__": indention + str(steps_per_revolution)}

        # ------------------------------- DIRECTION PIN -------------------------------
        if dir_pin is not None:
            self.vocabulary["__dir_port__"] = indention + self.mcu_hw.GPIO_to_port(dir_pin)
            self.vocabulary["__dir_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(dir_pin))
            config_flags.append(indention + "STEP_MOTOR_DIR_IN_USE |\\")
        else:
            self.vocabulary["__dir_port__"] = indention + "0"
            self.vocabulary["__dir_pin__"] = indention + "0"

        dv = self.get_config_option("dir", "default", mname, mcfg, dev_name, None, {"CW", "CCW"})
        if dv == "CW":
            config_flags.append(indention + "STEP_MOTOR_DIRECTION_CW |\\")

        # ------------------------------- M1 PIN -------------------------------
        if m1_pin is not None:
            self.vocabulary["__m1_port__"] = indention + self.mcu_hw.GPIO_to_port(m1_pin)
            self.vocabulary["__m1_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(m1_pin))
            config_flags.append(indention + "STEP_MOTOR_M1_IN_USE |\\")
        else:
            self.vocabulary["__m1_port__"] = indention + "0"
            self.vocabulary["__m1_pin__"] = indention + "0"

        dv = self.get_config_option("m1", "default", mname, mcfg, dev_name, None, {1, 0})
        if dv == 1:
            config_flags.append(indention + "STEP_MOTOR_M1_DEFAULT |\\")

        # ------------------------------- M2 PIN -------------------------------
        if m2_pin is not None:
            self.vocabulary["__m2_port__"] = indention + self.mcu_hw.GPIO_to_port(m2_pin)
            self.vocabulary["__m2_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(m2_pin))
            config_flags.append(indention + "STEP_MOTOR_M2_IN_USE |\\")
        else:
            self.vocabulary["__m2_port__"] = indention + "0"
            self.vocabulary["__m2_pin__"] = indention + "0"

        dv = self.get_config_option("m2", "default", mname, mcfg, dev_name, None, {1, 0})
        if dv == 1:
            config_flags.append(indention + "STEP_MOTOR_M2_DEFAULT |\\")

        # ------------------------------- M3 PIN -------------------------------
        if m3_pin is not None:
            self.vocabulary["__m3_port__"] = indention + self.mcu_hw.GPIO_to_port(m3_pin)
            self.vocabulary["__m3_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(m3_pin))
            config_flags.append(indention + "STEP_MOTOR_M3_IN_USE |\\")
        else:
            self.vocabulary["__m3_port__"] = indention + "0"
            self.vocabulary["__m3_pin__"] = indention + "0"

        dv = self.get_config_option("m3", "default", mname, mcfg, dev_name, None, {1, 0})
        if dv == 1:
            config_flags.append(indention + "STEP_MOTOR_M3_DEFAULT |\\")

        # ------------------------------- ENABLE PIN -------------------------------
        if enable_pin is not None:
            self.vocabulary["__enable_port__"] = indention + self.mcu_hw.GPIO_to_port(enable_pin)
            self.vocabulary["__enable_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(enable_pin))
            config_flags.append(indention + "STEP_MOTOR_ENABLE_IN_USE |\\")
        else:
            self.vocabulary["__enable_port__"] = indention + "0"
            self.vocabulary["__enable_pin__"] = indention + "0"

        dv = self.get_config_option("enable", "default", mname, mcfg, dev_name, None, {"enable", "disable"})
        if dv == "disable":
            config_flags.append(indention + "STEP_MOTOR_DISABLE_DEFAULT |\\")

        # ------------------------------- RESET PIN -------------------------------
        if reset_pin is not None:
            self.vocabulary["__reset_port__"] = indention + self.mcu_hw.GPIO_to_port(reset_pin)
            self.vocabulary["__reset_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(reset_pin))
            config_flags.append(indention + "STEP_MOTOR_RESET_IN_USE |\\")
        else:
            self.vocabulary["__reset_port__"] = indention + "0"
            self.vocabulary["__reset_pin__"] = indention + "0"

        # ------------------------------- SLEEP PIN -------------------------------
        if sleep_pin is not None:
            self.vocabulary["__sleep_port__"] = indention + self.mcu_hw.GPIO_to_port(sleep_pin)
            self.vocabulary["__sleep_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(sleep_pin))
            config_flags.append(indention + "STEP_MOTOR_SLEEP_IN_USE |\\")
        else:
            self.vocabulary["__sleep_port__"] = indention + "0"
            self.vocabulary["__sleep_pin__"] = indention + "0"

        dv = self.get_config_option("sleep", "default", mname, mcfg, dev_name, None, {"sleep", "wakeup"})
        if dv == "wakeup":
            config_flags.append(indention + "STEP_MOTOR_WAKEUP_DEFAULT |\\")

        # ------------------------------- ERROR CONFIGURATION -------------------------------
        dv = self.get_config_option(None, "error_action", mname, mcfg, dev_name, None, {"stop", "stop_all"})
        if dv == "stop_all":
            config_flags.append(indention + "STEP_MOTOR_CONFIG_ERROR_ALL |\\")

        # ------------------------------- FAULT PIN -------------------------------
        if fault_pin is not None:
            self.vocabulary["__fault_port__"] = indention + self.mcu_hw.GPIO_to_port(fault_pin)
            self.vocabulary["__fault_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(fault_pin))
            self.vocabulary["__fault_exticr__"] = indention + self.mcu_hw.GPIO_to_AFIO_EXTICR(fault_pin)
            dev_requires["{0}_{1}_fault_extiline".format(dev_name.lower(), mname.lower())] = {
                "exti_line": self.mcu_hw.GPIO_to_EXTI_line(fault_pin)}
            active_level = self.get_config_option("fault", "active_level", mname, mcfg, dev_name, None, {"high", "low"})
            if active_level == "high":
                config_flags.append(indention + "STEP_MOTOR_FAULT_ACTIVE_HIGH |\\")
            config_flags.append(indention + "STEP_MOTOR_FAULT_IN_USE |\\")

            dv = self.get_config_option("fault", "action", mname, mcfg, dev_name, None, {"ignore", "stop", "stop_all"})
            if dv != "stop":
                config_flags.append(indention + self.get_action_flag("STEP_MOTOR_CONFIG_FAILURE", dv) + " |\\")
        else:
            self.vocabulary["__fault_port__"] = indention + "0"
            self.vocabulary["__fault_pin__"] = indention + "0"
            self.vocabulary["__fault_exticr__"] = indention + "0"

        dv = self.get_config_option("sleep", "default", mname, mcfg, dev_name, None, {"sleep", "wakeup"})
        if dv == "wakeup":
            config_flags.append(indention + "STEP_MOTOR_WAKEUP_DEFAULT |\\")

        # ------------------------------- CW ENCODER PIN -------------------------------
        if cw_endstop_pin is not None:
            self.vocabulary["__cw_sft_limit__"] = indention + "0"
            self.vocabulary["__cw_endstop_port__"] = indention + self.mcu_hw.GPIO_to_port(cw_endstop_pin)
            self.vocabulary["__cw_endstop_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(cw_endstop_pin))
            self.vocabulary["__cw_endstop_exticr__"] = indention + self.mcu_hw.GPIO_to_AFIO_EXTICR(cw_endstop_pin)
            dev_requires["{0}_{1}_cw_endstop_extiline".format(dev_name.lower(), mname.lower())] = {
                "exti_line": self.mcu_hw.GPIO_to_EXTI_line(cw_endstop_pin)}
            active_level = self.get_config_option("cw_endstop", "active_level", mname, mcfg, dev_name, None,
                                                  {"high", "low"})
            if active_level == "high":
                config_flags.append(indention + "STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH |\\")
            config_flags.append(indention + "STEP_MOTOR_CWENDSTOP_IN_USE |\\")
        else:
            self.vocabulary["__cw_endstop_port__"] = indention + "0"
            self.vocabulary["__cw_endstop_pin__"] = indention + "0"
            self.vocabulary["__cw_endstop_exticr__"] = indention + "0"
            cw_sft_limit = self.get_int_config_option("cw_endstop", "position_limit", mname, mcfg, dev_name, None)
            self.vocabulary["__cw_sft_limit__"] = indention + str(cw_sft_limit)

        dv = self.get_config_option("cw_endstop", "action", mname, mcfg, dev_name, None, {"ignore", "stop", "stop_all"})
        if dv != "stop":
            config_flags.append(indention + self.get_action_flag("STEP_MOTOR_CONFIG_CW_ENDSTOP", dv) + " |\\")

        # ------------------------------- CCW ENCODER PIN -------------------------------
        if ccw_endstop_pin is not None:
            self.vocabulary["__ccw_sft_limit__"] = indention + "0"
            self.vocabulary["__ccw_endstop_port__"] = indention + self.mcu_hw.GPIO_to_port(ccw_endstop_pin)
            self.vocabulary["__ccw_endstop_pin__"] = indention + str(self.mcu_hw.GPIO_to_pin_number(ccw_endstop_pin))
            self.vocabulary["__ccw_endstop_exticr__"] = indention + self.mcu_hw.GPIO_to_AFIO_EXTICR(ccw_endstop_pin)
            dev_requires["{0}_{1}_ccw_endstop_extiline".format(dev_name.lower(), mname.lower())] = {
                "exti_line": self.mcu_hw.GPIO_to_EXTI_line(ccw_endstop_pin)}
            active_level = self.get_config_option("ccw_endstop", "active_level", mname, mcfg, dev_name, None,
                                                  {"high", "low"})
            if active_level == "high":
                config_flags.append(indention + "STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH |\\")
            config_flags.append(indention + "STEP_MOTOR_CCWENDSTOP_IN_USE |\\")

        else:
            self.vocabulary["__ccw_endstop_port__"] = indention + "0"
            self.vocabulary["__ccw_endstop_pin__"] = indention + "0"
            self.vocabulary["__ccw_endstop_exticr__"] = indention + "0"
            ccw_sft_limit = self.get_int_config_option("ccw_endstop", "position_limit", mname, mcfg, dev_name, None)
            self.vocabulary["__ccw_sft_limit__"] = indention + str(ccw_sft_limit)

        dv = self.get_config_option("ccw_endstop", "action", mname, mcfg, dev_name, None, {"ignore", "stop", "stop_all"})
        if dv != "stop":
            config_flags.append(indention + self.get_action_flag("STEP_MOTOR_CONFIG_CCW_ENDSTOP", dv) + " |\\")

        # ------------------------------- Check software endstops are correct -------------------------------
        if cw_endstop_pin is None and ccw_endstop_pin is None and cw_sft_limit <= ccw_sft_limit:
            raise RuntimeError("Device {0} motor {1} : Software limit for CW direction must be greater than software limit for CCW direction".format(dev_name, mname));




        if len(config_flags) == 0:
            config_flags.append(indention + "0|\\")

        self.vocabulary["__config_flags__"] = concat_lines(config_flags)[:-2]
        self.vocabulary["__buffer__"] = indention + buf_name
        self.vocabulary["__buffer_size__"] = indention + str(buf_size)

        return template.format(**self.vocabulary)

    def get_action_flag(self, purpose, action : str):
        action_map = {"ignore": "_IGNORE", "stop_all": "_ALL"}
        return purpose+action_map[action]

    def get_int_config_option(self, line, opt: str, mname: str, mcfg: dict, dev_name: str, default)-> int:
        co = self.get_config_option(line,opt, mname, mcfg, dev_name, None, None)
        try:
            res = int(co)
        except ValueError:
            raise RuntimeError("Device '{0}' motor '{1}' line '{2}' has value '{3}' of not integer type.".format(dev_name, mname, line, opt))

        return res

    def get_config_option(self, line, opt: str, mname: str, mcfg: dict, dev_name: str, default,
                          allowed_values) -> str:
        if line is not None:
            if line not in mcfg:
                raise RuntimeError("Line {2} is not defined for motor {0} in device {1}".format(mname, dev_name, line))
            linecfg = mcfg[line]
        else:
            linecfg = mcfg

        if opt in linecfg:
            val = linecfg[opt]
        else:
            val = default

        if val is None:
            # This option must be specified, it's not optional
            raise RuntimeError(
                "Option {0} is mandatory, but it is not specified for the motor {1} in device {2}".format(opt, mname,
                                                                                                          dev_name))
        elif allowed_values is not None:
            if val in allowed_values:
                return val
            else:
                raise RuntimeError(
                    "Invalid {0} value specified for the motor {1} in device {2}".format(opt, mname, dev_name))
        else:
            return val

    def get_drive_type(self, mname: str, mcfg: dict, dev_name: str):
        dt = self.get_config_option(None, "drive_type", mname, mcfg, dev_name, "unknown",
                                    {"unknown", "a4998", "drv8825"})

        # calculate default microstep value
        if "m1" in mcfg:
            m1 = self.get_config_option("m1", "default", mname, mcfg, dev_name, None, {1, 0})
        else:
            m1 = 0

        if "m2" in mcfg:
            m2 = self.get_config_option("m2", "default", mname, mcfg, dev_name, None, {1, 0})
        else:
            m2 = 0

        if "m3" in mcfg:
            m3 = self.get_config_option("m3", "default", mname, mcfg, dev_name, None, {1, 0})
        else:
            m3 = 0

        microstep = m1 + (m2<<1) + (m3<<2)
        bad_ms_values = set()


        # sanitize fault pin (is not available in a4998)
        if dt == "a4998":
            bad_ms_values.update({4,5,6})
            if "fault" in mcfg:
                raise RuntimeError(
                    "Motor {0} in device {1} uses fault pin. However A4998 drivers don't it.".format(mname, dev_name))

        # check for incorrect default microstep values
        if microstep in bad_ms_values:
            raise RuntimeError("Motor {0} (drive type is: {2}) in device {1} uses unacceptable default value for microsteps(M1={3}, M2={4}, M3={5})".format(mname, dev_name, dt, m1, m2, m3))

        if dt == "unknown":
            return None
        else:
            return dt
