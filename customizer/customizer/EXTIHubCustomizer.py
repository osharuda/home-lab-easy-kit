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
from tools import *
import os
import glob
from shutil import copy

class EXTIHubCustomizer(BaseDeviceCustomizer):
    def __init__(self, mcu_hw, common_config):
        super().__init__(mcu_hw, None, common_config)
        self.device_name = "EXTIHUB"
        self.exti_dict = dict()
        self.fw_header = "exti_conf.h"
        self.required_resources = None

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

    def register_exti(self, dev_name, dev_cfg):
        req_list = get_leaf_values(dev_cfg)
        for res in req_list:
            rtype, rname = self.get_resource_by_name(res)
            if rtype=="exti_line":
                if rname in self.exti_dict.keys():
                    raise RuntimeError("Conflicting exti_line ({0}) detected in devices {1} and {2}".format(rname,
                                                                                                            self.exti_dict[rname],
                                                                                                            dev_name))
                self.exti_dict[rname]=dev_name

    def enabled(self) -> bool:
        return len(self.exti_dict)!=0

    def customize(self, configuration):
        if not self.enabled():
            return

        configuration["devices"]["EXTIHubCustomizer"] = {self.device_name: {"requires": dict()}}
        config_dict = configuration["devices"]["EXTIHubCustomizer"]
        self.require_feature("SYSTICK", config_dict)

        dev_requires = config_dict[self.device_name]["requires"]
        irq_handlers = set()
        exti_irq_defines = []

        # #define EXTIHUB_LINE_TO_IRQN {__EXTIHUB_LINE_TO_IRQN__}
        # compose EXTIHUB_LINE_TO_IRQN irqn
        exti_line_to_irqn = []
        exti_line_count = len(self.get_resources_by_type("exti_line"))
        for i in range(0, exti_line_count):
            hndl = self.mcu_hw.EXTINum_to_EXTIHandler(i)
            exti_line_to_irqn.append(self.mcu_hw.ISRHandler_to_IRQn(hndl))


        # get all required IRQ Handlers
        for el in self.exti_dict.keys():
            irq_handlers.add(self.mcu_hw.EXTIline_to_EXTIHandler(el))

        irq_handlers_sorted = set_to_ordered_list(irq_handlers)
        index = 0
        for exti_irq_handler in irq_handlers_sorted:
            exti_irq_defines.append("EXTIHUB_IRQ_HANDLER({0}) \\".format(exti_irq_handler))

            # add requirement for this IRQ handler
            dev_requires["exti_irq_{0}".format(index)] = {"irq_handler": exti_irq_handler}
            index = index + 1

        vocabulary = {"__EXTIHUB_ENABLED__": 1,
                      "__EXTIHUB_IRQ_HANDLERS__": concat_lines(exti_irq_defines)[:-1],
                      "__EXTIHUB_LINE_TO_IRQN__": ", ".join(exti_line_to_irqn),
                      "__EXTIHUB_LINE_COUNT__": exti_line_count}
        self.patch_templates(vocabulary)

        return dev_requires

