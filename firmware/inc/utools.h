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
 *   \brief Multipurpose functions tools and macro definitions header.
 *   \author Oleh Sharuda
 */

#pragma once

/// \brief Macro for suppressing warnings for some unused parameters which actually are not being used.
/// \param x - Unused parameter to be ignored.
#define UNUSED(x) (void)(x)

#ifdef DISABLE_NOT_TESTABLE_CODE
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/// Defines required by code to be tested
#define MCU_FREQUENCY_MHZ                   72
#define MCU_FREQUENCY                       72000000
#define MCU_MAXIMUM_TIMER_US                59652323

#define assert_param(x) assert((x))

#else

#include "fw.h"
#include <limits.h>
#include "stm32f10x_conf.h"

/// \struct GPIO_descr
/// \brief Structure that describes a pin
struct GPIO_descr {
    GPIOMode_TypeDef type;          /// Input/Output type
    GPIO_TypeDef*    port;          /// Port
    uint16_t         pin_mask;      /// Pin mask
    uint8_t          pin_number;    /// Number of the pin
    uint8_t          default_val;   /// Default pin state (depends on type)
};

/// \defgroup group_utools uTools
/// \brief Multipurpose micro tools library
/// @{
/// \page page_utools
/// \tableofcontents
///
/// \section sect_utools_01 Details.
///
/// uTools (micro tools) are made as place to keep some common and widely used features like:
/// - Bit, mask and flag operations.
/// - SysTick interrupt providing global 64 bit tick counter.
/// - Time delay functions.
/// - Timer functions.
/// - Some debugging functions.
///

extern GPIO_TypeDef g_null_port;

extern uint32_t g_dummy_reg32;
extern uint16_t g_dummy_reg16;

/// \brief Allow to get member of the structure size.
/// \param T - structure name
/// \param M - member name (from the structure T)
#define STRUCT_MEMBER_SIZE(T,M) (sizeof( ( (T*)0 ) -> M ) )

#define DBGMCU_CR_DBG_TIM_ALL_STOP (DBGMCU_CR_DBG_TIM1_STOP | \
                                    DBGMCU_CR_DBG_TIM2_STOP | \
                                    DBGMCU_CR_DBG_TIM3_STOP | \
                                    DBGMCU_CR_DBG_TIM4_STOP | \
                                    DBGMCU_CR_DBG_TIM5_STOP | \
                                    DBGMCU_CR_DBG_TIM6_STOP | \
                                    DBGMCU_CR_DBG_TIM7_STOP | \
                                    DBGMCU_CR_DBG_TIM8_STOP | \
                                    DBGMCU_CR_DBG_TIM9_STOP | \
                                    DBGMCU_CR_DBG_TIM10_STOP | \
                                    DBGMCU_CR_DBG_TIM11_STOP | \
                                    DBGMCU_CR_DBG_TIM12_STOP | \
                                    DBGMCU_CR_DBG_TIM13_STOP | \
                                    DBGMCU_CR_DBG_TIM14_STOP | \
                                    DBGMCU_CR_DBG_TIM15_STOP | \
                                    DBGMCU_CR_DBG_TIM16_STOP | \
                                    DBGMCU_CR_DBG_TIM17_STOP)

#define DBGMCU_CR_DBG_I2C_ALL_STOP ( DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT | DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT )

#ifndef NDEBUG
/// \brief This variable is used in order to check if #DISABLE_IRQ and #ENABLE_IRQ macro were used correctly.
/// \note is used in debug builds only.
extern volatile uint8_t g_irq_disabled;
#endif

/// \brief Check if all bits specified by f are set in x.
/// \param x - value to be tested.
/// \param f - bitmask where 1 indicates bit that should be tested for 1, bits with 0 are ignored.
#define IS_SET(x,f)     (((x) & (f))==(f))

