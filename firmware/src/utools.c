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
int g_assert_param_count = 0;
#else

#include "fw.h"

/// \addtogroup group_utools
/// @{

/// \brief This global variable is used by #delay_us() in order to provide required wait.
/// \warning Don't access to this variable, it's dedicated solely by #delay_us() use.
volatile uint32_t g_usTicks = 0;

/// \brief This global variable is used by #SysTick_Handler() in order to provide 64-bit timestamp.
/// \warning Do not access to this variable directly, use #get_us_clock() instead.
/// \note This is 64-bit variable and STM32 is 32-bit MCU. It means this variable can't be modified atomically.
///       Atomicity is achieved by disabling interrupts in functions that read it (#get_us_clock()) and by modifying it
///       in SysTick interrupt only.
volatile uint64_t g_usClock __attribute__ ((aligned)) = 0;

#ifndef NDEBUG
volatile uint8_t g_irq_disabled = 0;
#endif

uint64_t get_us_clock(void) {
    uint64_t res_clock;
    DISABLE_IRQ
        res_clock = g_usClock;
    ENABLE_IRQ
    return  res_clock;
}

/// \brief SysTick interrupt handler.
/// \note This handler is modifying #g_usClock and #g_usTicks variables.
void SysTick_Handler(void)
{
	g_usClock++;
    g_usTicks++;
}

void systick_init(void)
{
    IS_ALIGNED(&g_usClock, sizeof(uint64_t));
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000000);
	NVIC_SetPriority(SysTick_IRQn, IRQ_PRIORITY_SYSTICK);
}

void debug_checks_init(void) {
#ifndef NDEBUG
    g_irq_disabled = 0;
#endif
}

void delay_us(uint32_t us)
{
    DISABLE_IRQ
    g_usTicks = 0;
    ENABLE_IRQ

	while (g_usTicks < us);
}

uint64_t get_tick_diff_64(uint64_t ev_1, uint64_t ev_2) {
    if (ev_2 >= ev_1) {
        return ev_2 - ev_1;
    } else {
        return ULLONG_MAX - ev_1 + ev_2;
    }
}

void delay_ms(uint32_t ms) {
	while (ms--)
		delay_us(1000);
}

void timer_schedule_update_ev(TIM_TypeDef* timer, uint32_t us, IRQn_Type irqn, uint32_t priority) {
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

void timer_reschedule_update_ev(TIM_TypeDef* timer, uint32_t us) {
    SET_FLAGS(timer->CR1, TIM_CR1_UDIS);	// DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    timer_get_params(us, &(timer->PSC), &(timer->ARR));
    timer->CNT=0;
    timer->EGR = TIM_PSCReloadMode_Immediate;
    CLEAR_FLAGS(timer->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION
}

void timer_disable(TIM_TypeDef* timer) {
    DISABLE_IRQ
    TIM_ITConfig(timer, TIM_IT_Update, DISABLE);
    TIM_Cmd(timer, DISABLE);
    ENABLE_IRQ
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

void set_debug_pin_0(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

void clear_debug_pin_0(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
}

void set_debug_pin_1(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);
}

void clear_debug_pin_1(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);
}

void set_debug_pin_2(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_SET);
}

void clear_debug_pin_2(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_RESET);
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
