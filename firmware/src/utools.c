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

#include "utools.h"

#ifdef DISABLE_NOT_TESTABLE_CODE
#else



/// \addtogroup group_utools
/// @{

GPIO_TypeDef g_null_port;

/// \brief Dummy 32-bit register which may be referenced to avoid if statements when some device need to write peripheral
///        register, while another device of the same kind shouldn't update peripheral register. In this case, device stores
///        pointer to required register. The first device initializes it to actual peripheral register pointer, the last
///        device initializes it to this dummy register.
uint32_t g_dummy_reg32;

/// \brief Dummy 16-bit register which may be referenced to avoid if statements when some device need to write peripheral
///        register, while another device of the same kind shouldn't update peripheral register. In this case, device stores
///        pointer to required register. The first device initializes it to actual peripheral register pointer, the last
///        device initializes it to this dummy register.
uint16_t g_dummy_reg16;



#ifndef NDEBUG
volatile uint8_t g_irq_disabled = 0;
#endif

void debug_checks_init(void) {
#ifndef NDEBUG
    g_irq_disabled = 0;
#endif
}

volatile uint32_t g_no_var;
void delay_loop(uint32_t n) {
    for (uint32_t i=0; i<n; i++) {
        g_no_var = n;
    }
}

void timer_start_ex(TIM_TypeDef* timer, uint16_t prescaller, uint16_t period, IRQn_Type irqn, uint32_t priority, uint8_t force_first_call) {
    // Check timer is disabled
    assert_param(IS_TIM_ALL_PERIPH(timer));
    assert_param(IS_CLEARED(timer->CR1, TIM_CR1_CEN));

    timer->DIER = TIM_IT_Update;                  // Setup interrupt
    NVIC_SetPriority(irqn, priority);
    NVIC_DisableIRQ(irqn);                        // Disable IRQ

    timer->CR1 = TIM_CR1_CEN;                     // Enable timer
    timer->PSC = prescaller;                      // Set prescaller
    timer->ARR = period;                          // Set period
    timer->EGR = TIM_PSCReloadMode_Immediate;     // Generate update event (IRQ will not be fired because it is disabled via NVIC (see above)

    if (force_first_call==0) {
        timer->SR = 0;                            // Clear status register
        NVIC_ClearPendingIRQ(irqn);               // Clear pending interrupt (to avoid immediate IRQ handler call)
    }

    // Enable IRQ
    NVIC_EnableIRQ(irqn);
}

void timer_disable_ex(TIM_TypeDef* timer) {
    timer->CR1 = 0;
}


void timer_start(TIM_TypeDef* timer, uint16_t prescaller, uint16_t period, IRQn_Type irqn, uint32_t priority) {
    TIM_TimeBaseInitTypeDef init_struct;
    TIM_Cmd(timer, DISABLE);

    // Prepare timer
    init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    init_struct.TIM_Prescaler         = prescaller;
    init_struct.TIM_Period            = period;
    init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    init_struct.TIM_RepetitionCounter = 0;

    // Setup interrupt and enable timer
    NVIC_SetPriority(irqn, priority);
    NVIC_EnableIRQ(irqn);
    TIM_TimeBaseInit(timer, &init_struct);
    TIM_Cmd(timer, ENABLE);
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
}

void timer_start_periodic(TIM_TypeDef* timer, uint16_t prescaller, uint16_t period, IRQn_Type irqn, uint32_t priority) {
    TIM_TimeBaseInitTypeDef init_struct;
    TIM_Cmd(timer, DISABLE);

    // Prepare timer
    init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    init_struct.TIM_Prescaler         = prescaller;
    init_struct.TIM_Period            = period;
    init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    init_struct.TIM_RepetitionCounter = 0;


    // Setup interrupt and enable timer
    NVIC_SetPriority(irqn, priority);
    NVIC_EnableIRQ(irqn);
    TIM_TimeBaseInit(timer, &init_struct);
    TIM_ARRPreloadConfig(timer, ENABLE);
    TIM_Cmd(timer, ENABLE);
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
}


