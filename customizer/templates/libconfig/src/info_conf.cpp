#include "info_conf.hpp"

namespace {__NAMESPACE_NAME__}
{{
    const InfoConfig info_config {{
        {__DEVICE_ID__},
        "{__INFO_PROJECT_NAME__}",
        {{ {__INFO_UUID__} }},
        {{ {__INFO_DEVICES__} }} }};

    const InfoConfig* info_config_ptr = &info_config;
}}