/// \brief Check if bits specified by f are actually cleared in x.
/// \param x - value to be tested.
/// \param f - bitmask where 1 indicates bit that should be tested for 0, bits with 0 are ignored.
#define IS_CLEARED(x,f) (((x) & (f))==0)

/// \brief Clears bits specified by f in x.
/// \param x - value to be modified.
/// \param f - bitmask where 1 indicates bit that should be cleared, bits with 0 are ignored.
/// \note This macro will check statically that variables being passed are the same in type size.
///       If static assert doesn't allow compilation, make sure types have the same size or use explicit type casting.
#define CLEAR_FLAGS(x,f) { _Static_assert(sizeof(x) == sizeof(f), "Types size are not the same"); } \
                         ((x) = (x) & (~(f)))

/// \brief Sets bits specified by f in x.
/// \param x - value to be modified.
/// \param f - bitmask where 1 indicates bit that should be set, bits with 0 are ignored.
/// \note This macro will check statically that variables being passed are the same in type size.
///       If static assert doesn't allow compilation, make sure types have the same size or use explicit type casting.
#define SET_FLAGS(x,f)  { _Static_assert(sizeof(x) == sizeof(f), "Types size are not the same") ; } \
                        ((x) = (x) | (f))

/// \brief Sets bits specified by value in x using mask.
/// \param x - value to be modified.
/// \param mask - bitmask where 1 indicates bit of interest that will be modified as specified by value, bits with 0 are
///        ignored.
/// \param value - value that specifies new bit values.
/// \note This macro will check statically that variables being passed are the same in type size.
///       If static assert doesn't allow compilation, make sure types have the same size or use explicit type casting.
#define SET_BIT_FIELD(x, mask, value) { _Static_assert(sizeof(x) == sizeof(mask), "Types size are not the same");   \
                                        _Static_assert(sizeof(x) == sizeof(value), "Types size are not the same"); }\
                                      (x) = (((x) & (~(mask))) | ((value)&(mask)))

/// \brief Checks if flags specified by mask are set as specified
/// \param x - value to be inspected
/// \param mask - a mask that specify bits (flags) of interest
/// \param flags - expected flags
/// \return non-zero if flags specified by mask are set as described by flags
#define CHECK_FLAGS(x, mask, flags) ( ((x) & (mask)) == ((flags) & (mask)) )

/// \brief Checks if flags are not set in the register
/// \param x - value to be inspected
/// \param mask - a mask that specify bits (flags) of interest
/// \param flags - expected flags combinations
/// \return non-zero if flags specified by mask are NOT set as expected
#define FLAGS_ARE_NOT_SET(x, mask, flags) ( ((x) & (mask)) != ((flags) & (mask)) )

/// Converts bit in flag into 0 or 1 using bit offset.
/// \param flag - value with bit of interest.
/// \param bit_offset - offset of the bit of interest.
#define TO_ZERO_OR_ONE(flag, bit_offset) (((flag) >> (bit_offset)) & 1)

/// \brief This macro check if single bit is set in unsiged value
/// \param x - Parameter to check
#define IS_SINGLE_BIT(x) ( ((x)!=0) && (((x) & ((x) - 1))==0) )

/// \brief This macro produce function without parameters with name specified by func_name.
/// \param empty - do not specify it, just leave blank like: MAKE_VOID_FUNCTION_VOID_NAME(, foo)
/// \param func_name - name of the function to be defined
#define MAKE_VOID_FUNCTION_VOID_NAME(empty, func_name) void empty##func_name (void)

/// \brief This macro produces interrupt handler.
/// \param isr_name - name of the function to be defined
#define MAKE_ISR(isr_name) MAKE_VOID_FUNCTION_VOID_NAME(,isr_name)

/// \brief This macro produces indexed interrupt handler.
/// \param isr_name - irq handler name to be defined.
/// \param callee - function to be called by isr_name. This function must accept index parameter.
/// \param index - value to be passed into callee as parameter.
#define MAKE_ISR_WITH_INDEX(isr_name,callee, index) MAKE_ISR(isr_name)	{	\
																callee(index);			\
															}
