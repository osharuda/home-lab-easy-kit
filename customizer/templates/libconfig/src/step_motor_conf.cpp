#include "step_motor_conf.hpp"

namespace {__NAMESPACE_NAME__} {{

/// \brief Stepper motor definitions
{__SW_STEP_MOTOR_DESCRIPTORS__}

/// \brief Stepper motor groups
{__SW_STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS__}

/// \brief Describes all step motor devices devices.
const StepMotorConfig {__STEP_MOTOR_CONFIGURATION_ARRAY_NAME__}[] = {{
    {__SW_STEP_MOTOR_DEVICE_DESCRIPTORS__}
}};

{__STEP_MOTOR_CONFIGURATIONS__}

}}