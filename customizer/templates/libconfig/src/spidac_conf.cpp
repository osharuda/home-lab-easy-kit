#include "spidac_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    {__SPIDAC_SW_CHANNEL_DESCRIPTORS__}

    /// \brief Defines array with configurations for SPIDAC virtual devices.
    const SPIDACConfig {__SPIDAC_CONFIGURATION_ARRAY_NAME__}[] = {{
        {__SPIDAC_SW_DEV_DESCRIPTOR__}
    }};

    {__SPIDAC_CONFIGURATIONS__}
}}