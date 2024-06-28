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
    // For the I2C EV IRQ we just have to disable it's interrupt line to do something
    #define ENTER_CRITICAL_SECTION_WRITER(lk) NVIC_DisableIRQ(I2C_IT_EVT);
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) NVIC_EnableIRQ(I2C_IT_EVT);

    // It is known that I2C EV IRQ handler has the highest priority, therefor it can't be superceeded by any of the IRQ
    // handlers. In this case we ommit synchronisation for state reader functions
    #define ENTER_CRITICAL_SECTION_READER(lk) void(0)
    #define LEAVE_CRITICAL_SECTION_READER(lk) void(0)
#elif defined(SEQ_LOCK_CUSTOM_INLINE)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) enter_writer_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) leave_writer_crit_section(lk)

    #define ENTER_CRITICAL_SECTION_READER(lk) enter_reader_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_READER(lk) leave_reader_crit_section(lk)
#elif defined(SEQ_LOCK_DISABLED_IRQ)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()

    #define ENTER_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()
#elif defined(SEQ_LOCK_TEST)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()

    #define ENTER_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->lock()
    #define LEAVE_CRITICAL_SECTION_READER(lk) reinterpret_cast<std::mutex*>((lk)->context)->unlock()
#else
    static_assert(0, "Unknown sequential locking mechanism");
#endif

struct sequential_lock {
    volatile uint32_t counter;
    void* context;
};

__attribute__((always_inline))
static inline void seq_lock_init(struct sequential_lock* lock, void* context) {
    IS_SIZE_ALIGNED(&lock->counter);
    lock->counter = 0;
    lock->context = context;
}

__attribute__((always_inline))
static inline void seq_lock_r_acquire(struct sequential_lock* lock) {
    ENTER_CRITICAL_SECTION_READER((lock));
    lock->counter++;
    LEAVE_CRITICAL_SECTION_READER((lock));
}

__attribute__((always_inline))
static inline void seq_lock_r_release(struct sequential_lock* lock) {
    ENTER_CRITICAL_SECTION_READER((lock));
    lock->counter++;
    LEAVE_CRITICAL_SECTION_READER((lock));
}

#define sl_write_acquire(lock)                                  \
    {                                                           \
    uint32_t lock_counter_copy;                                 \
                                                                \
    do {                                                        \
        do {                                                    \
            lock_counter_copy = (lock)->counter;                \
        } while (lock_counter_copy & 1);

#define sl_write_update(lock)                                   \
        ENTER_CRITICAL_SECTION_WRITER((lock));                  \
        if (lock_counter_copy==(lock)->counter) {

#define sl_write_complete(lock)                                 \
            LEAVE_CRITICAL_SECTION_WRITER((lock));              \
            break;                                              \
    } else {                                                    \
            LEAVE_CRITICAL_SECTION_WRITER((lock));              \
        }                                                       \
    } while (1);                                                \
    }

