#include "{devname}_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    /// \brief Defines array with configurations for {DEVNAME} virtual devices.
    const {DevName}Config {__{DEVNAME}_CONFIGURATION_ARRAY_NAME__}[] = {{
        {__{DEVNAME}_SW_DEV_DESCRIPTOR__}
    }};

    {__{DEVNAME}_CONFIGURATIONS__}
}}