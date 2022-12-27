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

#include <stdint.h>

/// \addtogroup group_info_dev
/// @{{

{__INFO_SHARED_HEADER__}

/// \defgroup group_info_dev_dev_hint Virtual device hints
/// \brief Contains values that play role of hint and specify virtual device behaviour
/// @{{

/// \def INFO_DEV_HINT_NONE
/// \brief No hint available for this kind of device
#define INFO_DEV_HINT_NONE      (uint8_t)0

/// \def INFO_DEV_HINT_GSM_MODEM
/// \brief This hint specifies that device is a #GSMModem. Valid with #INFO_DEV_TYPE_UART_PROXY only.
#define INFO_DEV_HINT_GSM_MODEM (uint8_t)1

/// \def INFO_DEV_HINT_25LC640
/// \brief This hint specifies that device is a #25LC640. Valid with #INFO_DEV_TYPE_SPIPROXY only.
#define INFO_DEV_HINT_25LC640 (uint8_t)2

/// \def INFO_DEV_HINT_ADXL350
/// \brief This hint specifies that device is a #ADXL350. Valid with #INFO_DEV_TYPE_SPIPROXY only.
#define INFO_DEV_HINT_ADXL350 (uint8_t)3

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
#define INFO_DEVICE_ADDRESSES ({__COMM_MAX_DEV_ADDR__}+1)

/// \def INFO_UUID_LEN
/// \brief Length of the uuid
#define INFO_UUID_LEN {__INFO_UUID_LEN__}

/// \struct tag_InfoConfig
/// \brief Describes all project information available to #INFODev
typedef struct tag_InfoConfig {{
	uint8_t device_id;                  ///< Configured device ID
	const char* device_name;            ///< Configured device name.
    uint8_t uuid[INFO_UUID_LEN];
    const InfoDeviceDescriptor devices[INFO_DEVICE_ADDRESSES];
}} InfoConfig;

/// @}}
