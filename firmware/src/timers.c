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
 *   \brief Multipurpose functions tools and macro definitions C source file
 *   \author Oleh Sharuda
 */

#include "timers.h"

void timer_init(struct TimerData* timer_data,
                uint32_t priority,
                uint16_t counter_mode,
                uint16_t clock_div) {
    TIM_TypeDef* timer = timer_data->timer;
    IRQn_Type irqn = timer_data->irqn;

    assert_param(IS_TIM_ALL_PERIPH(timer));

    // Clear pending bit register
    timer_data->preinit_data.icpr_register = NVIC->ICPR + ((uint32_t)(irqn) >> 5);
    timer_data->preinit_data.icpr_value = (1 << ((uint32_t)(irqn) & 0x1F));

    // Priority register and value
    timer_data->preinit_data.ip_register = NVIC->IP + (uint32_t)(irqn);
    timer_data->preinit_data.ip_value = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);

    // Enable (iser) /Disable (icer) registers and value
    uint32_t offset = ((uint32_t)(irqn) >> 5);
    timer_data->preinit_data.iser_register = NVIC->ISER + offset; // Enable IRQ register
    timer_data->preinit_data.icer_register = NVIC->ICER + offset; // Disable IRQ register
    timer_data->preinit_data.iser_icer_value = (1 << ((uint32_t)(irqn) & 0x1F));   // Disable/Enable IRQ value

    // Repetition counter register
    if ((timer == TIM1) || (timer == TIM8)|| (timer == TIM15)|| (timer == TIM16) || (timer == TIM17))
    {
        timer_data->preinit_data.rcr_register = &(timer->RCR);
    } else {
        timer_data->preinit_data.rcr_register = &g_dummy_reg16;
    }
    timer_data->preinit_data.rcr_value = 0;

    timer_data->preinit_data.cr1_clear = 0xffff;
    timer_data->preinit_data.cr1_set = TIM_CR1_ARPE;

    if((timer == TIM1) || (timer == TIM8)|| (timer == TIM2) || (timer == TIM3)||
       (timer == TIM4) || (timer == TIM5))
    {
        // Select the Counter Mode
        timer_data->preinit_data.cr1_clear &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS)));
        timer_data->preinit_data.cr1_set |= counter_mode;
    }

    if((timer != TIM6) && (timer != TIM7))
    {
        // Clock divider
        timer_data->preinit_data.cr1_clear &= (uint16_t)(~((uint16_t)TIM_CR1_CKD));
        timer_data->preinit_data.cr1_set |= clock_div;
    }
}

inline static void timer_data_init(struct TimerData* timer_data, uint16_t prescaller, uint16_t period) {
    // Disable timer
    timer_data->timer->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));

    uint16_t cr1 = timer_data->timer->CR1;
    cr1 &= timer_data->preinit_data.cr1_clear;
    cr1 |= timer_data->preinit_data.cr1_set;
    timer_data->timer->CR1 = cr1;

    // Set the period and prescaler values
    timer_data->timer->ARR = period;
    timer_data->timer->PSC = prescaller;

    // Repetition value (if not applicable, pointer to dummy register is used)
    *timer_data->preinit_data.rcr_register = timer_data->preinit_data.rcr_value;

    // Generate update event to reload prescaler and repetition registers.
    timer_data->timer->EGR = TIM_PSCReloadMode_Immediate;

    // Enable update event
    timer_data->timer->DIER |= TIM_IT_Update;
}

inline static void timer_data_init_complete(struct TimerData* timer_data) {
    // Set irq priority
    *timer_data->preinit_data.ip_register = timer_data->preinit_data.ip_value;

    // Enable IRQ (NVIC)
    *timer_data->preinit_data.iser_register = timer_data->preinit_data.iser_icer_value;

    // Enable timer
    timer_data->timer->CR1 |= TIM_CR1_CEN;
}

inline static void timer_data_suppress_pending_irq(struct TimerData* timer_data) {
    timer_data->timer->SR = 0;

    // Clear prending IRQ (NVIC)
    *timer_data->preinit_data.icpr_register = timer_data->preinit_data.icpr_value;
}

void periodic_timer_start_and_fire(struct TimerData* timer_data, uint16_t prescaller, uint16_t period) {
    timer_data_init(timer_data, prescaller, period);
    timer_data_init_complete(timer_data);
}


void periodic_timer_start(struct TimerData* timer_data, uint16_t prescaller, uint16_t period) {
    timer_data_init(timer_data, prescaller, period);
    timer_data_suppress_pending_irq(timer_data);
    timer_data_init_complete(timer_data);
}

void dynamic_timer_start(struct TimerData* timer_data,
                         uint16_t prescaler,
                         uint16_t period,
                         uint16_t next_prescaler) {
    TIM_TypeDef* timer = timer_data->timer;
    // Check timer is disabled
    assert_param(IS_TIM_ALL_PERIPH(timer));
    assert_param(IS_CLEARED(timer->CR1, TIM_CR1_CEN));

    timer->DIER = TIM_IT_Update;              // Setup interrupt
    NVIC_DisableIRQ(timer_data->irqn);        // Disable IRQ

    timer->PSC = prescaler;                   // Set prescaler
    timer->ARR = period;                      // Set period
    timer->EGR = TIM_PSCReloadMode_Immediate; // Generate update event (IRQ will not be fired because it is disabled via NVIC (see above)

    timer_data_suppress_pending_irq(timer_data);
    timer_data_init_complete(timer_data);
    timer->PSC = next_prescaler;
}

void dynamic_timer_update(struct TimerData* timer_data, uint16_t prescaler, uint16_t period, uint16_t next_prescaler) {
    TIM_TypeDef* timer = timer_data->timer;
    assert_param(IN_INTERRUPT);
    SET_FLAGS(timer->CR1, TIM_CR1_UDIS);   // DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    timer->PSC = prescaler;
    timer->ARR = period;
    timer->CNT=0;
    timer->EGR = TIM_PSCReloadMode_Immediate;
    CLEAR_FLAGS(timer->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION
    timer->PSC = next_prescaler;
}

void timer_disable(struct TimerData* timer_data) {
    // Clear pending bit
    timer_data_suppress_pending_irq(timer_data);

    // Disable interrupt (NVIC)
    *timer_data->preinit_data.icer_register = timer_data->preinit_data.iser_icer_value;

    // Disable timer
    timer_data->timer->CR1 = 0;
}

void timer_get_params(uint32_t us, volatile uint16_t* prescaller, volatile uint16_t* period) {
    uint32_t psc, per;
    uint64_t k;

    if (us > MCU_MAXIMUM_TIMER_US) {
        assert_param(0); // This function works incorrectly when higher value is passed
        *prescaller = USHRT_MAX;
        *period = USHRT_MAX;
        return;
    }

    k = (uint64_t)us * MCU_FREQUENCY_MHZ;

    psc = k >> 16;
    if (psc > 0) {
        per = k / (psc+1);
    } else {
        per = k;
    }

    if (per>USHRT_MAX) {
        per = USHRT_MAX;
    } else if (per>0) {
        per--;
    }

    *prescaller = (uint16_t)psc;
    *period = (uint16_t)per;
}

/// @}