/// \brief This macro start gpio definition by declaring gpio structure (see GPIO_InitTypeDef in CMSIS).
#define START_PIN_DECLARATION 				GPIO_InitTypeDef gpio;

/// \brief This macro will remap pin(s) specified by port and pin if they have to be used as GPIO.
/// \param port - GPIO port
/// \param pin - pin mask (see GPIO_Pin_XX values in CMSIS)
#define REMAP_GPIO_PIN(port, pin)                                       \
    if ( ((port)==GPIOB && (pin)==(1<<4)) ) {                           \
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);            \
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);        \
    }                                                                   \
                                                                        \
    if ( ((port)==GPIOB && (pin)==(1<<3)) ||                            \
         ((port)==GPIOA && (pin)==(1<<13)) ||                           \
         ((port)==GPIOA && (pin)==(1<<14)) ||                           \
         ((port)==GPIOA && (pin)==(1<<15)) ) {                          \
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);            \
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);        \
    }

/// \brief This macro will remap pin(s) specified by port and pin if they have to be used as I2C.
/// \param port - GPIO port
/// \param pin - pin mask (see GPIO_Pin_XX values in CMSIS)
#define REMAP_I2C_PIN(port, pin)                                        \
    if ( ((port)==GPIOB && (pin)==(1<<8)) ||                            \
         ((port)==GPIOB && (pin)==(1<<9))) {                            \
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);            \
        GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);                   \
    }


/// \brief Defines GPIO pin.
/// \param port - GPIO port
/// \param pin - pin mask (see GPIO_Pin_XX values in CMSIS)
/// \param mode - pin mode (one of GPIO_Mode_XXX values in CMSIS)
#define DECLARE_PIN_NO_REMAP(port, pin, mode)   gpio.GPIO_Pin = (pin);						\
    										    gpio.GPIO_Mode = mode;						\
    										    gpio.GPIO_Speed = GPIO_DEFAULT_SPEED;		\
    										    GPIO_Init( port , &gpio);

/// \brief Defines GPIO pin.
/// \param port - GPIO port
/// \param pin - pin mask (see GPIO_Pin_XX values in CMSIS)
/// \param mode - pin mode (one of GPIO_Mode_XXX values in CMSIS)
#define DECLARE_PIN(port, pin, mode)        REMAP_GPIO_PIN((port), (pin));              \
                                            DECLARE_PIN_NO_REMAP((port), (pin), (mode))

#define PIN_SET_RESET(port, pin)    (port)->BSRR = pin; \
                                    (port)->BRR = pin;

#define PIN_RESET_SET(port, pin)    (port)->BRR = pin; \
                                    (port)->BSRR = pin;

#define SPI_WAIT(spi)   while (((spi)->SR & (SPI_I2S_FLAG_BSY | SPI_I2S_FLAG_TXE)) != SPI_I2S_FLAG_TXE) {}


/// \brief Set's BASEPI register value
/// \note Don't use CMSIS __set_BASEPRI() because it is not inline, which adds a function call. Let's allow compiler to
///       make descision regarding inlining.
static inline void set_BASEPRI(uint32_t x)
{
    __ASM volatile ("MSR basepri, %0" : : "r" (x) );
}

static inline uint32_t get_BASEPRI(void)
{
    uint32_t res=0;
    __ASM volatile ("MRS %0, basepri_max" : "=r" (res) );
    return res;
}




#ifndef NDEBUG

#define ASSERT_OUT_CRIT_SECTION assert_param(g_irq_disabled==0);
#define ASSERT_IN_CRIT_SECTION assert_param(g_irq_disabled==1);

