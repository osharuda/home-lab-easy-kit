#include "irrc_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    const char* IRRC_DEVICE_NAME = "{__IRRC_DEVICE_NAME__}";

    const IRRCConfig irrc_config = {{
        {__DEVICE_ID__},
        IRRC_DEVICE_NAME,
        {__IRRC_BUF_LEN__}
    }};
    const IRRCConfig* irrc_config_ptr = &irrc_config;
}}