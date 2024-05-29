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
 *   \brief High-resolution System Tick Counter header.
 *   \author Oleh Sharuda
 */

#pragma once

#include "fw.h"
#include <limits.h>
#include "stm32f10x_conf.h"

// Systick is configured to fire interrupt once per 65536 ticks
#define SYSTICK_PRESCALLER (0)
#define SYSTICK_CNT_FREQ  (MCU_FREQUENCY/(SYSTICK_PRESCALLER + 2))
#define SYSTICK_PERIOD (0xFFFF)
#define SYSTICK_ONE_US_CNT (SYSTICK_CNT_FREQ / 1000000)
#define SYSTICK_ONE_MS_CNT (SYSTICK_CNT_FREQ / 1000)

#define delay_us(x) delay( ((uint64_t)(SYSTICK_ONE_US_CNT)) * ((uint64_t)(x)) )
#define delay_ms(x) delay( ((uint64_t)(SYSTICK_ONE_MS_CNT)) * ((uint64_t)(x)) )

void systick_init(void);
void systick_get(uint64_t* timestamp);
void delay(uint64_t duration);
