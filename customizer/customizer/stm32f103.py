#   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
#
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import json
import re
from keywords import *

from tools import *
import operator

mcu_name = "stm32f103"

# Pin type declaration
PIN_IN_FLOAT        = "GPIO_Mode_IN_FLOATING"
PIN_IN_PULLED_UP    = "GPIO_Mode_IPU"
PIN_IN_PULLED_DOWN  = "GPIO_Mode_IPD"
PIN_OUT_OPEN_DRAIN  = "GPIO_Mode_Out_OD"
PIN_OUT_PUSH_PULL   = "GPIO_Mode_Out_PP"

mcu_gpio_pin_types        = {PIN_IN_FLOAT, PIN_IN_PULLED_UP, PIN_IN_PULLED_DOWN, PIN_OUT_OPEN_DRAIN, PIN_OUT_PUSH_PULL}
mcu_gpio_input_pin_types  = {PIN_IN_FLOAT, PIN_IN_PULLED_UP, PIN_IN_PULLED_DOWN}
mcu_gpio_output_pin_types = {PIN_OUT_OPEN_DRAIN, PIN_OUT_PUSH_PULL}


AHB_BUS = {"RCC_AHBPeriph_DMA1", "RCC_AHBPeriph_DMA2", "RCC_AHBPeriph_SRAM", "RCC_AHBPeriph_FLITF", "RCC_AHBPeriph_CRC",
           "RCC_AHBPeriph_FSMC", "RCC_AHBPeriph_SDIO"}

APB1_BUS = {"RCC_APB1Periph_TIM2", "RCC_APB1Periph_TIM3", "RCC_APB1Periph_TIM4", "RCC_APB1Periph_TIM5",
            "RCC_APB1Periph_TIM6", "RCC_APB1Periph_TIM7", "RCC_APB1Periph_TIM12", "RCC_APB1Periph_TIM13",
            "RCC_APB1Periph_TIM14", "RCC_APB1Periph_WWDG", "RCC_APB1Periph_SPI2", "RCC_APB1Periph_SPI3",
            "RCC_APB1Periph_USART2", "RCC_APB1Periph_USART3", "RCC_APB1Periph_UART4", "RCC_APB1Periph_UART5",
            "RCC_APB1Periph_I2C1", "RCC_APB1Periph_I2C2", "RCC_APB1Periph_USB", "RCC_APB1Periph_CAN1",
            "RCC_APB1Periph_CAN2", "RCC_APB1Periph_BKP", "RCC_APB1Periph_PWR", "RCC_APB1Periph_DAC",
            "RCC_APB1Periph_CEC"}

APB2_BUS = {"RCC_APB2Periph_AFIO", "RCC_APB2Periph_GPIOA", "RCC_APB2Periph_GPIOB", "RCC_APB2Periph_GPIOC",
            "RCC_APB2Periph_GPIOD", "RCC_APB2Periph_GPIOE", "RCC_APB2Periph_GPIOF", "RCC_APB2Periph_GPIOG",
            "RCC_APB2Periph_ADC1", "RCC_APB2Periph_ADC2", "RCC_APB2Periph_TIM1", "RCC_APB2Periph_SPI1",
            "RCC_APB2Periph_TIM8", "RCC_APB2Periph_USART1", "RCC_APB2Periph_ADC3", "RCC_APB2Periph_TIM15",
            "RCC_APB2Periph_TIM16", "RCC_APB2Periph_TIM17", "RCC_APB2Periph_TIM9", "RCC_APB2Periph_TIM10",
            "RCC_APB2Periph_TIM11"}

FW_FEATURES = {"SYSTICK"}


def get_bus_frequency(bus: str) -> int:
    if bus in APB1_BUS:
        return 18000000
    elif bus in APB2_BUS:
        return system_clock
    elif bus in AHB_BUS:
        return system_clock
    else:
        raise RuntimeError(f'Unknown bus: "{bus}"')


GPIO_PORT_LEN = 16

mcu_available_resources = "{"

# General purpose pins (GPIO)
mcu_available_resources += f"""
    "PA_0" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA"}},
    "PA_1" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_2" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_3" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_4" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_5" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_6" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_7" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_8" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_9" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_10" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_11" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_12" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_13" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_14" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},
    "PA_15" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOA" }},

    "PB_0" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_1" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_2" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_3" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_4" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_5" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_6" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_7" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_8" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_9" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_10" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_11" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_12" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_13" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_14" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    "PB_15" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOB" }},
    """
mcu_available_resources += f"""
    "PC_13" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
    "PC_14" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
    "PC_15" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
    """

# The following GPIO is not available on Blue Pill and other STM32F103x devices in LQFP48 packages
# Uncomment the following lines if you need them
# mcu_available_resources += f"""
#    "PC_0" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_1" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_2" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_3" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_4" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_5" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_6" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_7" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_8" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_9" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_10" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_11" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    "PC_12" : {{ "type" : "{RT_GPIO}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_GPIOC" }},
#    """

