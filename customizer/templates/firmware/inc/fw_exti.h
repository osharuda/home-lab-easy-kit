/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief Generated include header of firmware part for EXTI hub
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define EXTIHUB_DEVICE_ENABLED {__EXTIHUB_ENABLED__}

#define EXTIHUB_LINE_COUNT {__EXTIHUB_LINE_COUNT__}
#define EXTIHUB_LINE_TO_IRQN {{ {__EXTIHUB_LINE_TO_IRQN__} }}

#define EXTIHUB_IRQ_HANDLER(isr_name) MAKE_ISR(isr_name) {{EXTIHUB_COMMON_IRQ_HANDLER();}}
#define EXTIHUB_IRQ_HANDLERS {__EXTIHUB_IRQ_HANDLERS__}

