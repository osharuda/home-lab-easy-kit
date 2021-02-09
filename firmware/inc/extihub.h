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
 *   \brief EXTI hub header.
 *   \author Oleh Sharuda
 */

#pragma once

#ifdef EXTIHUB_DEVICE_ENABLED

/// \defgroup group_exti_hub_group EXTI hub
/// \brief EXTI hub implementation details
/// @{
/// \page page_exti_hub_group
/// \tableofcontents
///
/// \section sect_exti_hub_group_01 EXTI HUB details
///
/// EXTI HUB (EXTernal Interrupt HUB) is an abstraction that allows to use EXTI lines with more flexibility. It is
/// possible for some EXTI lines to share the same interrupt vector. So, when some virtual device uses some EXTI line,
/// other devices can't use other lines that share the same interrupt. This functionality wraps most of the EXTI details
/// and allows all EXTI lines to be used independently.
///
/// Additionally to this:
/// 1. All EXTI callbacks are provided with 64 bit timestamp.
/// 2. All EXTI callbacks are provided with context value.
/// 3. There is possibility to mask/unmask EXTI lines with #MASK_EXTI_PIN and #UNMASK_EXTI_PIN macros.
///
/// Usage of EXTI HUB is straightforward:
/// 1. Call #exti_hub_init() to initialize EXTI HUB.
/// 2. Call #exti_register_callback() to register callback for required EXTI line.
///
/// Of cause, there is a bit of inefficiency during callback call, but this is fair price for flexibility achieved by this
/// functionality.

/// \typedef PFN_EXTIHUB_CALLBACK
/// \brief Defines EXTI HUB callback. Every virtual device that utilize EXTI HUB must use callbacks in order to get
///        notified about external interrupts.
/// \param clock - 64 bit timestamp
/// \param ctx - context value that was passed to #exti_register_callback()
typedef void(*PFN_EXTIHUB_CALLBACK)(uint64_t clock, volatile void* ctx);

#pragma pack(push, 1)

/// \struct tag_ExtiHandlerDescr
/// \brief Internal structure that represents EXTI HUB handler.
typedef struct tag_ExtiHandlerDescr {
    PFN_EXTIHUB_CALLBACK exti_handler;  ///< Pointer to EXTI handler function.

    volatile void* context;             ///< Context value to be passed to the EXTI handler function.
} ExtiHandlerDescr, *PExtiHandlerDescr;

#pragma pack(pop)

/// \brief Masks EXTI line. This will prevent specified exti line to generate interrupts
/// \param pin - EXTI line. It corresponds to pin number used for this EXTI line (see GPIO_TypeDef in CMSIS)
#define MASK_EXTI_PIN(pin)  EXTI->IMR &= ~(1 << (pin));

/// \brief Unmasks EXTI line. This will make specified exti line able to generate interrupts
/// \param pin - EXTI line. It corresponds to pin number used for this EXTI line (see GPIO_TypeDef in CMSIS)
#define UNMASK_EXTI_PIN(pin) EXTI->IMR  |= (1 << (pin));

/// \brief Defines EXTI line.
/// \param exticr1 - EXTICR register that corresponds required EXTI line
/// \param pin - EXTI line. It corresponds to pin number used for this EXTI line (see GPIO_TypeDef in CMSIS)
/// \param _raise - non-zero if EXTI interrupt must triggered on logical level raise
/// \param _fall - non-zero if EXTI interrupt must triggered on logical level fall
/// \param masked - non-zero if EXTI interrupt should be masked (will not trigger interrupts until unmasked). Use
///        #UNMASK_EXTI_PIN to unmask EXTI pin.
#define DEFINE_EXIT_PIN(exticr1, pin, _raise, _fall, masked)    MASK_EXTI_PIN((pin))                    \
                                                                AFIO->EXTICR[(pin) >> 2] &= ~exticr1;	\
													            AFIO->EXTICR[(pin) >> 2] |=  exticr1;	\
                                                                if ((_fall))	{						\
                                                                    EXTI->FTSR |= (1 << (pin));			\
                                                                }										\
                                                                if ((_raise)) {							\
                                                                    EXTI->RTSR |= (1 << (pin));			\
                                                                }                                       \
                                                                if (masked==0) {                        \
                                                                    UNMASK_EXTI_PIN((pin));             \
                                                                }


/// \brief Initializes EXTI HUB. Must be called once in order to initialize EXTI HUB functionality.
void exti_hub_init(void);

/// \brief Registers EXTI HUB callback handler.
/// \param port - port that corresponds to required EXTI line
/// \param pin_num - EXTI line. It corresponds to pin number used for this EXTI line (see GPIO_TypeDef in CMSIS)
/// \param gpio_mode - GPIO mode for EXTI line. Valid values are: GPIO_Mode_IN_FLOATING, GPIO_Mode_IPD and GPIO_Mode_IPU only.
/// \param exti_cr - EXTICR register that corresponds required EXTI line
/// \param raise - non-zero if EXTI interrupt must triggered on logical level raise
/// \param fall - non-zero if EXTI interrupt must triggered on logical level fall
/// \param fn - pointer to the callback function
/// \param ctx - context value to be passed to the callback function
/// \param masked - non-zero if EXTI interrupt should be masked (will not trigger interrupts until unmasked). Use
/// #UNMASK_EXTI_PIN to unmask EXTI pin.
/// \return EXTI pin logical lever read at the moment of this call. 0 (Bit_RESET) correspond low logical level, 1
///         (Bit_SET) correspond high logical level.
uint8_t exti_register_callback(GPIO_TypeDef * port,
                            uint8_t pin_num,
                            GPIOMode_TypeDef gpio_mode,
                            uint16_t exti_cr,
                            uint8_t raise,
                            uint8_t fall,
                            PFN_EXTIHUB_CALLBACK fn,
                            volatile void* ctx,
                            uint8_t masked);

/// @}
#endif