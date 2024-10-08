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
 *   \brief Generated include header for firmware
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stm32f10x.h>

#define GPIO_DEFAULT_SPEED                  GPIO_Speed_50MHz

#define MCU_FREQUENCY_MHZ                   {__MCU_FREQUENCY_MHZ__}
#define MCU_FREQUENCY                       {__MCU_FREQUENCY__}
#define MCU_MAXIMUM_TIMER_US                {__MCU_MAXIMUM_TIMER_US__}

 //---------------------------------------------------------------
 // Configure interrupt preemption and sub priorities
 //---------------------------------------------------------------

 // Here should be at least two preemption levels to allow I2C EV interrupt to preempt other interrupts.
 // Therefore we use straightforward NVIC_PriorityGroup_4 configuration
 #define IRQ_NVIC_PRIORITY_GROUP (NVIC_PriorityGroup_4)

// Priority levels
#define IRQ_PRIORITY_I2C_EV                0
#define IRQ_PRIORITY_I2C_ER                0
#define IRQ_PRIORITY_SYSTICK               1
#define IRQ_PRIORITY_CRITICAL_SECTION      2

#define IRQ_PRIORITY_PACEMAKER_MAIN        3
#define IRQ_PRIORITY_PACEMAKER_INTERNAL    4

#define IRQ_PRIORITY_DMA                   3
#define IRQ_PRIORITY_DAC_TIMER             4

#define IRQ_PRIORITY_ADC_HI_PRIO           5
#define IRQ_PRIORITY_ADC_LO_PRIO           6

#define IRQ_PRIORITY_SPWM                  6

#define IRQ_PRIORITY_USART                 7
#define IRQ_PRIORITY_CAN                   7
#define IRQ_PRIORITY_SPI                   7
#define IRQ_PRIORITY_EXTI                  8
#define IRQ_PRIORITY_STEP_MOTOR_TIMER      9

#define MAIN_PRIORITY                      0xFF

#define ENABLE_SYSTICK {__ENABLE_SYSTICK__}


#define I2C_BUS_PERIPH                {__I2C_BUS_PERIPH__}
#define I2C_BUS_PINS_REMAP            {__I2C_BUS_PINS_REMAP__}
#define I2C_BUS_CLOCK_SPEED           {__I2C_BUS_CLOCK_SPEED__}

#define I2C_BUS_SDA_PORT              {__I2C_BUS_SDA_PORT__}
#define I2C_BUS_SDA_PIN               {__I2C_BUS_SDA_PIN__}
#define I2C_BUS_SDA_PIN_MASK          {__I2C_BUS_SDA_PIN_MASK__}

#define I2C_BUS_SCL_PORT              {__I2C_BUS_SCL_PORT__}
#define I2C_BUS_SCL_PIN               {__I2C_BUS_SCL_PIN__}
#define I2C_BUS_SCL_PIN_MASK          {__I2C_BUS_SCL_PIN_MASK__}

#define I2C_BUS_EV_ISR                {__I2C_BUS_EV_ISR__}
#define I2C_BUS_EV_IRQ                {__I2C_BUS_EV_IRQ__}

#define I2C_BUS_ER_ISR                {__I2C_BUS_ER_ISR__}
#define I2C_BUS_ER_IRQ                {__I2C_BUS_ER_IRQ__}

#define SYS_TICK_PERIPH               {__SYS_TICK_PERIPH__}
#define SYS_TICK_ISR                  {__SYS_TICK_ISR__}
#define SYS_TICK_IRQ                  {__SYS_TICK_IRQ__}

// LibHLEK error codes extracted from ekit_error.hpp. Are used to return device errors.
{__LIBHLEK_ERROR_DEFINES__}

#include "i2c_proto.h"

{__FW_FEATURE_DEFINES__}

{__FW_HEADERS__}

{__APB_CLOCK_ENABLE__}


