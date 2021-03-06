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
from tools import *


mcu_name = "stm32f103"
mcu_gpio_pin_types = {"GPIO_Mode_IN_FLOATING", "GPIO_Mode_IPU", "GPIO_Mode_IPD", "GPIO_Mode_Out_OD", "GPIO_Mode_Out_PP"}

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

GPIO_PORT_LEN = 16

mcu_available_resources = "{"

# General purpose pins (GPIO)
mcu_available_resources += """
    "PA_0" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA"},
    "PA_1" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_2" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_3" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_4" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_5" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_6" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_7" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_8" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_9" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_10" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_11" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_12" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_13" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_14" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },
    "PA_15" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOA" },

    "PB_0" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_1" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_2" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_3" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_4" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_5" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_6" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_7" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_8" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_9" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_10" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_11" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_12" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_13" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_14" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    "PB_15" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOB" },
    """
mcu_available_resources += """
    "PC_13" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
    "PC_14" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
    "PC_15" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
    """

# The following GPIO is not available on Blue Pill and other STM32F103x devices in LQFP48 packages
# Uncomment the following lines if you need them
# mcu_available_resources += """
#    "PC_0" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_1" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_2" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_3" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_4" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_5" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_6" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_7" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_8" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_9" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_10" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_11" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    "PC_12" : { "type" : "gpio", "requires" : {}, "bus" : "RCC_APB2Periph_GPIOC" },
#    """

# IRQ Handlers (some devices can't share it, therefore it is treated as a resource of MCU)
mcu_available_resources += """
     "WWDG_IRQHandler"                : { "type" : "irq_handler", "requires" : {}},
     "PVD_IRQHandler"                 : { "type" : "irq_handler" , "requires" : {}},
     "TAMPER_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "RTC_IRQHandler"                 : { "type" : "irq_handler" , "requires" : {}},
     "FLASH_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     "RCC_IRQHandler"                 : { "type" : "irq_handler" , "requires" : {}},
     "EXTI0_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "EXTI1_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "EXTI2_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "EXTI3_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "EXTI4_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "DMA1_Channel1_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel2_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel3_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel4_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel5_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel6_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA1_Channel7_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "ADC1_2_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "USB_HP_CAN1_TX_IRQHandler"      : { "type" : "irq_handler" , "requires" : {}},
     "USB_LP_CAN1_RX0_IRQHandler"     : { "type" : "irq_handler" , "requires" : {}},
     "CAN1_RX1_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "CAN1_SCE_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "EXTI9_5_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "TIM1_BRK_TIM9_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_UP_TIM10_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_TRG_COM_TIM11_IRQHandler"  : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_BRK_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_UP_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_TRG_COM_IRQHandler"        : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_CC_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "TIM2_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "TIM3_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "TIM4_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "I2C1_EV_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "I2C1_ER_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "I2C2_EV_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "I2C2_ER_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "SPI1_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "SPI2_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "USART1_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "USART2_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "USART3_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "EXTI15_10_IRQHandler"           : { "type" : "irq_handler" , "requires" : {}, "bus" : "RCC_APB2Periph_AFIO"},
     "RTCAlarm_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "USBWakeUp_IRQHandler"           : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_BRK_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_UP_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_TRG_COM_IRQHandler"        : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_BRK_TIM12_IRQHandler"      : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_UP_TIM13_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_TRG_COM_TIM14_IRQHandler"  : { "type" : "irq_handler" , "requires" : {}},
     "TIM8_CC_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "ADC3_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "FSMC_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "SDIO_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "TIM5_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "SPI3_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "UART4_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     "UART5_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     "TIM6_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "TIM7_IRQHandler"                : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel1_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel2_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel3_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel4_5_IRQHandler"     : { "type" : "irq_handler" , "requires" : {}},
     "CAN1_TX_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "CAN1_RX0_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "OTG_FS_WKUP_IRQHandler"         : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel4_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "DMA2_Channel5_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "ETH_IRQHandler"                 : { "type" : "irq_handler" , "requires" : {}},
     "ETH_WKUP_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "CAN2_TX_IRQHandler"             : { "type" : "irq_handler" , "requires" : {}},
     "CAN2_RX0_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "CAN2_RX1_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "CAN2_SCE_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "OTG_FS_IRQHandler"              : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_BRK_TIM15_IRQHandler"      : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_UP_TIM16_IRQHandler"       : { "type" : "irq_handler" , "requires" : {}},
     "TIM1_TRG_COM_TIM17_IRQHandler"  : { "type" : "irq_handler" , "requires" : {}},
     "CEC_IRQHandler"                 : { "type" : "irq_handler" , "requires" : {}},
     "TIM6_DAC_IRQHandler"            : { "type" : "irq_handler" , "requires" : {}},
     "TIM12_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     "TIM13_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     "TIM14_IRQHandler"               : { "type" : "irq_handler" , "requires" : {}},
     """