# IRQ Handlers (some devices can't share it, therefore it is treated as a resource of MCU)
mcu_available_resources += f"""
     "WWDG_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}", "{KW_REQUIRES}" : {{}}}},
     "PVD_IRQHandler"                 : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TAMPER_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "RTC_IRQHandler"                 : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "FLASH_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "RCC_IRQHandler"                 : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "EXTI0_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "EXTI1_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "EXTI2_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "EXTI3_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "EXTI4_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "DMA1_Channel1_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel2_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel3_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel4_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel5_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel6_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA1_Channel7_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "ADC1_2_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USB_HP_CAN1_TX_IRQHandler"      : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USB_LP_CAN1_RX0_IRQHandler"     : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN1_RX1_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN1_SCE_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "EXTI9_5_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "TIM1_BRK_TIM9_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_UP_TIM10_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_TRG_COM_TIM11_IRQHandler"  : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_BRK_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_UP_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_TRG_COM_IRQHandler"        : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_CC_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM2_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM3_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM4_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "I2C1_EV_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "I2C1_ER_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "I2C2_EV_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "I2C2_ER_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "SPI1_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "SPI2_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USART1_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USART2_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USART3_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "EXTI15_10_IRQHandler"           : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB2Periph_AFIO"}},
     "RTCAlarm_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "USBWakeUp_IRQHandler"           : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_BRK_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_UP_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_TRG_COM_IRQHandler"        : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_BRK_TIM12_IRQHandler"      : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_UP_TIM13_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_TRG_COM_TIM14_IRQHandler"  : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM8_CC_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "ADC3_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "FSMC_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "SDIO_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM5_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "SPI3_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "UART4_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "UART5_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM6_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM7_IRQHandler"                : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel1_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel2_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel3_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel4_5_IRQHandler"     : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN1_TX_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN1_RX0_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "OTG_FS_WKUP_IRQHandler"         : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel4_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "DMA2_Channel5_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "ETH_IRQHandler"                 : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "ETH_WKUP_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN2_TX_IRQHandler"             : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN2_RX0_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN2_RX1_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CAN2_SCE_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "OTG_FS_IRQHandler"              : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_BRK_TIM15_IRQHandler"      : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_UP_TIM16_IRQHandler"       : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM1_TRG_COM_TIM17_IRQHandler"  : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "CEC_IRQHandler"                 : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM6_DAC_IRQHandler"            : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM12_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM13_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     "TIM14_IRQHandler"               : {{ "type" : "{RT_IRQ_HANDLER}" , "{KW_REQUIRES}" : {{}}}},
     """

# External interrupt lines
mcu_available_resources += f"""     
     "EXTI_Line0" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line1" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line2" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line3" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line4" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line5" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line6" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line7" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line8" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line9" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line10" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line11" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line12" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line13" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line14" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     "EXTI_Line15" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},
     """

# EXTI_Line16 is connected to PVD output, and therefore is not supported
# mcu_available_resources += f"""
#     "EXTI_Line16" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},"""
# EXTI_Line17 is connected to RTC Alarm event, and therefore is not supported
# mcu_available_resources += f"""
#     "EXTI_Line17" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},"""
# EXTI_Line18 is connected to USB Wakeup event, and therefore is not supported
# mcu_available_resources += f"""
#     "EXTI_Line18" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},"""
# EXTI_Line19 is connected to Ethernet Wakeup event, and therefore is not supported
# mcu_available_resources += f"""
#     "EXTI_Line19" : {{"type" : "{RT_EXTI_LINE}", "{KW_REQUIRES}" : {{}}}},"""


# Backup registers
mcu_available_resources += f"""
    "BKP_DR1" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR2" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR3" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR4" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR5" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR6" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR7" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR8" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR9" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR10" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR11" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR12" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR13" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR14" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR15" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR16" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR17" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR18" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR19" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR20" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR21" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR22" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR23" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR24" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR25" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR26" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR27" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR28" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR29" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR30" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR31" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR32" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR33" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR34" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR35" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR36" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR37" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR38" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR39" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR40" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR41" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
    "BKP_DR42" : {{"type" : "{RT_BACKUP_REG}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_BKP"}},
"""

# Timers
mcu_available_resources += f"""
    "TIM1": {{"type": "{RT_TIMER}", "subtype": "advanced", "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM1_UP_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB2Periph_TIM1"}},
    "TIM2": {{"type": "{RT_TIMER}", "subtype": "general",  "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM2_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM2"}},
    "TIM3": {{"type": "{RT_TIMER}", "subtype": "general",  "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM3_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM3"}},
    "TIM4": {{"type": "{RT_TIMER}", "subtype": "general",  "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM4_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM4"}},
    "TIM5": {{"type": "{RT_TIMER}", "subtype": "general",  "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM5_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM5"}},
    "TIM6": {{"type": "{RT_TIMER}", "subtype": "basic",    "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM6_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM6"}},
    "TIM7": {{"type": "{RT_TIMER}", "subtype": "basic",    "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM7_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM7"}},
    "TIM8": {{"type": "{RT_TIMER}", "subtype": "advanced", "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM8_UP_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB2Periph_TIM8"}},
    "TIM9": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM9"}},
    "TIM10": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM10"}},
    "TIM11": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM11"}},
    "TIM12": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM12_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM12"}},
    "TIM13": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM13_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM13"}},
    "TIM14": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{ "{KW_TIMER_IRQ_HANDLER}" : {{"{RT_IRQ_HANDLER}" : "TIM14_IRQHandler"}} }}, "{KW_BUS}": "RCC_APB1Periph_TIM14"}},
    "TIM15": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM15"}},
    "TIM16": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM16"}},
    "TIM17": {{"type": "{RT_TIMER}", "subtype": "lite", "features": ["16bit"],  "{KW_REQUIRES}": {{}}, "{KW_BUS}": "RCC_APB2Periph_TIM17"}},
"""
# Real Time Clock
mcu_available_resources += f"""
    "RTC" : {{"type" : "rtc", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_APB1Periph_PWR"}},
"""

