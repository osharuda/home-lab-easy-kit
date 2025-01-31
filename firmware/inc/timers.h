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
 *   \brief Common functionality for timers.
 *   \author Oleh Sharuda
 */

#pragma once

#include "utools.h"

struct TimerData {
    TIM_TypeDef* timer;               /// Timer
    IRQn_Type irqn;                   /// IRQn
    struct {
        volatile uint16_t *rcr_register;  /// Repetition Counter Register
        volatile uint32_t* icpr_register;  /// Clear pending bit ICPR register
        volatile uint32_t *iser_register; /// Enable IRQ register
        volatile uint32_t *icer_register; /// Disable IRQ register
        volatile uint8_t* ip_register;    /// Interrupt priority register

        uint32_t iser_icer_value;         /// Enable/Disable IRQ register
        uint32_t icpr_value;              /// Clear pending bit value
        uint16_t rcr_value;               /// Repetition counter value (if applicable)
        uint16_t cr1_clear;               /// Bit's to be cleared in timer's CR1 register
        uint16_t cr1_set;                 /// Bit's to be set in timer's CR1 register
        uint8_t  ip_value;                 /// Interrupt priority value
        // ip_value;
    } preinit_data;
};

/// \brief Initialize timer data structure
/// \param timer_data - Preinitialized timer data structure.
///        The following fields must be initialized before the call: timer, irqn.
/// \param priority - timer IRQ priority
/// \param counter_mode - timer counter mode (one of the TIM_CounterMode_Up, TIM_CounterMode_Down,
///        TIM_CounterMode_CenterAligned1, TIM_CounterMode_CenterAligned2, TIM_CounterMode_CenterAligned3 values)
/// \param clock_div - timer clock divider (one of the TIM_CKD_DIV1, TIM_CKD_DIV2, TIM_CKD_DIV4 values)
/// \note timer and irqn members of the TimerData structure must be initialized before the call.
/// \param irqn - timer IRQn
void timer_init(struct TimerData* timer_data,
                uint32_t priority,
                uint16_t counter_mode,
                uint16_t clock_div);

/// \brief Initialize TIMER to generate repeatitive update event using prescaller and period values which doesn't change.
/// \param timer_data - Preinitialized timer data structure.
/// \param prescaler - prescaller value.
/// \param period - period value.
void periodic_timer_start_and_fire(struct TimerData* timer_data, uint16_t prescaler, uint16_t period);

/// \brief Initialize TIMER to generate repeatitive update event using prescaller and period values.
/// \param timer_data - Preinitialized timer data structure.
/// \param prescaler - prescaller value.
/// \param period - period value.
void periodic_timer_start(struct TimerData* timer_data, uint16_t prescaller, uint16_t period);


/// \brief Initialize dynamic TIMER to generate sequence of update events with programmed and changing periods of time.
/// \param timer_data - Preinitialized timer data structure.
/// \param prescaler - prescaller value (for the first update event).
/// \param period - period value.
/// \param next_prescaler - next prescaller value, requied here to properly intialize second update event.
void dynamic_timer_start(   struct TimerData* timer_data,
                            uint16_t prescaller,
                            uint16_t period,
                            uint16_t next_prescaler);

/// \brief Reinitializes timer with new parameters and reset counter register.
/// \param timer_data - Preinitialized timer data structure.
/// \param prescaler - prescaler value to be applied for the next update event.
/// \param next_prescaler - prescaler value to be applied after next update event is generated.
/// \param period - period value.
/// \details Timer should be previously initialized, this function just changes when this timer will be triggered next time.
/// \details Also, this function may be called in interrupt context. It is designed to be called from very same timer interrupt handler.
/// \note Important note! Due to timer peripherals implementation, next_prescaller value will be used for the period
///       following after the next event. (Prescaller ratio is updated AFTER update event only). Therefore the prescaller
///       value for the following period should be passed in the previous call to the dynamic_timer_update() or dynamic_timer_start().\n\n
///       Extract from documentation:\n
///       "Bit 0 UG: Update generation
///        This bit can be set by software, it is automatically cleared by hardware.
///         0: No action
///         1: Reinitialize the counter and generates an update of the registers. Note that the prescaler
///         counter is cleared too (anyway the prescaler ratio is not affected). The counter is cleared if
///         the center-aligned mode is selected or if DIR=0 (upcounting), else it takes the auto-reload
///         value (TIMx_ARR) if DIR=1 (downcounting)."
/// \note Obviously this function must be called in corresponding update event interrupt handler context. Therefor it has
///       specific check for this (debug only).
void dynamic_timer_update(struct TimerData* timer_data, uint16_t prescaler, uint16_t period, uint16_t next_prescaler);


/// \brief Disables both dynamic and periodic timers.
/// \param timer_data - Preinitialized timer data structure.
void timer_disable(struct TimerData* timer_data);

/// \brief Returns non-zero if active timer event is update event (TIM_IT_Update)
/// \param preinit_cache - Preinitialized cache with timer specific values.
#define TIMER_IS_UPDATE_EV(preinit_cache)  ((preinit_cache)->timer->SR & (preinit_cache)->timer->DIER & TIM_IT_Update)

/// \brief Returns non-zero if active timer event is update event (TIM_IT_Update)
/// \param preinit_cache - Preinitialized cache with timer specific values.
#define TIMER_CLEAR_IT_PENDING_EV(preinit_cache) (preinit_cache)->timer->SR = (uint16_t)~TIM_IT_Update;

/// \brief This macro remembers timer NVIC IRQ state (enabled or disabled). Use returned value to disable/recover IRQ.
/// \param preinit_cache - Preinitialized cache with timer specific values.
#define TIMER_NVIC_IRQ_STATE(preinit_cache)            ((*(preinit_cache)->preinit_data.iser_register) & ((preinit_cache)->preinit_data.iser_icer_value))

/// \brief This macro allows to disable IRQ.
/// \param preinit_cache - Preinitialized cache with timer specific values.
/// \param state - IRQn state returned by NVIC_IRQ_STATE macro
#define TIMER_NVIC_DISABLE_IRQ(preinit_cache, state)   *(preinit_cache)->preinit_data.icer_register = (state)

/// \brief This macro recovers state of IRQ.
/// \param preinit_cache - Preinitialized cache with timer specific values.
/// \param state - IRQn state returned by NVIC_IRQ_STATE macro
#define TIMER_NVIC_RESTORE_IRQ(preinit_cache, state)   *(preinit_cache)->preinit_data.iser_register = (state)


/// \brief This function is used to calculate optimal prescaller and period values to schedule timer.
/// \param us - input number of microseconds, must not be greater than #MCU_MAXIMUM_TIMER_US.
/// \param prescaller - pointer to the output value of prescaller.
/// \param period - pointer to the output value of period.
void timer_get_params(uint32_t us, volatile uint16_t* prescaller, volatile uint16_t* period);

/// @}
