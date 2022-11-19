#include "desk_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    const char* DESKDEV_DEVICE_NAME = "{__DESKDEV_DEVICE_NAME__}";

    const DeskConfig desk_config = {{
        {__DEVICE_ID__},
        DESKDEV_DEVICE_NAME
    }};
    const DeskConfig* desk_config_ptr = &desk_config;
}}