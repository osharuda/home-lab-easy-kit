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
 *   \brief SPWM (Software Pulse Width Modulation) device header.
 *   \author Oleh Sharuda
 */

#pragma once

/// \defgroup group_spwm_dev SPWMDev
/// \brief Multichannel software PWM support
/// @{
/// \page page_spwm_dev
/// \tableofcontents
///
/// \section sect_spwm_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

#ifdef SPWM_DEVICE_ENABLED

void spwm_init(void);

/// @}

#endif

