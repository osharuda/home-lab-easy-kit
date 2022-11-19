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
 *   \brief Generated include header of firmware part for IRRC (Infra Red Remote Control) device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define IRRC_DEVICE_ENABLED             1

{__CONTROLS_SHARED_HEADER__}

#define IRRC_ADDR						{__DEVICE_ID__}
#define IRRC_BUF_LEN 					{__IRRC_BUF_LEN__}

#define IRRC_OUT_PORT			        {__IRRC_OUT_PORT__}
#define IRRC_OUT_PIN			        {__IRRC_OUT_PIN__}
#define IRRC_OUT_PIN_MASK               {__IRRC_OUT_PIN_MASK__}

#define IRRC_EXTICR                     {__IRRC_EXTICR__}
