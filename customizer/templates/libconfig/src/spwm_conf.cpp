#include "spwm_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    /// \brief SPWMDev virtual device name as configured in JSON configuration file.
    const char* SPWM_DEVICE_NAME = "{__SPWM_DEVICE_NAME__}";

    const SPWMChannel spwm_channels[] = {{ {__SPWM_SW_DESCRIPTION__} }};

    const SPWMConfig spwm_config = {{
        {__DEVICE_ID__},
        SPWM_DEVICE_NAME,
        {__SPWM_DEF_FREQ__},
        {__SPWM_PRESCALE_VALUE__},
        {__SPWM_PORT_COUNT__},
        {__SPWM_CHANNEL_COUNT__},
        spwm_channels
    }};

    const SPWMConfig* spwm_config_ptr = &spwm_config;
}}