# External interrupt lines
mcu_available_resources += """     
     "EXTI_Line0" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line1" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line2" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line3" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line4" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line5" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line6" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line7" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line8" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line9" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line10" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line11" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line12" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line13" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line14" : {"type" : "exti_line", "requires" : {}},
     "EXTI_Line15" : {"type" : "exti_line", "requires" : {}},
     """

# EXTI_Line16 is connected to PVD output, and therefore is not supported
#mcu_available_resources += """
#     "EXTI_Line16" : {"type" : "exti_line", "requires" : {}},"""
# EXTI_Line17 is connected to RTC Alarm event, and therefore is not supported
#mcu_available_resources += """
#     "EXTI_Line17" : {"type" : "exti_line", "requires" : {}},"""
# EXTI_Line18 is connected to USB Wakeup event, and therefore is not supported
#mcu_available_resources += """
#     "EXTI_Line18" : {"type" : "exti_line", "requires" : {}},"""
# EXTI_Line19 is connected to Ethernet Wakeup event, and therefore is not supported
#mcu_available_resources += """
#     "EXTI_Line19" : {"type" : "exti_line", "requires" : {}},"""


# Backup registers
mcu_available_resources += """
    "BKP_DR1" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR2" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR3" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR4" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR5" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR6" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR7" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR8" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR9" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR10" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR11" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR12" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR13" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR14" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR15" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR16" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR17" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR18" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR19" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR20" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR21" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR22" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR23" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR24" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR25" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR26" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR27" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR28" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR29" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR30" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR31" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR32" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR33" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR34" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR35" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR36" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR37" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR38" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR39" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR40" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR41" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
    "BKP_DR42" : {"type" : "bkp", "requires" : {}, "bus" : "RCC_APB1Periph_BKP"},
"""

# Timers
mcu_available_resources += """
    "TIM1": {"type": "timer", "subtype": "advanced", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM1", "timer_handler" : {"irq_handler" : "TIM1_UP_IRQHandler"}},
    "TIM2": {"type": "timer", "subtype": "general",  "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM2", "timer_handler" : {"irq_handler" : "TIM2_IRQHandler"}},
    "TIM3": {"type": "timer", "subtype": "general",  "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM3", "timer_handler" : {"irq_handler" : "TIM3_IRQHandler"}},
    "TIM4": {"type": "timer", "subtype": "general",  "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM4", "timer_handler" : {"irq_handler" : "TIM4_IRQHandler"}},
    "TIM5": {"type": "timer", "subtype": "general",  "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM5", "timer_handler" : {"irq_handler" : "TIM5_IRQHandler"}},
    "TIM6": {"type": "timer", "subtype": "basic",    "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM6", "timer_handler" : {"irq_handler" : "TIM6_IRQHandler"}},
    "TIM7": {"type": "timer", "subtype": "basic",    "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM7", "timer_handler" : {"irq_handler" : "TIM7_IRQHandler"}},
    "TIM8": {"type": "timer", "subtype": "advanced", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM8", "timer_handler" : {"irq_handler" : "TIM8_UP_IRQHandler"}},
    "TIM9": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM9"},
    "TIM10": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM10"},
    "TIM11": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM11"},
    "TIM12": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM12", "timer_handler" : {"irq_handler" : "TIM12_IRQHandler"}},
    "TIM13": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM13", "timer_handler" : {"irq_handler" : "TIM13_IRQHandler"}},
    "TIM14": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB1Periph_TIM14", "timer_handler" : {"irq_handler" : "TIM14_IRQHandler"}},
    "TIM15": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM15"},
    "TIM16": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM16"},
    "TIM17": {"type": "timer", "subtype": "lite", "features": ["16bit"],  "requires": {}, "bus": "RCC_APB2Periph_TIM17"},
"""
# Real Time Clock
mcu_available_resources += """
    "RTC" : {"type" : "rtc", "requires" : {}, "bus" : "RCC_APB1Periph_PWR"},
"""