# USART
mcu_available_resources += f"""
    "USART1" : {{"type" : "{RT_UART}", "{KW_REQUIRES}" : {{"{RT_IRQ_HANDLER}" : "USART1_IRQHandler", 
                                                "RX" : {{"{RT_GPIO}" : "PA_10"}}, 
                                                "TX" : {{"{RT_GPIO}" : "PA_9"}} }},
                "{KW_BUS}" : "RCC_APB2Periph_USART1"}},
                                                
    "USART2" : {{"type" : "{RT_UART}", "{KW_REQUIRES}" : {{"{RT_IRQ_HANDLER}" : "USART2_IRQHandler", 
                                                "RX" : {{"{RT_GPIO}" : "PA_3"}}, 
                                                "TX" : {{"{RT_GPIO}" : "PA_2" }} }},
                "{KW_BUS}" : "RCC_APB1Periph_USART2" }},
                                                
    "USART3" : {{"type" : "{RT_UART}", "{KW_REQUIRES}" : {{"{RT_IRQ_HANDLER}" : "USART3_IRQHandler", 
                                                "RX" : {{"{RT_GPIO}" : "PB_11"}}, 
                                                "TX" : {{"{RT_GPIO}" : "PB_10"}} }},
                "{KW_BUS}" : "RCC_APB1Periph_USART3" }},
"""

# I2C
mcu_available_resources += f"""
    "I2C1" : {{"type" : "{RT_I2C}", "{KW_REQUIRES}" : {{"{KW_EV_IRQ_HLR}" : {{"{RT_IRQ_HANDLER}" : "I2C1_EV_IRQHandler"}}, 
                                            "{KW_ER_IRQ_HLR}" : {{"{RT_IRQ_HANDLER}" : "I2C1_ER_IRQHandler"}}, 
                                            "{KW_SDA_LINE}" : {{"{RT_GPIO}" : "PB_7"}}, 
                                            "{KW_SCL_LINE}" : {{"{RT_GPIO}" : "PB_6"}}}},
                "{KW_BUS}" : "RCC_APB1Periph_I2C1", "{KW_CLOCK_SPEED}": ["100000", "400000"]}},
                                            
    "I2C2" : {{"type" : "{RT_I2C}", "{KW_REQUIRES}" : {{"{KW_EV_IRQ_HLR}" : {{"{RT_IRQ_HANDLER}" : "I2C2_EV_IRQHandler"}}, 
                                            "{KW_ER_IRQ_HLR}" : {{"{RT_IRQ_HANDLER}" : "I2C2_ER_IRQHandler"}},
                                            "{KW_SDA_LINE}" : {{"{RT_GPIO}" : "PB_11"}}, 
                                            "{KW_SCL_LINE}" : {{"{RT_GPIO}" : "PB_10"}}}},
                "{KW_BUS}" : "RCC_APB1Periph_I2C2", "{KW_CLOCK_SPEED}": ["100000", "400000"]}},
"""

# DMA controllers and channels
mcu_available_resources += f"""
    "DMA1" : {{ "type" : "{RT_DMA}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", 
    "{KW_CLOCK_SPEED}": ["100000", "400000"]}},
    """
# The DMA2 controller and its relative requests are available only in high-density and connectivity line devices
# mcu_available_resources += f"""
#    "DMA2" : {{ "type" : "{RT_DMA}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2"}},
#    """
mcu_available_resources += f"""
    "DMA1_Channel1" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel1_IRQHandler"}}}},
    "DMA1_Channel2" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel2_IRQHandler"}}}},
    "DMA1_Channel3" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel3_IRQHandler"}}}},
    "DMA1_Channel4" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel4_IRQHandler"}}}},
    "DMA1_Channel5" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel5_IRQHandler"}}}},
    "DMA1_Channel6" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel6_IRQHandler"}}}},
    "DMA1_Channel7" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA1", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA1_Channel7_IRQHandler"}}}},
    """

# The DMA2 controller and its relative requests are available only in high-density and connectivity line devices
# mcu_available_resources += f"""
#    "DMA2_Channel1" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA2_Channel1_IRQHandler"}}}},
#    "DMA2_Channel2" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA2_Channel2_IRQHandler"}}}},
#    "DMA2_Channel3" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA2_Channel3_IRQHandler"}}}},
#    "DMA2_Channel4" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA2_Channel4_IRQHandler"}}}},
#    "DMA2_Channel5" : {{ "type" : "{RT_DMA_CHANNEL}", "{KW_REQUIRES}" : {{}}, "{KW_BUS}" : "RCC_AHBPeriph_DMA2", "dma_handler" : {{"{RT_IRQ_HANDLER}" : "DMA2_Channel5_IRQHandler"}}}},
# """

