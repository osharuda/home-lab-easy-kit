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
#include "utools.h"

#if ENABLE_SYSTICK!=0

extern volatile uint64_t g_systick_irq_cnt __attribute__ ((aligned));

// Systick is configured to fire interrupt once per 65536 ticks
#define SYSTICK_PRESCALLER (0)
#define SYSTICK_CNT_FREQ  (MCU_FREQUENCY/(SYSTICK_PRESCALLER + 1))
#define SYSTICK_PERIOD (0xFFFF)
#define SYSTICK_ONE_US_CNT (SYSTICK_CNT_FREQ / 1000000)
#define SYSTICK_TO_uS(x) ((x)/SYSTICK_ONE_US_CNT)
#define SYSTICK_ONE_MS_CNT (SYSTICK_CNT_FREQ / 1000)
#define SYSTICK_TO_mS(x) ((x)/SYSTICK_ONE_MS_CNT)

#define delay_us(x) delay( ((uint64_t)(SYSTICK_ONE_US_CNT)) * ((uint64_t)(x)) )
#define delay_ms(x) delay( ((uint64_t)(SYSTICK_ONE_MS_CNT)) * ((uint64_t)(x)) )


void systick_init(void);

__attribute__((always_inline))
static inline void systick_get(volatile uint64_t* timestamp) {
    uint16_t dr;
    uint32_t irq_cnt_1;
    uint32_t irq_cnt_2;
    uint64_t tmp;

    do {
        set_debug_pin_2();
        irq_cnt_1 = g_systick_irq_cnt;
        dr = SYS_TICK_PERIPH->CNT;
        irq_cnt_2 = g_systick_irq_cnt;
        clear_debug_pin_2();
    } while (irq_cnt_1 != irq_cnt_2);
    tmp = irq_cnt_1;
    tmp = (tmp << (sizeof(uint16_t)*CHAR_BIT)) + dr;
    *timestamp = tmp;
}

__attribute__((always_inline))
static inline uint64_t get_us_clock(void) {
        uint64_t res;
        systick_get(&res);
        return SYSTICK_TO_uS(res);
}

__attribute__((always_inline))
static inline uint64_t get_tick_diff_64(uint64_t ev_1, uint64_t ev_2) {
    if (ev_2 >= ev_1) {
        return ev_2 - ev_1;
    } else {
        return ULLONG_MAX - ev_1 + ev_2;
    }
}

__attribute__((always_inline))
static inline void delay(uint64_t duration) {
    uint64_t when, end;
    systick_get(&when);
    when += duration;
    do {
        systick_get(&end);
    } while (end < when);
}

#endif // of ENABLE_SYSTICK
