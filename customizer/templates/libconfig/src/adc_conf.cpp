#include "adc_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    {__ADCDEV_SW_CHANNELS__};

    /// \brief Defines array with configurations for ADCDev virtual devices
    const ADCConfig {__ADCDEV_CONFIGURATION_ARRAY_NAME__}[] = {{  {__ADCDEV_SW_DEV_DESCRIPTOR__} }};

    {__ADCDEV_CONFIGURATIONS__}
}}