# USART
mcu_available_resources += """
    "USART1" : {"type" : "usart", "requires" : {"irq_handler" : "USART1_IRQHandler", 
                                                "RX" : {"gpio" : "PA_10"}, 
                                                "TX" : {"gpio" : "PA_9"} },
                "bus" : "RCC_APB2Periph_USART1"},
                                                
    "USART2" : {"type" : "usart", "requires" : {"irq_handler" : "USART2_IRQHandler", 
                                                "RX" : {"gpio" : "PA_3"}, 
                                                "TX" : {"gpio" : "PA_2" } },
                "bus" : "RCC_APB1Periph_USART2" },
                                                
    "USART3" : {"type" : "usart", "requires" : {"irq_handler" : "USART3_IRQHandler", 
                                                "RX" : {"gpio" : "PB_11"}, 
                                                "TX" : {"gpio" : "PB_10"} },
                "bus" : "RCC_APB1Periph_USART3" },
"""

# I2C
mcu_available_resources += """
    "I2C1" : {"type" : "i2c", "requires" : {"ev_irq_handler" : {"irq_handler" : "I2C1_EV_IRQHandler"}, 
                                            "er_irq_handler" : {"irq_handler" : "I2C1_ER_IRQHandler"}, 
                                            "SDA" : {"gpio" : "PB_7"}, 
                                            "SCL" : {"gpio" : "PB_6"}},
                "bus" : "RCC_APB1Periph_I2C1"},
                                            
    "I2C2" : {"type" : "i2c", "requires" : {"ev_irq_handler" : {"irq_handler" : "I2C2_EV_IRQHandler"}, 
                                            "er_irq_handler" : {"irq_handler" : "I2C2_ER_IRQHandler"},
                                            "SDA" : {"gpio" : "PB_11"}, 
                                            "SCL" : {"gpio" : "PB_10"}},
                "bus" : "RCC_APB1Periph_I2C2"},
"""

# DMA controllers and channels
mcu_available_resources += """
    "DMA1" : { "type" : "dma", "requires" : {}, "bus" : "RCC_AHBPeriph_DMA1"},
    """
# The DMA2 controller and its relative requests are available only in high-density and connectivity line devices
# mcu_available_resources += """
#    "DMA2" : { "type" : "dma", "requires" : {}, "bus" : "RCC_AHBPeriph_DMA2"},
#    """
mcu_available_resources += """
    "DMA1_Channel1" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel1_IRQHandler"}},
    "DMA1_Channel2" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel2_IRQHandler"}},
    "DMA1_Channel3" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel3_IRQHandler"}},
    "DMA1_Channel4" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel4_IRQHandler"}},
    "DMA1_Channel5" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel5_IRQHandler"}},
    "DMA1_Channel6" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel6_IRQHandler"}},
    "DMA1_Channel7" : { "type" : "dma_channel", "requires" : {"dma" : "DMA1"}, "bus" : "RCC_AHBPeriph_DMA1", "dma_handler" : {"irq_handler" : "DMA1_Channel7_IRQHandler"}},
    """

# The DMA2 controller and its relative requests are available only in high-density and connectivity line devices
# mcu_available_resources += """
#    "DMA2_Channel1" : { "type" : "dma_channel", "requires" : {"dma" : "DMA2"}, "bus" : "RCC_AHBPeriph_DMA2", "dma_handler" : {"irq_handler" : "DMA2_Channel1_IRQHandler"}},
#    "DMA2_Channel2" : { "type" : "dma_channel", "requires" : {"dma" : "DMA2"}, "bus" : "RCC_AHBPeriph_DMA2", "dma_handler" : {"irq_handler" : "DMA2_Channel2_IRQHandler"}},
#    "DMA2_Channel3" : { "type" : "dma_channel", "requires" : {"dma" : "DMA2"}, "bus" : "RCC_AHBPeriph_DMA2", "dma_handler" : {"irq_handler" : "DMA2_Channel3_IRQHandler"}},
#    "DMA2_Channel4" : { "type" : "dma_channel", "requires" : {"dma" : "DMA2"}, "bus" : "RCC_AHBPeriph_DMA2", "dma_handler" : {"irq_handler" : "DMA2_Channel4_IRQHandler"}},
#    "DMA2_Channel5" : { "type" : "dma_channel", "requires" : {"dma" : "DMA2"}, "bus" : "RCC_AHBPeriph_DMA2", "dma_handler" : {"irq_handler" : "DMA2_Channel5_IRQHandler"}},
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