# map resource/feature onto DMA channel
dma_request_map = {"ADC1": "DMA1_Channel1", "TIM2_CH3": "DMA1_Channel1", "TIM4_CH1": "DMA1_Channel1",
                   "SPI1_RX": "DMA1_Channel2", "USART3_TX": "DMA1_Channel2", "TIM1_CH1": "DMA1_Channel2",
                   "TIM2_UP": "DMA1_Channel2", "TIM3_CH3": "DMA1_Channel2",
                   "SPI1_TX": "DMA1_Channel3", "USART3_RX": "DMA1_Channel3", "TIM1_CH2": "DMA1_Channel3",
                   "TIM3_CH4": "DMA1_Channel3", "TIM3_UP": "DMA1_Channel3",
                   "SPI2_RX": "DMA1_Channel4", "I2S2_RX": "DMA1_Channel4", "USART1_TX": "DMA1_Channel4",
                   "I2C2_TX": "DMA1_Channel4", "TIM1_CH4": "DMA1_Channel4", "TIM1_TRIG": "DMA1_Channel4",
                   "TIM1_COM": "DMA1_Channel4", "TIM4_CH2": "DMA1_Channel4",
                   "SPI2_TX": "DMA1_Channel5", "I2S2_TX": "DMA1_Channel5", "USART1_RX": "DMA1_Channel5",
                   "I2C2_RX": "DMA1_Channel5", "TIM1_UP": "DMA1_Channel5", "TIM2_CH1": "DMA1_Channel5",
                   "TIM4_CH3": "DMA1_Channel5",
                   "USART2_RX": "DMA1_Channel6", "I2C1_TX": "DMA1_Channel6", "TIM1_CH3": "DMA1_Channel6",
                   "TIM3_CH1": "DMA1_Channel6", "TIM3_TRIG": "DMA1_Channel6",
                   "USART2_TX": "DMA1_Channel7", "I2C1_RX": "DMA1_Channel7", "TIM2_CH2": "DMA1_Channel7",
                   "TIM2_CH4": "DMA1_Channel7", "TIM4_UP": "DMA1_Channel7",

                   # Uncomment the following line if your device supports DMA2 : The DMA2 controller and its relative requests are available only in high-density and connectivity line devices.
                   # ADC3, SDIO and TIM8 DMA requests are available only in high-density devices.
                   "SPI3_RX": "DMA2_Channel1", "I2S3_RX": "DMA2_Channel1", "TIM5_CH4": "DMA2_Channel1",
                   "TIM5_TRIG": "DMA2_Channel1", "TIM8_CH3": "DMA2_Channel1", "TIM8_UP": "DMA2_Channel1",
                   "SPI3_TX": "DMA2_Channel2", "I2S3_TX": "DMA2_Channel2", "TIM5_CH3": "DMA2_Channel2",
                   "TIM5_UP": "DMA2_Channel2", "TIM8_CH4": "DMA2_Channel2", "TIM8_TRIG": "DMA2_Channel2",
                   "TIM8_COM": "DMA2_Channel2",
                   "UART4_RX": "DMA2_Channel3", "TIM6_UP": "DMA2_Channel3", "DAC_Channel1": "DMA2_Channel3",
                   "TIM8_CH1": "DMA2_Channel3",
                   "SDIO": "DMA2_Channel4", "TIM5_CH2": "DMA2_Channel4", "TIM7_UP": "DMA2_Channel4",
                   "DAC_Channel2": "DMA2_Channel4",
                   "ADC3": "DMA2_Channel5", "UART4_TX": "DMA2_Channel5", "TIM5_CH1": "DMA2_Channel5",
                   "TIM8_CH2": "DMA2_Channel5",
                   }

def get_DMA_Channel(feature: str) -> str:
    if feature not in dma_request_map:
        raise RuntimeError("There is no DMA channel defined for {0} feature".format(feature))

    return dma_request_map[feature]

def get_DMA_from_channel(channel: str) -> str:
    return channel[:4]

# ADC and ADC channels
mcu_available_resources += f"""
    "ADC1" : {{ "type" : "{RT_ADC}", "{KW_REQUIRES}" : {{}}, "features": ["dma_support"], "dma_dr_address" : "0x4001244C", "{KW_BUS}" : "RCC_APB2Periph_ADC1", "adc_handler" : {{"{RT_IRQ_HANDLER}" : "ADC1_2_IRQHandler"}}}},
    "ADC2" : {{ "type" : "{RT_ADC}", "{KW_REQUIRES}" : {{}}, "features": [],              "{KW_BUS}" : "RCC_APB2Periph_ADC2", "adc_handler" : {{"{RT_IRQ_HANDLER}" : "ADC1_2_IRQHandler"}}}},
    """
# ADC Sample time constants
adc_sample_times = {"ADC_SampleTime_1Cycles5", "ADC_SampleTime_7Cycles5", "ADC_SampleTime_13Cycles5",
                    "ADC_SampleTime_28Cycles5", "ADC_SampleTime_41Cycles5", "ADC_SampleTime_55Cycles5",
                    "ADC_SampleTime_71Cycles5", "ADC_SampleTime_239Cycles5"}

# Blue Pill and some other stm32F103x controllers doesn't have ADC3, uncomment it if you need it and your MCU has it
# ADC3 is available only in high-density devices.
# mcu_available_resources += f"""
#    "ADC3" : {{ "type" : "{RT_ADC}", "{KW_REQUIRES}" : {{}}, "features": ["dma_support"], "dma_dr_address" : "0x40013C4C", "{KW_BUS}" : "RCC_APB2Periph_ADC3", "adc_handler" : {{"{RT_IRQ_HANDLER}" : "ADC3_IRQHandler"}}}},
#    """
mcu_available_resources += f"""
    "ADC_Channel_0" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_0"}} }},
    "ADC_Channel_1" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_1"}} }},
    "ADC_Channel_2" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_2"}} }},
    "ADC_Channel_3" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_3"}} }},
    "ADC_Channel_4" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_4"}} }},
    "ADC_Channel_5" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_5"}} }},
    "ADC_Channel_6" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_6"}} }},
    "ADC_Channel_7" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PA_7"}} }},
    "ADC_Channel_8" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PB_0"}} }},
    "ADC_Channel_9" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{"{RT_GPIO}" : "PB_1"}} }},
    "ADC_Channel_TempSensor" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{}}, "use_adc" : "ADC1" }},
    "ADC_Channel_Vrefint" : {{"type" : "{RT_ADC_INPUT}", "{KW_REQUIRES}" : {{}}, "use_adc" : "ADC1" }},
"""
# Blue Pill and some other stm32F103x controllers doesn't these channels, uncomment it if you need it and your MCU has it
# mcu_available_resources += f"""
# "ADC_Channel_10": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_0"}}}},
# "ADC_Channel_11": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_1"}}}},
# "ADC_Channel_12": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_2"}}}},
# "ADC_Channel_13": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_3"}}}},
# "ADC_Channel_14": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_4"}}}},
# "ADC_Channel_15": {{"type": "{RT_ADC_INPUT}", "{KW_REQUIRES}": {{"{RT_GPIO}": "PC_5"}}}},
# """


