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
 *   \brief High-resolution System Tick Counter source.
 *   \author Oleh Sharuda
 */

#include "utools.h"
#include "fw.h"
#include "sys_tick_counter.h"

#if ENABLE_SYSTICK!=0

volatile uint64_t g_systick_irq_cnt __attribute__ ((aligned)) = 0;

MAKE_ISR(SYS_TICK_ISR) {
    g_systick_irq_cnt++;
    TIM_ClearITPendingBit(SYS_TICK_PERIPH, TIM_IT_Update);
}

void systick_init(void) {
    timer_start_periodic(SYS_TICK_PERIPH, SYSTICK_PRESCALLER, SYSTICK_PERIOD, SYS_TICK_IRQ, IRQ_PRIORITY_SYSTICK);
}
#endif // of ENABLE_SYSTICK