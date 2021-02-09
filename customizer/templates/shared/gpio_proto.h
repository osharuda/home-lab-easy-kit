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

 /* --------------------> END OF THE TEMPLATE HEADER <-------------------- */

/// \def GPIODEV_ADDR
/// \brief GPIODev device id
#define GPIODEV_ADDR						{__DEVICE_ID__}

/// \def GPIO_PIN_COUNT
/// \brief Number of pins configured
#define GPIO_PIN_COUNT         {__GPIO_PIN_COUNT__}

/// \defgroup group_gpio_dev_pin_indexes GPIO pin indexes
/// @{{
{__GPIO_DEV_PINS_DECLARATION__}
/// @}}