# Blue Pill has shared memory used by USB and CAN. This resource must be added in order to trigger resource collision when
# CAN and USB is used.
mcu_available_resources += f"""
    "USB_CAN_Shared_SRAM" : {{ "type" : "usb_can_shared_sram", "{KW_REQUIRES}" : {{}} }},
"""

# CAN

mcu_available_resources += f"""
    "CAN1" : {{"type" : "can", "{KW_BUS}" : ["RCC_APB1Periph_CAN1", "RCC_APB2Periph_AFIO"], "{KW_REQUIRES}" : {{
        "sram"  : {{"usb_can_shared_sram" : "USB_CAN_Shared_SRAM"}},
        "CANRX" : {{"{RT_GPIO}" : "PA_11"}},
        "CANTX" : {{"{RT_GPIO}" : "PA_12"}}}},
        "irq_tx_handler" : {{"{RT_IRQ_HANDLER}": "USB_HP_CAN1_TX_IRQHandler"}},
        "irq_rx0_handler" : {{"{RT_IRQ_HANDLER}": "USB_LP_CAN1_RX0_IRQHandler"}},
        "irq_rx1_handler" : {{"{RT_IRQ_HANDLER}": "CAN1_RX1_IRQHandler"}},
        "irq_sce_handler" : {{"{RT_IRQ_HANDLER}": "CAN1_SCE_IRQHandler"}}
    }},
    "CAN1_REMAP" : {{"type" : "can", "{KW_BUS}" : ["RCC_APB1Periph_CAN1", "RCC_APB2Periph_AFIO"], "{KW_REQUIRES}" : {{
        "sram"  : {{"usb_can_shared_sram" : "USB_CAN_Shared_SRAM"}},
        "CANRX" : {{"{RT_GPIO}" : "PB_8"}},
        "CANTX" : {{"{RT_GPIO}" : "PB_9"}}}} ,
        "irq_tx_handler" : {{"{RT_IRQ_HANDLER}": "USB_HP_CAN1_TX_IRQHandler"}},
        "irq_rx0_handler" : {{"{RT_IRQ_HANDLER}": "USB_LP_CAN1_RX0_IRQHandler"}},
        "irq_rx1_handler" : {{"{RT_IRQ_HANDLER}": "CAN1_RX1_IRQHandler"}},
        "irq_sce_handler" : {{"{RT_IRQ_HANDLER}": "CAN1_SCE_IRQHandler"}}
    }},
"""
# "ADC1": {{"type": "{RT_ADC}", "{KW_REQUIRES}": {{}}, "features": ["dma_support"], "dma_dr_address": "0x4001244C",         "{KW_BUS}": "RCC_APB2Periph_ADC1", "adc_handler": {{"{RT_IRQ_HANDLER}": "ADC1_2_IRQHandler"}}}},
# "ADC2": {{"type": "{RT_ADC}", "{KW_REQUIRES}": {{}}, "features": [], "{KW_BUS}": "RCC_APB2Periph_ADC2",         "adc_handler": {{"{RT_IRQ_HANDLER}": "ADC1_2_IRQHandler"}}}},


# USB
mcu_available_resources += f"""
    "USB1" : {{"type" : "usb", "{KW_BUS}" : "RCC_APB1Periph_USB",  "{KW_REQUIRES}" : {{
        "sram"      : {{"usb_can_shared_sram" : "USB_CAN_Shared_SRAM"}},
        "USB_MINUS" : {{"{RT_GPIO}" : "PA_11"}},
        "USB_PLUS"  : {{"{RT_GPIO}" : "PA_12"}} 
    }}}},
"""