void timer_start_periodic_ex(TIM_TypeDef* timer, uint16_t prescaler, uint16_t period, IRQn_Type irqn, uint32_t priority, uint8_t force_first_call) {
    TIM_TimeBaseInitTypeDef init_struct;
    TIM_Cmd(timer, DISABLE);

    // Prepare timer
    init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    init_struct.TIM_Prescaler         = prescaler;
    init_struct.TIM_Period            = period;
    init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    init_struct.TIM_RepetitionCounter = 0;

    // Setup interrupt and enable timer
    NVIC_SetPriority(irqn, priority);
    TIM_TimeBaseInit(timer, &init_struct);
    TIM_ARRPreloadConfig(timer, ENABLE);
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);

    if (force_first_call==0) {
        timer->SR = 0;                            // Clear status register
        NVIC_ClearPendingIRQ(irqn);               // Clear pending interrupt (to avoid immediate IRQ handler call)
    }

    NVIC_EnableIRQ(irqn);
    TIM_Cmd(timer, ENABLE);
}



void timer_start_us(TIM_TypeDef* timer, uint32_t us, IRQn_Type irqn, uint32_t priority) {
    TIM_TimeBaseInitTypeDef init_struct;
    TIM_Cmd(timer, DISABLE);

    // Prepare timer
    init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    timer_get_params(us, &(init_struct.TIM_Prescaler), &(init_struct.TIM_Period));
    init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    init_struct.TIM_RepetitionCounter = 0;

    // Setup interrupt and enable timer
    NVIC_SetPriority(irqn, priority);
    NVIC_EnableIRQ(irqn);
    TIM_TimeBaseInit(timer, &init_struct);
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
    TIM_Cmd(timer, ENABLE);
}

void timer_reschedule(TIM_TypeDef* timer, uint16_t prescaller, uint16_t period) {
    SET_FLAGS(timer->CR1, TIM_CR1_UDIS);	// DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    timer->PSC = prescaller;
    timer->ARR = period;
    timer->CNT=0;
    timer->EGR = TIM_PSCReloadMode_Immediate;
    CLEAR_FLAGS(timer->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION
}

void timer_reschedule_us(TIM_TypeDef* timer, uint32_t us) {
    SET_FLAGS(timer->CR1, TIM_CR1_UDIS);	// DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    timer_get_params(us, &(timer->PSC), &(timer->ARR));
    timer->CNT=0;
    timer->EGR = TIM_PSCReloadMode_Immediate;
    CLEAR_FLAGS(timer->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION
}

void timer_disable(TIM_TypeDef* timer, IRQn_Type irqn) {
    assert_param(IS_TIM_ALL_PERIPH(timer));
    timer->DIER &= (uint16_t)~TIM_IT_Update;
    timer->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
    timer->SR = (uint16_t)~TIM_IT_Update;
    NVIC_DisableIRQ(irqn);
}

#if EMERGENCY_DEBUG_TOOLS!=0

volatile uint32_t counted_break_counter = 0;
volatile uint32_t hit = 0;
void counted_break(uint32_t cnt)
{
    counted_break_counter++;
    if (counted_break_counter>=cnt) {
        hit++;
        counted_break_counter = 0;
    }
}

 void enable_debug_pins(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    START_PIN_DECLARATION;
    DECLARE_PIN(GPIOC, 			GPIO_Pin_13, 			GPIO_Mode_Out_PP);
    DECLARE_PIN(GPIOC, 			GPIO_Pin_14, 			GPIO_Mode_Out_PP);
    DECLARE_PIN(GPIOC, 			GPIO_Pin_15, 			GPIO_Mode_Out_PP);

    clear_debug_pin_0();
	clear_debug_pin_1();
	clear_debug_pin_2();
}

#endif

void fail_assert(uint8_t* src, uint32_t line) {
	UNUSED(src);
	UNUSED(line);
    while (1) {
        // place to stuck if assert_param() fails
    };
}

#endif // DISABLE_NOT_TESTABLE_CODE

void timer_get_params(uint32_t us, volatile uint16_t* prescaller, volatile uint16_t* period) {
    uint32_t psc, per;
    uint64_t k;

    if (us > MCU_MAXIMUM_TIMER_US) {
        assert_param(0); // This function works incorrectly when higher value is passed
        *prescaller = USHRT_MAX;
        *period = USHRT_MAX;
        return;
    }

    k = us * MCU_FREQUENCY_MHZ;

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
