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

#if defined(SEQ_LOCK_CUSTOM_INLINE)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) enter_writer_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) leave_writer_crit_section(lk)

    #define ENTER_CRITICAL_SECTION_READER(lk) enter_reader_crit_section(lk)
    #define LEAVE_CRITICAL_SECTION_READER(lk) leave_reader_crit_section(lk)

    #define SEQ_LOCK_DEFINED 1
#elif defined(SEQ_LOCK_DISABLED_IRQ)
    #define ENTER_CRITICAL_SECTION_WRITER(lk) \
        CRITICAL_SECTION_ENTER;
    #define LEAVE_CRITICAL_SECTION_WRITER(lk) \
        CRITICAL_SECTION_LEAVE;

    #define ENTER_CRITICAL_SECTION_READER(lk) \
        CRITICAL_SECTION_ENTER;
    #define LEAVE_CRITICAL_SECTION_READER(lk) \
        CRITICAL_SECTION_LEAVE;

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



#define seq_lock_write_acquire_optimized(lock)                  \
    {                                                           \
        uint32_t lock_counter_copy;                             \
        uint32_t do_update;                                     \
        uint32_t do_not_update;                                 \
        do {                                                    \
            lock_counter_copy = (lock)->counter;

#define seq_lock_write_update_optimized(lock)                   \
            ENTER_CRITICAL_SECTION_WRITER((lock));              \
            do_update = lock_counter_copy==(lock)->counter;     \
            do_not_update = !do_update;

#define seq_lock_update_variable_optimized(dest, source)        \
            (dest) = ( do_not_update * (dest) ) | ( do_update * (source))

#define seq_lock_write_release_optimized(lock)                  \
            LEAVE_CRITICAL_SECTION_WRITER((lock));              \
        } while (do_not_update);                                \
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

#define seq_lock_write_acquire_optimized(lock)  assert_param(0)

#define seq_lock_write_update_optimized(lock) assert_param(0)

#define seq_lock_update_variable_optimized(dest, source) assert_param(0)

#define seq_lock_write_release_optimized(lock) assert_param(0)
#endif

