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
 *   \brief SPWM (Software Pulse Width Modulation) device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef SPWM_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "spwm.h"
#include "spwm_conf.h"



volatile uint16_t g_current_pwm_index;
volatile uint8_t g_pwm_buffer[SPWM_BUFFER_SIZE];


volatile uint16_t  g_pwm_entries_count;
volatile SPWM_GPIO_DESCRIPTOR g_spwm_descriptor[] = SPWM_GPIO_DESCRIPTION;
volatile DeviceContext spwm_ctx __attribute__ ((aligned));

void spwm_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    UNUSED(cmd_byte);
    uint8_t status = 0;
    if (length>SPWM_BUFFER_SIZE) {
        status = COMM_STATUS_FAIL;
        goto done;
    }

    SET_FLAGS(SPWM_TIMER->CR1, TIM_CR1_UDIS);	// DISABLE UPDATE INTERRUPT GENERATION

    memcpy((void*)g_pwm_buffer, data, length);
    g_pwm_entries_count = length / PWM_ENTRY_SIZE(SPWM_PORT_COUNT);

    if (g_current_pwm_index>=g_pwm_entries_count) {
        g_current_pwm_index = 0;
    }

    CLEAR_FLAGS(SPWM_TIMER->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION

done:
    comm_done(status);
}

// possibly this function may be optimized by using 32-bit BSRR
void spwm_set_port(GPIO_TypeDef* port, uint16_t mask, uint16_t value) {
    uint32_t x = (value & mask) | ((~value & mask) << 16);
    port->BSRR = x;
}

void spwm_init_pins(void) {
    START_PIN_DECLARATION;
    for (int i=0; i<SPWM_PORT_COUNT; i++) {
        for (int p=0; p<16; p++) {
            uint16_t t = 1 << p;
            if (t & g_spwm_descriptor[i].mask) {
                GPIOMode_TypeDef mode = GPIO_Mode_Out_PP;
                if (t & g_spwm_descriptor[i].open_drain_bits) {
                    mode = GPIO_Mode_Out_OD;
                }
                DECLARE_PIN(g_spwm_descriptor[i].port, t, mode);
            }
        }

        spwm_set_port(g_spwm_descriptor[i].port, g_spwm_descriptor[i].mask, g_spwm_descriptor[i].def_vals);
    }
}

void spwm_init(void) {
    TIM_TimeBaseInitTypeDef timer;

    // setup variables into initial state
    g_current_pwm_index = 0;
    volatile PWM_ENTRY* pwm = GET_PWM_ENTRY_BY_INDEX(g_pwm_buffer, 0, PWM_ENTRY_SIZE(SPWM_PORT_COUNT));
    pwm->n_periods = 0xFFFF;
    g_pwm_entries_count = 1;
    for (int i=0; i<SPWM_PORT_COUNT; i++) {
        pwm->data[i] = g_spwm_descriptor[i].def_vals;
    }


    // setup pins into their default state
    spwm_init_pins();

    // Prepare timer
    timer.TIM_CounterMode       = TIM_CounterMode_Up;
    timer.TIM_Prescaler         = SPWM_PRESCALE_VALUE;
    timer.TIM_Period            = pwm->n_periods;
    timer.TIM_ClockDivision     = TIM_CKD_DIV1;
    timer.TIM_RepetitionCounter = 0;


    // Setup interrupt and enable timer
    NVIC_SetPriority(SPWM_TIM_IRQn, IRQ_PRIORITY_SPWM);
    NVIC_EnableIRQ(SPWM_TIM_IRQn);
    TIM_TimeBaseInit(SPWM_TIMER, &timer);
    TIM_ITConfig(SPWM_TIMER, TIM_IT_Update, ENABLE);
    TIM_Cmd(SPWM_TIMER, ENABLE);

    // Register device
    memset((void*)&spwm_ctx, 0, sizeof(spwm_ctx));
    spwm_ctx.device_id = SPWM_ADDR;
    spwm_ctx.on_command = spwm_dev_execute;
    comm_register_device(&spwm_ctx);
}

MAKE_ISR(SPWM_TIM_IRQ_HANDLER)
{
    if (TIM_GetITStatus(SPWM_TIMER, TIM_IT_Update) == RESET) {
        return;
    }

    g_current_pwm_index++;
    if (g_current_pwm_index==g_pwm_entries_count) {
        g_current_pwm_index=0;
    }

    volatile PWM_ENTRY* pwm = GET_PWM_ENTRY_BY_INDEX(g_pwm_buffer, g_current_pwm_index, PWM_ENTRY_SIZE(SPWM_PORT_COUNT)); // g_pwm_data+g_current_pwm_index;

    for (int i=0; i<SPWM_PORT_COUNT; i++) {
        spwm_set_port(g_spwm_descriptor[i].port, g_spwm_descriptor[i].mask, pwm->data[i]);
    }

    SET_FLAGS(SPWM_TIMER->CR1, TIM_CR1_UDIS);	// DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    SPWM_TIMER->ARR = pwm->n_periods;
    SPWM_TIMER->PSC = SPWM_PRESCALE_VALUE;
    SPWM_TIMER->CNT=0;
    SPWM_TIMER->EGR = TIM_PSCReloadMode_Immediate;
    CLEAR_FLAGS(SPWM_TIMER->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION

    TIM_ClearITPendingBit(SPWM_TIMER, TIM_IT_Update);
}

#endif