/// \brief Enters critical section for synchronization purposes (disables all virtual devices IRQs)
/// \note Interrupts above IRQ_PRIORITY_CRITICAL_SECTION are not disabled
/// \warning Asserts if CRITICAL_SECTION_ENTER and CRITICAL_SECTION_LEAVE are not called in pairs correctly.
#define CRITICAL_SECTION_ENTER                          \
    set_BASEPRI(IRQ_PRIORITY_CRITICAL_SECTION << 4);  \
    ASSERT_OUT_CRIT_SECTION                             \
    g_irq_disabled = 1;

/// \brief Leaves critical section for synchronization purposes (enables virtual devices IRQs)
/// \note Interrupts above IRQ_PRIORITY_CRITICAL_SECTION are not disabled
/// \warning Asserts if CRITICAL_SECTION_ENTER and CRITICAL_SECTION_LEAVE are not called in pairs correctly.
#define CRITICAL_SECTION_LEAVE                          \
    ASSERT_IN_CRIT_SECTION                              \
    g_irq_disabled = 0;                                 \
    set_BASEPRI(0);

#else

/// \brief Debug statement to check if current instruction is ourside critical section.
#define ASSERT_OUT_CRIT_SECTION (void)(0);

/// \brief Debug statement to check if current instruction is ourside critical section.
#define ASSERT_IN_CRIT_SECTION (void)(0);

/// \brief Enters critical section for synchronization purposes (disables virtual devices IRQs)
#define CRITICAL_SECTION_ENTER set_BASEPRI(IRQ_PRIORITY_CRITICAL_SECTION << 4);

/// \brief Leaves critical section for synchronization purposes (enables virtual devices IRQs)
#define CRITICAL_SECTION_LEAVE  set_BASEPRI(0);
#endif

/// \brief Enters recursive critical section for synchronization purposes (disables virtual devices IRQs)
/// \note Interrupts above IRQ_PRIORITY_CRITICAL_SECTION are not affected.
#define RECURSIVE_CRITICAL_SECTION_ENTER                    \
    {                                                       \
        uint32_t prev_base_rpi = get_BASEPRI();             \
        set_BASEPRI(IRQ_PRIORITY_CRITICAL_SECTION << 4);    \


/// \brief Leaves recursive critical section for synchronization purposes (enables virtual devices IRQs)
/// \note Interrupts above IRQ_PRIORITY_CRITICAL_SECTION are not affected.
#define RECURSIVE_CRITICAL_SECTION_LEAVE                    \
        set_BASEPRI(prev_base_rpi);                         \
    }

/// \brief Initializes miscellaneous checks made for debug builds.
void debug_checks_init(void);

#ifndef ENABLE_SYSTICK
#error "ENABLE_SYSTICK macro is not defined. It should be defined by customizer in any case. Probably utools.h is included before fw.h"
#endif

void delay_loop(uint32_t n);

/// Define this macro to 1 if you need some help during debugging. It allows counted breaks and debug pins.
#define EMERGENCY_DEBUG_TOOLS 0

#if EMERGENCY_DEBUG_TOOLS!=0
/// \brief Implements breakpoint that triggers on N-th call.
/// \param cnt - number of call when breakpoint must trigger.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
/// \details It is possibly just one counted breakpoint.
void counted_break(uint32_t cnt);

/// \brief Enables two debug pins (PC_13, PC_14 and PC_15).
/// \warning These pins shouldn't be used in JSON configuration file.
/// \details Debug pins are established in order to simplify debug using logical analyzer and oscilloscope.
/// \details This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
void enable_debug_pins(void);

/// \brief set debug pin PC_13 to logical 1.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void set_debug_pin_0(void) {
    GPIOC->BSRR = GPIO_Pin_13;
}

/// \brief clears debug pin PC_13 to logical 0.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void clear_debug_pin_0(void) {
    GPIOC->BRR = GPIO_Pin_13;
}

/// \brief set debug pin PC_14 to logical 1.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void set_debug_pin_1(void) {
    GPIOC->BSRR = GPIO_Pin_14;
}

