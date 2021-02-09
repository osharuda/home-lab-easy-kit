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
 *   \brief RTC (Real Time Clock) device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef RTC_DEVICE_ENABLED

/// \defgroup group_rtc_dev RTCDev
/// \brief Real Time Clock support
/// @{
/// \page page_rtc_dev
/// \tableofcontents
///
/// \section sect_rtc_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

#define RTC_MAGIC_NUM 0xABAC

void rtc_init();

/// @}

#endif