# ADC and ADC channels
mcu_available_resources += """
    "ADC1" : { "type" : "adc", "requires" : {}, "features": ["dma_support"], "dma_dr_address" : "0x4001244C", "bus" : "RCC_APB2Periph_ADC1", "adc_handler" : {"irq_handler" : "ADC1_2_IRQHandler"}},
    "ADC2" : { "type" : "adc", "requires" : {}, "features": [],              "bus" : "RCC_APB2Periph_ADC2", "adc_handler" : {"irq_handler" : "ADC1_2_IRQHandler"}},
    """
# ADC Sample time constants
adc_sample_times = {"ADC_SampleTime_1Cycles5", "ADC_SampleTime_7Cycles5", "ADC_SampleTime_13Cycles5",
                    "ADC_SampleTime_28Cycles5", "ADC_SampleTime_41Cycles5", "ADC_SampleTime_55Cycles5",
                    "ADC_SampleTime_71Cycles5", "ADC_SampleTime_239Cycles5"}

# Blue Pill and some other stm32F103x controllers doesn't have ADC3, uncomment it if you need it and your MCU has it
# ADC3 is available only in high-density devices.
# mcu_available_resources += """
#    "ADC3" : { "type" : "adc", "requires" : {}, "features": ["dma_support"], "dma_dr_address" : "0x40013C4C", "bus" : "RCC_APB2Periph_ADC3", "adc_handler" : {"irq_handler" : "ADC3_IRQHandler"}},
#    """
mcu_available_resources += """
    "ADC_Channel_0" : {"type" : "adc_input", "requires" : {"gpio" : "PA_0"} },
    "ADC_Channel_1" : {"type" : "adc_input", "requires" : {"gpio" : "PA_1"} },
    "ADC_Channel_2" : {"type" : "adc_input", "requires" : {"gpio" : "PA_2"} },
    "ADC_Channel_3" : {"type" : "adc_input", "requires" : {"gpio" : "PA_3"} },
    "ADC_Channel_4" : {"type" : "adc_input", "requires" : {"gpio" : "PA_4"} },
    "ADC_Channel_5" : {"type" : "adc_input", "requires" : {"gpio" : "PA_5"} },
    "ADC_Channel_6" : {"type" : "adc_input", "requires" : {"gpio" : "PA_6"} },
    "ADC_Channel_7" : {"type" : "adc_input", "requires" : {"gpio" : "PA_7"} },
    "ADC_Channel_8" : {"type" : "adc_input", "requires" : {"gpio" : "PB_0"} },
    "ADC_Channel_9" : {"type" : "adc_input", "requires" : {"gpio" : "PB_1"} },
    "ADC_Channel_TempSensor" : {"type" : "adc_input", "requires" : {}, "use_adc" : "ADC1" },
    "ADC_Channel_Vrefint" : {"type" : "adc_input", "requires" : {}, "use_adc" : "ADC1" }
}"""
# Blue Pill and some other stm32F103x controllers doesn't these channels, uncomment it if you need it and your MCU has it
# mcu_available_resources += """
# "ADC_Channel_10": {"type": "adc_input", "requires": {"gpio": "PC_0"}},
# "ADC_Channel_11": {"type": "adc_input", "requires": {"gpio": "PC_1"}},
# "ADC_Channel_12": {"type": "adc_input", "requires": {"gpio": "PC_2"}},
# "ADC_Channel_13": {"type": "adc_input", "requires": {"gpio": "PC_3"}},
# "ADC_Channel_14": {"type": "adc_input", "requires": {"gpio": "PC_4"}},
# "ADC_Channel_15": {"type": "adc_input", "requires": {"gpio": "PC_5"}},
# """

mcu_resources = json.loads(mcu_available_resources)

system_clock = 72000000  # 72Mhz

max_address = 15

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
    return mcu_resources[adc_channel]["requires"]["gpio"]


def ADC_to_ADCHandler(adc: str) -> str:
    return mcu_resources[adc]["adc_handler"]["irq_handler"]

def get_ADC_MAXVAL() -> int:
    return 4095 # 12 bit ADC