/// \brief clears debug pin PC_14 to logical 0.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void clear_debug_pin_1(void) {
    GPIOC->BRR = GPIO_Pin_14;
}

/// \brief set debug pin PC_15 to logical 1.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void set_debug_pin_2(void) {
    GPIOC->BSRR = GPIO_Pin_15;
}

/// \brief clears debug pin PC_15 to logical 0.
/// \warning This is for debugging only. Normally this function shouldn't be used. To use it place a breakpoint into this function body.
__attribute__((always_inline)) static inline
void clear_debug_pin_2(void) {
    GPIOC->BRR = GPIO_Pin_15;
}
#else
__attribute__((always_inline)) static inline
void counted_break(uint32_t cnt) {UNUSED(cnt);}
__attribute__((always_inline)) static inline
void enable_debug_pins(void) {}
__attribute__((always_inline)) static inline
void set_debug_pin_0(void) {}
__attribute__((always_inline)) static inline
void clear_debug_pin_0(void) {}
__attribute__((always_inline)) static inline
void set_debug_pin_1(void) {}
__attribute__((always_inline)) static inline
void clear_debug_pin_1(void) {}
__attribute__((always_inline)) static inline
void set_debug_pin_2(void) {}
__attribute__((always_inline)) static inline
void clear_debug_pin_2(void) {}
#endif // of EMERGENCY_DEBUG_TOOLS

/// \brief call this function to fail some assertion. Execution will stuck inside until reset.
/// \param src - source file name with failed assertion, pass __FILE__ .
/// \param line - line of the code where with failed assertion, pass __LINE__ .
void fail_assert(uint8_t* src, uint32_t line);

#endif // of DISABLE_NOT_TESTABLE_CODE

#ifdef __cplusplus
extern "C" {
#endif

/// \brief This macro remembers IRQ state (enabled or disabled). Use returned value to disable/recover IRQ.
/// \param irqn - IRQn to specify in state
#define NVIC_IRQ_STATE(irqn)            (NVIC->ISER[((uint32_t)(irqn) >> 5)] & (1 << ((uint32_t)(irqn) & 0x1F)))

/// \brief This macro allows to disable IRQ.
/// \param irqn - IRQn to specify in state
/// \param state - IRQn state returned by NVIC_IRQ_STATE macro
#define NVIC_DISABLE_IRQ(irqn, state)   NVIC->ICER[((uint32_t)(irqn) >> 5)] = (state)

/// \brief This macro recovers state of IRQ.
/// \param irqn - IRQn to specify in state
/// \param state - IRQn state returned by NVIC_IRQ_STATE macro
#define NVIC_RESTORE_IRQ(irqn, state)   NVIC->ISER[((uint32_t)(irqn) >> 5)] = (state)

#define IN_INTERRUPT (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)


/// \brief Checks if pointer is aligned as required
/// \param _ptr - pointer
/// \param _size - size to be aligned
#define IS_ALIGNED(_ptr, _size) \
    assert_param( (((uintptr_t) ( (const void*)(_ptr))) % (_size)) == 0);

/// \brief Checks if pointer is aligned by word
/// \param _ptr - pointer
#define IS_WORD_ALIGNED(_ptr) \
    assert_param( (((uintptr_t) ( (const void*)(_ptr))) % (sizeof(uint32_t))) == 0);


/// \brief Checks if pointer is aligned by word
/// \param _ptr - pointer
#define IS_SIZE_ALIGNED(_ptr) \
    assert_param( (((uintptr_t) ( (const void*)(_ptr))) % (sizeof(*(_ptr)))) == 0);

#ifdef __cplusplus
}
#endif
#ifndef DISABLE_NOT_TESTABLE_CODE
DMA_TypeDef* get_dma_from_channel(DMA_Channel_TypeDef* channel);
uint32_t get_dma_ifcr_it_mask(DMA_Channel_TypeDef* channel);
void test_deinit();

#endif // DISABLE_NOT_TESTABLE_CODE

/// @}