# SPI
mcu_available_resources += f"""
    "SPI1" : {{"type" : "{RT_SPI}", "{KW_BUS}": ["RCC_APB2Periph_SPI1", "RCC_APB2Periph_AFIO"], "{KW_REQUIRES}" : {{
        "SPI_SCK" : {{"{RT_GPIO}" : "PA_5"}},
        "SPI_MISO" : {{"{RT_GPIO}" : "PA_6"}},
        "SPI_MOSI" : {{"{RT_GPIO}" : "PA_7"}},
        "SPI_NSS" : {{"{RT_GPIO}" : "PA_4"}},
        "SPI_RX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI1_RX")}"}},
        "SPI_TX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI1_TX")}"}},
        "SPI_IRQ" : {{"{RT_IRQ_HANDLER}" : "SPI1_IRQHandler"}}
        }}
    }},

    "SPI1_REMAP" : {{"type" : "{RT_SPI}", "{KW_BUS}": ["RCC_APB2Periph_SPI1", "RCC_APB2Periph_AFIO"], "{KW_REQUIRES}" : {{
        "SPI_SCK" : {{"{RT_GPIO}" : "PB_3"}},
        "SPI_MISO" : {{"{RT_GPIO}" : "PB_4"}},
        "SPI_MOSI" : {{"{RT_GPIO}" : "PB_5"}},
        "SPI_NSS" : {{"{RT_GPIO}" : "PA_15"}},
        "SPI_RX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI1_RX")}"}},
        "SPI_TX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI1_TX")}"}},
        "SPI_IRQ" : {{"{RT_IRQ_HANDLER}" : "SPI1_IRQHandler"}}
        }}
    }},
    
    "SPI2" : {{"type" : "{RT_SPI}", "{KW_BUS}": ["RCC_APB1Periph_SPI2", "RCC_APB2Periph_AFIO"], "{KW_REQUIRES}" : {{
        "SPI_SCK" : {{"{RT_GPIO}" : "PB_13"}},
        "SPI_MISO" : {{"{RT_GPIO}" : "PB_14"}},
        "SPI_MOSI" : {{"{RT_GPIO}" : "PB_15"}},
        "SPI_NSS" : {{"{RT_GPIO}" : "PB_12"}},
        "SPI_RX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI2_RX")}"}},
        "SPI_TX_DMA" : {{"{RT_DMA}" : "{get_DMA_Channel("SPI2_TX")}"}},
        "SPI_IRQ" : {{"{RT_IRQ_HANDLER}" : "SPI2_IRQHandler"}}
        }}
    }}
"""

# The last line
mcu_available_resources += f"""
}}"""

pull_type_map = {
    KW_PULL_UP: "GPIO_Mode_IPU",
    KW_PULL_DOWN: "GPIO_Mode_IPD",
    KW_PULL_NONE: "GPIO_Mode_IN_FLOATING"
}

out_type_map = {
    KW_PUSH_PULL: "GPIO_Mode_Out_PP",
    KW_OPEN_DRAIN: "GPIO_Mode_Out_OD"
}

trigger_type_map = {
    KW_RISE: (1, 0),
    KW_FALL: (0, 1),
    KW_EDGE: (1, 1)
}

test_lines = mcu_available_resources.splitlines()
mcu_resources = json.loads(mcu_available_resources)

system_clock = 72000000  # 72Mhz

max_address = 15


def is_remaped(obj: str):
    remap_required = len(obj)>6 and obj[-6:] == "_REMAP"
    if remap_required:
        return (obj[:-6], remap_required)
    else:
        return (obj, remap_required)

def get_GPIO(port: str, pin_number: int) -> str:
    if pin_number >= GPIO_PORT_LEN:
        raise RuntimeError("Invalid pin number")
    return "P{0}_{1}".format(port[4], pin_number)


def GPIO_to_port(pinname: str) -> str:
    return "GPIO" + pinname[1]


def GPIO_to_pin_number(pinname: str) -> int:
    return int(pinname[3:])


def GPIO_to_pin_mask(pinname: str) -> str:
    return "GPIO_Pin_" + pinname[3:]


def ADCChannel_to_GPIO(adc_channel: str) -> str:
    return mcu_resources[adc_channel][KW_REQUIRES][RT_GPIO]


def ADC_to_ADCHandler(adc: str) -> str:
    return mcu_resources[adc]["adc_handler"][RT_IRQ_HANDLER]


def get_ADC_MAXVAL() -> int:
    return 4095  # 12 bit ADC


def get_resources_by_type(rtype: str):
    result = list()
    for rname, rdata in mcu_resources:
        if rdata['type']==rtype:
            result.append(rname)

    return result
def get_ADC_CHANL_COUNT() -> int:
    return len(get_resources_by_type('adc_input'))

def check_ADC_sample_time(st: str) -> bool:
    return st in adc_sample_times


def is_GPIO_input(intype: str) -> bool:
    if intype not in mcu_gpio_pin_types:
        raise RuntimeError("{0} is not valid input type")
    return intype in mcu_gpio_input_pin_types

def is_GPIO_open_drain(intype: str) -> bool:
    if intype not in mcu_gpio_pin_types:
        raise RuntimeError("{0} is not valid input type")

    return PIN_OUT_OPEN_DRAIN == intype