def check_ADC_sample_time(st: str) -> bool:
    return st in adc_sample_times

def is_GPIO_input(intype: str) -> bool:
    if intype not in mcu_gpio_pin_types:
        raise RuntimeError("{0} is not valid input type")

    if intype == "GPIO_Mode_Out_OD" or intype == "GPIO_Mode_Out_PP":
        return False
    else:
        return True


def is_GPIO_open_drain(intype: str) -> bool:
    if intype not in mcu_gpio_pin_types:
        raise RuntimeError("{0} is not valid input type")

    return "GPIO_Mode_Out_OD" == intype


def GPIO_to_AFIO_EXTICR(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    reg_indx = pin_number // 4
    return "AFIO_EXTICR{0}_EXTI{1}_{2}".format(reg_indx + 1, pin_number, gpio[:2])


def GPIO_to_PortSource(gpio: str) -> str:
    return "GPIO_PortSourceGPIO"+gpio[1]

def GPIO_to_PinSource(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return "GPIO_PinSource" + str(pin_number)


def GPIO_to_EXTI_line(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return "EXTI_Line{0}".format(pin_number)


def EXTINum_to_EXTIHandler(extinum : int) -> str:
    if 5 <= extinum <= 9:
        return "EXTI9_5_IRQHandler"
    elif 10 <= extinum <= 15:
        return "EXTI15_10_IRQHandler"
    elif 0 <= extinum <= 4:
        return "EXTI{0}_IRQHandler".format(extinum)
    else:
        raise RuntimeError("Unsupported EXTI line used: {0}".format(extinum))

def EXTINum_to_EXTILine(extinum : int) -> str:
    "EXTI_Line{0}".format(extinum)

def GPIO_to_EXTIHandler(gpio: str) -> str:
    pin_number = int(GPIO_to_pin_number(gpio))
    return EXTINum_to_EXTIHandler(pin_number)


def EXTIline_to_EXTIHandler(extiline : str) -> str:
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
    return (mcu_resources[usart]["requires"]["irq_handler"],
            mcu_resources[usart]["requires"]["RX"]["gpio"],
            mcu_resources[usart]["requires"]["TX"]["gpio"])


def TIMER_to_IRQHandler(timer: str, kind: str = "timer_handler") -> str:
    return mcu_resources[timer][kind]["irq_handler"]


def get_TIMER_freq(timer: str, prescaller: int = 1) -> int:
    if prescaller < 1 or prescaller > 65535:
        raise RuntimeError("Invalid prescaller value: {0}. Value 1..65535 is expected".format(prescaller))

    mult = 1
    if prescaller > 1:
        mult = 2

    # get number of APB bus RCC_APB ->2<- Periph_TIM1
    n = int(mcu_resources[timer]["bus"][7])
    if n == 1:
        return mult*system_clock // 2
    elif n == 2:
        return mult*system_clock
    else:
        raise RuntimeError("Invalid bus definition for timer {0}".format(timer))


def DMA_Channel_to_IRQHandler(dmachannel: str) -> str:
    return mcu_resources[dmachannel]["dma_handler"]["irq_handler"]


def get_DMA_Channel(feature: str) -> str:
    if feature not in dma_request_map:
        raise RuntimeError("There is no DMA channel defined for {0} feature".format(feature))

    return dma_request_map[feature]

def get_DMA_IT_Flag(channel: str, flag : str) -> str:
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
        if "bus" in rr:
            bus = rr["bus"]
            if bus in AHB_BUS:
                ahb_set.add(bus)
            elif bus in APB1_BUS:
                apb1_set.add(bus)
            elif bus in APB2_BUS:
                apb2_set.add(bus)
            else:
                raise RuntimeError("{0} resource describes incorrect bus value {1}".format(r, bus))

    lines = ["#define ENABLE_PERIPH_CLOCK \\"]
    if bool(ahb_set):
        lines.append("RCC_AHBPeriphClockCmd({0}, ENABLE); \\".format("|".join(list(ahb_set))))

    if bool(apb1_set):
        lines.append("RCC_APB1PeriphClockCmd({0}, ENABLE); \\".format("|".join(list(apb1_set))))

    if bool(apb2_set):
        lines.append("RCC_APB2PeriphClockCmd({0}, ENABLE); \\".format("|".join(list(apb2_set))))

    return concat_lines(lines)[:-1]
