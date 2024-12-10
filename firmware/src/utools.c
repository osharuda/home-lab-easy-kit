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


/// @}
