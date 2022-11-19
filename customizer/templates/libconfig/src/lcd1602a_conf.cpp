#include "lcd1602a_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    const char* LCD1602a_DEVICE_NAME = "{__LCD1602a_DEVICE_NAME__}";

    const LCD1602aConfig lcd1602a_config = {{
        {__DEVICE_ID__},
        LCD1602a_DEVICE_NAME
    }};
    const LCD1602aConfig* lcd1602a_config_ptr = &lcd1602a_config;
}}

