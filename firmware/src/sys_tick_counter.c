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

#include "fw.h"
#if ENABLE_SYSTICK!=0

#include "timers.h"
#include "sys_tick_counter.h"



volatile uint64_t g_systick_irq_cnt __attribute__ ((aligned)) = 0;

MAKE_ISR(SYS_TICK_ISR) {
    if (SYS_TICK_PERIPH->SR & TIM_IT_Update) {
        g_systick_irq_cnt++;
        TIM_ClearITPendingBit(SYS_TICK_PERIPH, TIM_IT_Update);
    }
}

void systick_init(void) {
    IS_SIZE_ALIGNED(&g_systick_irq_cnt);
    struct TimerData timer_data;
    timer_data.timer = SYS_TICK_PERIPH;
    timer_data.irqn = SYS_TICK_IRQ;
    timer_init(&timer_data, IRQ_PRIORITY_SYSTICK, TIM_CounterMode_Up, TIM_CKD_DIV1);

    periodic_timer_start_and_fire(&timer_data, SYSTICK_PRESCALLER, SYSTICK_PERIOD);
}
#endif // of ENABLE_SYSTICK