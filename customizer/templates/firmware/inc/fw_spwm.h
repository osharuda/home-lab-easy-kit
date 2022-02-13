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
 *   \brief Generated include header of firmware part for SPWM (Software Pulse Width Modulation) device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define SPWM_DEVICE_ENABLED     1

/// \addtogroup group_spwm_dev
/// @{{


{__SPWM_SHARED_HEADER__}

typedef struct tag_SPWM_GPIO_DESCRIPTOR {{
    GPIO_TypeDef*    port;
	uint16_t         mask;
	uint8_t          n_bits;

	// if bit is set it is open_drain_bits it is GPIO_Mode_Out_OD, otherwise push pull pin GPIO_Mode_Out_PP
	uint16_t         open_drain_bits;
	uint16_t		 def_vals;
}} SPWM_GPIO_DESCRIPTOR;

#define SPWM_GPIO_DESCRIPTION   {{ {__SPWM_GPIO_DESCRIPTION__} }}
#define SPWM_TIMER              {__SPWM_TIMER__}
#define SPWM_TIM_IRQ_HANDLER    {__SPWM_TIM_IRQ_HANDLER__}
#define SPWM_TIM_IRQn           {__SPWM_TIM_IRQn__}

/// @}}