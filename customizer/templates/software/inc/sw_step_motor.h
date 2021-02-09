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
 *   \brief Generated include header of software part for Stepper Motor device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#include <cstddef>

#define STEP_MOTOR_DEVICE_ENABLED     1

{__STEP_MOTOR_SHARED_HEADER__}

#pragma pack(push, 1)

{__SW_STEP_MOTOR_DESCRIPTORS__}

{__SW_STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS__}
// Define device definitions here

// Define for all step_motor definition
#define SW_STEP_MOTOR_DEVICE_COUNT {__STEP_MOTOR_DEVICE_COUNT__}
#define SW_STEP_MOTOR_DEVICE_DESCRIPTORS {{ {__SW_STEP_MOTOR_DEVICE_DESCRIPTORS__} }}

#pragma pack(pop)