def GPIO_to_AFIO_EXTICR(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    reg_indx = pin_number // 4
    return "AFIO_EXTICR{0}_EXTI{1}_{2}".format(reg_indx + 1, pin_number, gpio[:2])


def GPIO_to_PortSource(gpio: str) -> str:
    return "GPIO_PortSourceGPIO" + gpio[1]


def GPIO_to_PinSource(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return "GPIO_PinSource" + str(pin_number)


def GPIO_to_EXTI_line(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return "EXTI_Line{0}".format(pin_number)

def GPIO_to_GPIO_Descr(gpio: str, pin_type: str, default_val: int) -> str:
    if pin_type not in pull_type_map.values() and pin_type not in out_type_map.values():
        raise RuntimeError(f"Incorrect pin type ({pin_type}) for {gpio}")

    if pin_type in pull_type_map.values() and default_val!=0:
        raise RuntimeError(f"Input pins must have default value equal to zero. {gpio} default value is {default_val}")

    pin_number = int(GPIO_to_pin_number(gpio))
    pin_port = GPIO_to_port(gpio)

    return f"{{.type={pin_type}, .port={pin_port}, .pin_mask={1 << pin_number}, .pin_number={pin_number}, .default_val={default_val} }}"


def EXTINum_to_EXTIHandler(extinum: int) -> str:
    if 5 <= extinum <= 9:
        return "EXTI9_5_IRQHandler"
    elif 10 <= extinum <= 15:
        return "EXTI15_10_IRQHandler"
    elif 0 <= extinum <= 4:
        return "EXTI{0}_IRQHandler".format(extinum)
    else:
        raise RuntimeError("Unsupported EXTI line used: {0}".format(extinum))


def EXTINum_to_EXTILine(extinum: int) -> str:
    "EXTI_Line{0}".format(extinum)


def GPIO_to_EXTIHandler(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return EXTINum_to_EXTIHandler(pin_number)


def EXTIline_to_EXTIHandler(extiline: str) -> str:
    extinum = int(extiline[9:])
    return EXTINum_to_EXTIHandler(extinum)


def ISRHandler_to_IRQn(isr_handler: str) -> str:
    return isr_handler[:-7] + "n"


def DMA_from_DMA_Channel(dma_channel: str) -> str:
    return dma_channel[:4]


def get_DMA_DR_for_resource(res: str) -> str:
    if res not in mcu_resources:
        raise RuntimeError("Invalid resource is used")

    if "dma_dr_address" not in mcu_resources[res]:
        raise RuntimeError("An attempt to get 'dma_dr_address' from resource {0} that doesn't support it".format(res))

    return mcu_resources[res]["dma_dr_address"]


def USART_to_resources(usart: str) -> tuple:
    return (mcu_resources[usart][KW_REQUIRES][RT_IRQ_HANDLER],
            mcu_resources[usart][KW_REQUIRES]["RX"][RT_GPIO],
            mcu_resources[usart][KW_REQUIRES]["TX"][RT_GPIO])


def TIMER_to_IRQHandler(timer: str, kind: str = KW_TIMER_IRQ_HANDLER) -> str:
    return mcu_resources[timer][KW_REQUIRES][kind][RT_IRQ_HANDLER]


def get_TIMER_freq(timer: str, prescaller: int = 1) -> int:
    if prescaller < 1 or prescaller > 65535:
        raise RuntimeError("Invalid prescaller value: {0}. Value 1..65535 is expected".format(prescaller))

    mult = 1
    if prescaller > 1:
        mult = 2

    # get number of APB bus RCC_APB ->2<- Periph_TIM1
    n = int(mcu_resources[timer][KW_BUS][7])
    if n == 1:
        return mult * system_clock
    elif n == 2:
        return mult * system_clock
    else:
        raise RuntimeError("Invalid bus definition for timer {0}".format(timer))


def DMA_Channel_to_IRQHandler(dmachannel: str) -> str:
    return mcu_resources[dmachannel]["dma_handler"][RT_IRQ_HANDLER]

def check_i2c_clock_speed(i2c, freq):
    if str(freq) not in mcu_resources[i2c][KW_CLOCK_SPEED]:
        raise RuntimeError(f"Invalid clock speed, allowed values are {', '.join(mcu_resources[i2c][KW_CLOCK_SPEED])} ")

def get_DMA_IT_Flag(channel: str, flag: str) -> str:
    if channel not in mcu_resources:
        raise RuntimeError("Invalid resource is used")

    dma_n = channel[3]
    ch_n = channel[12]

    return "DMA{0}_IT_{1}{2}".format(dma_n, flag, ch_n)


def ENABLE_CLOCK_on_APB(res: list) -> str:
    ahb_set = set()
    apb1_set = set()
    apb2_set = set()

    for r in res:
        rr = mcu_resources[r]
        if KW_BUS in rr:
            bus_list = list()
            bus_value = rr[KW_BUS]
            if isinstance(bus_value, str):
                bus_list.append(bus_value)
            elif isinstance(bus_value, list):
                bus_list = bus_value
            else:
                raise RuntimeError('Wrong type for bus: "{0}"'.format(str(bus_value)))

            for b in bus_list:
                if b in AHB_BUS:
                    ahb_set.add(b)
                elif b in APB1_BUS:
                    apb1_set.add(b)
                elif b in APB2_BUS:
                    apb2_set.add(b)
                else:
                    raise RuntimeError("{0} resource describes incorrect bus value {1}".format(r, b))

    lines = ["#define ENABLE_PERIPH_CLOCK \\"]
    if bool(ahb_set):
        lines.append("RCC_AHBPeriphClockCmd({0}, ENABLE); \\".format("|".join(list(ahb_set))))

    if bool(apb1_set):
        lines.append("RCC_APB1PeriphClockCmd({0}, ENABLE); \\".format("|".join(list(apb1_set))))

    if bool(apb2_set):
        lines.append("RCC_APB2PeriphClockCmd({0}, ENABLE); \\".format("|".join(list(apb2_set))))

    return concat_lines(lines)[:-1]

def check_errata(config):
    i2c1_re = re.compile(r'.*\"i2c\"\s*\:\s*\"I2C1\".*', re.DOTALL)
    spi1_remap_re = re.compile(r'.*SPI_MOSI\S*\"\s*\:\s*\"PB_5\".*', re.DOTALL)

    errata = 'I2C1 with SPI1 remapped and used in master mode'
    if i2c1_re.match(config) and spi1_remap_re.match(config):
        raise RuntimeError(f"ERRATA detected: {errata}")

def generate_init_gpio_bus_function(func_name: str, port_type: str, pins : dict) -> str:
    tab = "    "
    if port_type != "uint16_t":
        raise ValueError("Invalid port_type")
    header = f"""{tab}void {func_name}();
"""
    result = f"""{tab}void {func_name}() {{ \\
"""
    result += f"""{tab}START_PIN_DECLARATION; \\
"""
    result += f"""{tab}{port_type} pin_mask; \\
"""

    for pin_index, (pin_port, pin_number, pin_mode, pin_default) in pins.items():
        result += f"""{tab}pin_mask = (({port_type})1) << {pin_number}; \\
"""
        result += f"""{tab}DECLARE_PIN({pin_port}, pin_mask, {pin_mode}); \\
"""

        if pin_default == 0:
            result += f"""{tab}GPIO_ResetBits({pin_port}, pin_mask); \\
"""
        else:
            result += f"""{tab}GPIO_SetBits({pin_port}, pin_mask); \\
"""
    result += """}
"""
    return (header, result)

def generate_set_gpio_bus_function(func_name: str, inp_type: str, port_type: str, pins : dict) -> str:
    allowed_types = {"uint8_t", "uint16_t", "uint32_t"}
    if inp_type not in allowed_types:
        raise ValueError("Invalid inp_type")

    if port_type != "uint16_t":
        raise ValueError("Invalid port_type")
    header = f"void {func_name}({inp_type} data);"
    result = f"""void {func_name}({inp_type} data) {{ \\
    {port_type} accumulator = 0; \\
    {port_type} port_mask = 0;\\"""

    ports = dict()
    for data_bit, (pin_port, pin_number, pin_mode, pin_default) in pins.items():
        if pin_port not in ports:
            ports[pin_port] = [(data_bit, pin_number, data_bit-pin_number)]
        else:
            ports[pin_port].append((data_bit, pin_number, data_bit-pin_number))

    port_counter = 0
    for port, v in ports.items():
        sorted_by_offset = sorted(v, key=operator.itemgetter(2))
        last_offset = sorted_by_offset[0][2]
        data_mask = 0
        for data_bit, pin, offset in sorted_by_offset:
            if offset != last_offset:
                if data_mask == 0:
                    raise AssertionError("data_mask should be non-zero")

                # update accumulator
                shift_kind = ">>" if last_offset > 0 else "<<"
                result += f"""
    accumulator |= ({port_type})( (data & {data_mask}) {shift_kind} {abs(last_offset)} );\\
    port_mask |= ({port_type})({data_mask} {shift_kind} {abs(last_offset)});\\"""
                last_offset = offset
                data_mask = 0
            data_mask = data_mask + (1 << data_bit)

        # Put last block of bits (it should be anyway)
        shift_kind = ">>" if last_offset > 0 else "<<"
        result += f"""
    accumulator += ({port_type})( (data & {data_mask}) {shift_kind} {abs(offset)} );\\
    port_mask |= ({port_type})({data_mask} {shift_kind} {abs(last_offset)});\\"""

        # write to port
        result += f"""
    {port}->BSRR = port_mask & accumulator;\\
    {port}->BRR = port_mask & ( ~accumulator );\\"""

        port_counter += 1
        if port_counter < len(ports):
            result += f"""
    accumulator = 0;\\
    port_mask = 0;\\"""

    result = result + """
}"""
    return (header, result)


def spi_get_baud_rate_control(spi: str, value : str):
    bus_list = mcu_resources[spi][KW_BUS]
    bus = next(b for b in bus_list if b.find('Periph_SPI') != -1)
    freq = get_bus_frequency(bus)
    in_val = (str(freq) + "_" + value).lower()
    val_map = {
        # 18MHz bus
        "18000000_9mhz":        0x0000, # prescaller=2
        "18000000_4.5mhz":      0x0008, # prescaller=4
        "18000000_2.25mhz":     0x0010, # prescaller=8
        "18000000_1.125mhz":    0x0018, # prescaller=16
        "18000000_562khz":      0x0020, # prescaller=32
        "18000000_281khz":      0x0028, # prescaller=64
        "18000000_140khz":       0x0030, # prescaller=128
        "18000000_70khz":       0x0038, # prescaller=256

        # 72MHz bus
        "72000000_36mhz":       0x0000, # prescaller=2
        "72000000_18mhz":       0x0008, # prescaller=4
        "72000000_9mhz":        0x0010, # prescaller=8
        "72000000_4.5mhz":      0x0018, # prescaller=16
        "72000000_2.25mhz":     0x0020, # prescaller=32
        "72000000_1.125mhz":    0x0028, # prescaller=64
        "72000000_562khz":      0x0030, # prescaller=128
        "72000000_281khz":      0x0038  # prescaller=256
    }

    if in_val not in val_map:
        raise RuntimeError(f'Unsupported clock frequency ({value})for {spi}')

    brc = val_map.get(in_val)
    return brc, freq // (1 << ((brc >> 3) + 1))

def spi_get_clock_phase(value: str):
    val_map = {"first" : 0, "second": 1}
    return val_map.get(str(value).lower())

def spi_get_clock_polarity(value: str):
    val_map = {"idle_low": 0, "idle_high": 1}
    return val_map.get(str(value).lower())

def spi_get_frame_format(value: str):
    val_map = {"msb": 1, "lsb": 0}
    return val_map.get(str(value).lower())

def spi_get_frame_size(value: str):
    val_map = {8: 0, 16: 1}
    return val_map.get(int(value))

def systick_frequency():
    return system_clock