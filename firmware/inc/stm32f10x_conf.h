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
 *   \brief Header required by CMSIS library.
 *   \author Oleh Sharuda
 */

#pragma once

// Other headers are stored at ${STDPERIF_PATH}/inc, where STDPERIF_PATH is defined toolchain.cmake file
#include "stm32f10x_adc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_spi.h"
#include "misc.h"

#ifdef  IGNORE_FAILED_ASSERTS

/// \brief Empty stub, does nothing because IGNORE_FAILED_ASSERTS is defined
#define assert_param(x) ((void)0)

#else

void fail_assert(uint8_t* src, uint32_t line);

/// \brief CMSIS library uses assert_param to check various of things. Let's use the same macro for the same purposes
#define assert_param(x) ((x) ? (void)0 : fail_assert((uint8_t *)__FILE__, __LINE__))

#endif
