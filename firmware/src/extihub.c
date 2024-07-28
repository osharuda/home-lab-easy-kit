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
 *   \brief EXTI hub C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef EXTIHUB_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "extihub.h"
#include "sys_tick_counter.h"
#include "exti_conf.h"

/// \addtogroup group_exti_hub_group
/// @{

/// \brief Constant array that allows conversion from EXTI line number to EXTI interrupt number.
const IRQn_Type g_extihub_line_to_irqn[] = EXTIHUB_LINE_TO_IRQN;

/// \brief Array that represents EXTI handlers registered with EXTI HUB.
ExtiHandlerDescr g_extihub_handlers[EXTIHUB_LINE_COUNT];

/// \brief Common EXTI HUB IRQ handler.
/// \details This function is called by EXTI interrupt handlers which are used by firmware. This function calls
///          EXTI callbacks registered in EXTI HUB by #exti_register_callback().
void EXTIHUB_COMMON_IRQ_HANDLER(void) {
    uint64_t timestamp;
    set_debug_pin_1();
    systick_get(&timestamp);
    clear_debug_pin_1();
	uint32_t events = EXTI->PR;
	EXTI->PR |= events;

	for (uint8_t exti_line = 0; exti_line<EXTIHUB_LINE_COUNT; exti_line++) {
        PExtiHandlerDescr hndlr;
	    if ((events & (1<<exti_line))==0)
	        continue;

        hndlr = g_extihub_handlers + exti_line;
        hndlr->exti_handler(timestamp, hndlr->context);
	}
}

EXTIHUB_IRQ_HANDLERS

void exti_hub_init(void) {
    memset(&g_extihub_handlers, 0, sizeof(g_extihub_handlers));
}

uint8_t exti_register_callback(GPIO_TypeDef * port,
                            uint8_t pin_num,
                            GPIOMode_TypeDef gpio_mode,
                            uint16_t exti_cr,
                            uint8_t raise,
                            uint8_t fall,
                            PFN_EXTIHUB_CALLBACK fn,
                            volatile void* ctx,
                            uint8_t masked) {
    START_PIN_DECLARATION;
    assert_param(gpio_mode == GPIO_Mode_IN_FLOATING || gpio_mode == GPIO_Mode_IPD || gpio_mode == GPIO_Mode_IPU);
    DECLARE_PIN(port, 	1<<pin_num, 		gpio_mode);

    uint8_t pin_val = GPIO_ReadInputDataBit(port, 1<<pin_num);

    DEFINE_EXIT_PIN(exti_cr, pin_num, raise, fall, masked);

    PExtiHandlerDescr hndlr = g_extihub_handlers + pin_num;
    hndlr->exti_handler = fn;
    hndlr->context = ctx;

    NVIC_SetPriority(g_extihub_line_to_irqn[pin_num], IRQ_PRIORITY_EXTI);
    NVIC_EnableIRQ(g_extihub_line_to_irqn[pin_num]);
    return pin_val;
}

uint8_t exti_mask_callback(GPIO_TypeDef * port,
                           uint8_t pin_num){
    MASK_EXTI_PIN(pin_num);
    return GPIO_ReadInputDataBit(port, 1<<pin_num);
}

uint8_t exti_unmask_callback(GPIO_TypeDef * port,
                             uint8_t pin_num){
    UNMASK_EXTI_PIN(pin_num);
    return GPIO_ReadInputDataBit(port, 1<<pin_num);
}

/// @}

#endif

