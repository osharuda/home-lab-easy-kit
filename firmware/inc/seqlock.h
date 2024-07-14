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
 *   \brief Sequential lock.
 *   \author Oleh Sharuda
 */

#pragma once

#include <stdint.h>
#include <utools.h>

#if defined(SEQ_LOCK_I2C_READER)
#define ENTER_CRITICAL_SECTION_WRITER(lk) \
        ASSERT_IRQ_ENABLED                \
        __disable_irq();

#define LEAVE_CRITICAL_SECTION_WRITER(lk) \
        __enable_irq();


// It is known that I2C EV IRQ handler has the highest priority, therefor it can't be superceeded by any of the IRQ
// handlers. In this case we ommit synchronisation for state reader functions
#define ENTER_CRITICAL_SECTION_READER(lk) (void)(0)
#define LEAVE_CRITICAL_SECTION_READER(lk) (void)(0)

#define SEQ_LOCK_DEFINED 1

#elif defined(SEQ_LOCK_I2C_READER_IRQ_BASED)

    // Global flag to avoid I2C bus enabling before preempted interrupt leaved critical section
    extern volatile uint32_t g_i2c_bus_writer_lock_flag;

    #define I2C_BUS_DISABLE_REG (NVIC->ICER + ((uint32_t)((I2C_BUS_EV_IRQ)) >> 5))
    #define I2C_BUS_ENABLE_REG  (NVIC->ISER + ((uint32_t)((I2C_BUS_EV_IRQ)) >> 5))
    #define I2C_BUS_DISABLE_VALUE (1 << ((uint32_t)((I2C_BUS_EV_IRQ)) & 0x1F))

    // All 'threads' in firmware are IRQs and main function. The preemption is based on priorities: therefore:
    // 1. IRQ with the same or lower priority may not preempt currently executed IRQ
    // 2. Each IRQ may set disabled flag into global variable and save previous flag value into context on write lock enter.
    // 3. On write lock leave global variable is restored by value preserved in context.
    // 4. If restored flag becomes cleared it means that it's a last nested lock, and it's ok to enable IRQ
    //    Note: it is possible that IRQ will be enabled twice, because restoring global variable and comparing it
    //          is not atomic operation and may be preempted by IRQ with higher priority. From other side it's not a problem,
    //          so, it is acceptable.
    #define ENTER_CRITICAL_SECTION_WRITER(lk)                           \
        (lk)->context = (void*)g_i2c_bus_writer_lock_flag;     \
        g_i2c_bus_writer_lock_flag = 1;                                 \
        *I2C_BUS_DISABLE_REG = I2C_BUS_DISABLE_VALUE; \
        /*NVIC_DisableIRQ(I2C_BUS_EV_IRQ);*/


    #define LEAVE_CRITICAL_SECTION_WRITER(lk)                           \
        g_i2c_bus_writer_lock_flag = (uint32_t)(lk)->context;                     \
        if (g_i2c_bus_writer_lock_flag == 0) {                          \
            *I2C_BUS_ENABLE_REG = I2C_BUS_DISABLE_VALUE;                                                            \
            /*NVIC_EnableIRQ(I2C_BUS_EV_IRQ);*/                             \
        }


    // It is known that I2C EV IRQ handler has the highest priority, therefor it can't be superceeded by any of the IRQ
    // handlers. In this case we ommit synchronisation for state reader functions
    #define ENTER_CRITICAL_SECTION_READER(lk) (void)(0)
    #define LEAVE_CRITICAL_SECTION_READER(lk) (void)(0)

    #define SEQ_LOCK_DEFINED 1
#elif defined(SEQ_LOCK_CUSTOM_INLINE)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) enter_writer_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) leave_writer_crit_section(lk)

    #define ENTER_CRITICAL_SECTION_READER(lk) enter_reader_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_READER(lk) leave_reader_crit_section(lk)

    #define SEQ_LOCK_DEFINED 1
#elif defined(SEQ_LOCK_DISABLED_IRQ)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) __disable_irq()
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) __enable_irq()

    #define ENTER_CRITICAL_SECTION_READER(lk) __disable_irq()
    #define LEAVE_CRITICAL_SECTION_READER(lk) __enable_irq()

    #define SEQ_LOCK_DEFINED 1
#elif defined(SEQ_LOCK_TEST)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()

    #define ENTER_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()

    #define SEQ_LOCK_DEFINED 1
#else
    #define SEQ_LOCK_DEFINED 0
#endif

struct sequential_lock {
    volatile uint32_t counter;
    void*             context;
};

#if SEQ_LOCK_DEFINED
__attribute__((always_inline))
static inline void seq_lock_init(volatile struct sequential_lock* lock, void* context) {
    IS_SIZE_ALIGNED(&lock->counter);
    lock->counter = 0;
    lock->context = context;
}

__attribute__((always_inline))
static inline void seq_lock_read_acquire_release(volatile struct sequential_lock* lock) {
    ENTER_CRITICAL_SECTION_READER((lock));
    lock->counter++;
    LEAVE_CRITICAL_SECTION_READER((lock));
}

#define seq_lock_read_release seq_lock_read_acquire_release
#define seq_lock_read_acquire seq_lock_read_acquire_release

#define seq_lock_write_acquire(lock)                            \
    {                                                           \
    uint32_t lock_counter_copy;                                 \
                                                                \
    do {                                                        \
        do {                                                    \
            lock_counter_copy = (lock)->counter;                \
        } while (lock_counter_copy & 1);

#define seq_lock_write_update(lock)                             \
        ENTER_CRITICAL_SECTION_WRITER((lock));                  \
        if (lock_counter_copy==(lock)->counter) {

#define seq_lock_write_release(lock)                            \
            LEAVE_CRITICAL_SECTION_WRITER((lock));              \
            break;                                              \
    } else {                                                    \
            LEAVE_CRITICAL_SECTION_WRITER((lock));              \
        }                                                       \
    } while (1);                                                \
    }
#else

// We are not allowed to use sequential lock if critical section type is undefined

__attribute__((always_inline))
static inline void seq_lock_init(volatile struct sequential_lock* lock, void* context) {
    UNUSED(lock);
    UNUSED(context);
    assert_param(0);
}

__attribute__((always_inline))
static inline void seq_lock_read_acquire(volatile struct sequential_lock* lock) {
    UNUSED(lock);
    assert_param(0);
}

__attribute__((always_inline))
static inline void seq_lock_read_release(volatile struct sequential_lock* lock) {
    UNUSED(lock);
    assert_param(0);
}

#define seq_lock_write_acquire(lock) assert_param(0)

#define seq_lock_write_update(lock) assert_param(0)

#define seq_lock_write_release(lock) assert_param(0)
#endif

