#include "rtc_conf.hpp"

namespace {__NAMESPACE_NAME__} {{
    const char* RTC_DEVICE_NAME = "{__RTC_DEVICE_NAME__}";

    const RTCConfig rtc_config = {{
        {__DEVICE_ID__},
        RTC_DEVICE_NAME
    }};

    const RTCConfig* rtc_config_ptr = &rtc_config;
}}