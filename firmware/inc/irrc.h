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
 *   \brief IRRC (Infra Red Remote Control) device header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef IRRC_DEVICE_ENABLED

/// \defgroup group_irrc_dev IRRCDev
/// \brief NEC standard IR remote control support
/// @{
/// \page page_irrc_dev
/// \tableofcontents
///
/// \section sect_irrc_dev_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

#define IRRC_NEC_ALL_SEQUANCE_MIN 66000
#define IRRC_NEC_ALL_SEQUANCE_MAX 68000

#define IRRC_NEC_REPEAT_MAX		  150000

// Lead pulse 9ms
#define IRRC_NEC_LEAD_PULSE_MIN	8800
#define IRRC_NEC_LEAD_PULSE_MAX	9200

// Lead space 4.5ms
#define IRRC_NEC_LEAD_SPACE_MIN	4300
#define IRRC_NEC_LEAD_SPACE_MAX	4700

// Lead repeat space 2.25ms
#define IRRC_NEC_LEAD_RPT_SPACE_MIN	2050
#define IRRC_NEC_LEAD_RPT_SPACE_MAX	2450

// 0 and 1 definition
#define IRRC_NEC_1_MIN			900
#define IRRC_NEC_1_MAX			1400

#define IRRC_NEC_0_MIN			2000
#define IRRC_NEC_0_MAX			2500

#define IRRC_NEC_MAX_BIT		31

#define IRRC_NEC_NO_SIGNAL		0
#define IRRC_NEC_LEAD_PULSE		1
#define IRRC_NEC_DATA			2


typedef struct tag_IRRCPrivData{
    uint64_t signal_start;
    uint64_t last_bit_start;
 	CircBuffer circ;
 	uint32_t data;
	uint8_t	state;
	uint8_t bitcounter;
	uint8_t last_actual;
	uint8_t last_ir_address;
	uint8_t last_ir_command;
	uint8_t buffer[IRRC_BUF_LEN];
} IRRCPrivData, *PIRRCPrivData;

void irrc_init();
void irrc_command(uint8_t cmd_byte, uint8_t* data, uint16_t length);
void irrc_readdone(uint8_t device_id, uint16_t length);

/// @}

#endif
