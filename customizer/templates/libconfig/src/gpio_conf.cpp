#include "gpio_conf.hpp"

namespace {__NAMESPACE_NAME__} {{

    const char* GPIO_DEVICE_NAME = "{__GPIO_DEVICE_NAME__}";

    /// \brief Defines array with descriptors for all configured pins.
    const GPIOPin GPIO_DESCRIPTOR[] = {{ {__GPIO_SW_DEV_DESCRIPTION__} }};

    const GPIOConfig gpio_config = {{
        {__DEVICE_ID__},
        GPIO_DEVICE_NAME,
        {__GPIO_PIN_COUNT__},
        GPIO_DESCRIPTOR
    }};
    const GPIOConfig* gpio_config_ptr = &gpio_config;
}}