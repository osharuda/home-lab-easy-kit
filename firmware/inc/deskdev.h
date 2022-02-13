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
 *   \brief Interfacing desk device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef DESKDEV_DEVICE_ENABLED

/// \defgroup group_desk_dev DESKDev
/// \brief Simple set of four buttons and one encoder
/// @{
/// \page page_desk_dev
/// \tableofcontents
///
/// \section sect_desk_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

typedef struct tag_DeskDevButtonState
{
	volatile uint8_t	state;				// Current button state, true - pushed, false - released
} DeskDevButtonState;

typedef struct __attribute__ ((aligned)) tag_DeskDevEncoderState
{
    volatile uint64_t  last_ts;
	volatile uint8_t   last_ev;
	volatile uint8_t   ev_count;
} DeskDevEncoderState;

#define ENCODER_A				0
#define ENCODER_B				1

#define ENCODER_LAST_EV_REJECT_MS 1500
#define ENCODER_STALE_DATA_MS     1000000

void deskdev_init(void);

/// @}

#endif