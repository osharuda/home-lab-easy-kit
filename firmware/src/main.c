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
 *   \brief Main C source file for main() entry point.
 *   \author Oleh Sharuda
 */

#include <stm32f10x_flash.h>
#include "fw.h"
#include "utools.h"
#include "i2c_bus.h"
#include "info_dev.h"
#include "deskdev.h"
#include "rtc.h"
#include "i2c_proto.h"
#include "gpio_dev.h"
#include "lcd_1602A.h"
#include "uart_proxy.h"
#include "irrc.h"
#include "spwm.h"
#include "adcdev.h"
#include "step_motor.h"
#include "extihub.h"
#include "can.h"
#include "spiproxy.h"
#include "ad9850dev.h"
#include "spidac.h"
// INCLUDE_HEADER

void init_NVIC() {
    // Configure NVIC Priorities
    NVIC_PriorityGroupConfig(IRQ_NVIC_PRIORITY_GROUP);
}

/// \def USE_FAULT_HANDLERS
/// \brief Set to 1 in order to define default hard handlers that can be used for debugging purposes. Otherwise define to 0.
#define USE_FAULT_HANDLERS 0
#if USE_FAULT_HANDLERS
void HardFault_Handler(void)
{
    assert_param(0);
}

void BusFault_Handler(void){
    assert_param(0);
}

void UsageFault_Handler(void) {
    assert_param(0);
}
#endif

void RCCInit()
{
	RCC_HSEConfig(RCC_HSE_ON);
    ErrorStatus err = RCC_WaitForHSEStartUp();
    if (err!=SUCCESS) {
        assert_param(0);
        while (1) {}    // it is critical situation: stuck here in all build configurations
    }

    // Enable Prefetch Buffer
    FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);

    // Flash 2 wait state
    FLASH_SetLatency( FLASH_Latency_2);

	// Configure AHB, APB1 and APB2 clock
    RCC_HCLKConfig( RCC_SYSCLK_Div1);
    RCC_PCLK2Config( RCC_HCLK_Div1);
    RCC_PCLK1Config( RCC_HCLK_Div2);


    // Configure PLL
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);

	// Wait PLL is ready
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

    // Switch SYS
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08)  {}
}

int main(void)
{
    // Initialize CLOCK frequency (HSE, on board oscillator 8MHz)
    RCCInit();

    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA;
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA;
    DBGMCU->CR |= DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT | DBGMCU_CR_DBG_TIM4_STOP;


	// This macro enables clock on required peripheral devices. It is auto-generated and defined in hal.h
    ENABLE_PERIPH_CLOCK

    debug_checks_init();
    init_NVIC();

#if ENABLE_SYSTICK!=0
    systick_init();
#endif

    i2c_bus_init();

#ifdef EXTIHUB_DEVICE_ENABLED
    exti_hub_init();
#endif

#ifdef INFO_DEVICE_ENABLED
    info_dev_init();
#endif

#ifdef LCD1602a_DEVICE_ENABLED
    lcd_init();

    lcd_string((uint8_t*)LCD1602a_WELCOME_1, LCD1602a_LINE_1);
    lcd_string((uint8_t*)LCD1602a_WELCOME_2, LCD1602a_LINE_2);
#endif

#ifdef DESKDEV_DEVICE_ENABLED
    deskdev_init();
#endif

#ifdef RTC_DEVICE_ENABLED
    rtc_init();
#endif

#ifdef UART_PROXY_DEVICE_ENABLED
    uart_proxy_init();
#endif

#ifdef IRRC_DEVICE_ENABLED    
    irrc_init();
#endif    

#ifdef GPIODEV_DEVICE_ENABLED    
    gpio_init();
#endif

#ifdef SPWM_DEVICE_ENABLED
    spwm_init();
#endif

#ifdef ADCDEV_DEVICE_ENABLED
    init_adc();
#endif

#ifdef STEP_MOTOR_DEVICE_ENABLED
    step_motor_init();
#endif

#ifdef CAN_DEVICE_ENABLED
    can_init();
#endif

#ifdef SPIPROXY_DEVICE_ENABLED
    spiproxy_init();
#endif

#ifdef AD9850DEV_DEVICE_ENABLED
    ad9850dev_init();
#endif

#ifdef SPIDAC_DEVICE_ENABLED
    spidac_init();
#endif
// ADD_DEVICE

    enable_debug_pins();

    while(1) {
    	i2c_check_command();

#if ENABLE_SYSTICK!=0
    	i2c_pool_devices();
#endif
    }

}
