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
 *   \brief Generated include header of software part for GPIO device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <libhlek/gpio_common.hpp>

/// \addtogroup group_gpio_dev
/// @{{

/// \def Macro that indicates gpio is being used and all variables may be linked to.
/// \note This doesn't work in multi firmware configuration because macro definitions
///       may not be separated by namespaces
#define GPIODEV_DEVICE_ENABLED 1

namespace {__NAMESPACE_NAME__} {{

    /// \defgroup group_gpio_dev_pin_indexes GPIO pin indexes
    /// @{{
    {__CPP_GPIO_DEV_PINS_DECLARATION__}
    /// @}}

    extern const struct GPIOConfig* gpio_config_ptr;
}}
/// @}}
