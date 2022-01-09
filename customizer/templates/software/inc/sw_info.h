/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief Generated include header of software part for info device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#define INFO_DEVICE_ENABLED 1

/// \addtogroup group_info_dev
/// @{{

{__INFO_SHARED_HEADER__}

/// \def INFO_DEVICES
/// \brief Describes virtual devices available
#define INFO_DEVICES {{ {__INFO_DEVICES__} }}

/// \def INFO_PROJECT_NAME
/// \brief Project name
#define INFO_PROJECT_NAME (const char*)"{__INFO_PROJECT_NAME__}"

/// \def INFO_DESCRIPTION
/// \brief Description of the customized project
#define INFO_DESCRIPTION {{ INFO_UUID, INFO_DEVICES, INFO_PROJECT_NAME}}

/// \def INFO_DEVICES_NUMBER
/// \brief Number of virtual devices available
#define INFO_DEVICES_NUMBER {__INFO_DEVICES_NUMBER__}

/// \defgroup group_info_dev_dev_type Virtual device types
/// \brief Contains constants that define virtual device types
/// @{{
// ADD_DEV_TYPE
#define INFO_DEV_TYPE_NONE       (uint8_t)0
#define INFO_DEV_TYPE_INFO       (uint8_t)1
#define INFO_DEV_TYPE_DESKDEV    (uint8_t)2
#define INFO_DEV_TYPE_IRRC       (uint8_t)3
#define INFO_DEV_TYPE_LCD1602a   (uint8_t)4
#define INFO_DEV_TYPE_RTC        (uint8_t)5
#define INFO_DEV_TYPE_UART_PROXY (uint8_t)6
#define INFO_DEV_TYPE_GPIO       (uint8_t)7
#define INFO_DEV_TYPE_SPWM       (uint8_t)8
#define INFO_DEV_TYPE_ADC        (uint8_t)9
#define INFO_DEV_TYPE_STEP_MOTOR (uint8_t)10
#define INFO_DEV_TYPE_CAN        (uint8_t)11
// ADD_DEV_TYPE
/// @}}

/// \defgroup group_info_dev_dev_hint Virtual device hints
/// \brief Contains values that play role of hint and specify virtual device behaviour
/// @{{

/// \def INFO_DEV_HINT_NONE
/// \brief No hint available for this kind of device
#define INFO_DEV_HINT_NONE      (uint8_t)0

/// \def INFO_DEV_HINT_GSM_MODEM
/// \brief This hint specifies that device is a #GSMModem. Valid with #INFO_DEV_TYPE_UART_PROXY only.
#define INFO_DEV_HINT_GSM_MODEM (uint8_t)1

/// @}}

/// \struct tag_InfoDeviceDescriptor
/// \brief Describes virtual device
typedef struct tag_InfoDeviceDescriptor {{
    uint8_t type;   ///< Type of the virtual device (one of the @ref group_info_dev_dev_type values).
    uint8_t hint;   ///< Hint for the virtual device (one of the @ref group_info_dev_dev_hint values).
    const char* name; ///< Name of the virtual device.
}} InfoDeviceDescriptor, *PInfoDeviceDescriptor;

/// \def INFO_DEVICE_ADDRESSES
/// \brief Number of virtual device addresses available (including INFODev device)
#define INFO_DEVICE_ADDRESSES (COMM_MAX_DEV_ADDR+1)

/// \struct tag_InfoProjectData
/// \brief Describes all project information available to #INFODev
typedef struct tag_InfoProjectData {{
    uint8_t uuid[INFO_UUID_LEN];
    const InfoDeviceDescriptor devices[INFO_DEVICE_ADDRESSES];
    const char* name;
}} InfoProjectData, *PInfoProjectData;

/// @}}
