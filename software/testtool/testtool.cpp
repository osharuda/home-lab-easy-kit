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
 *   \brief Testtool utility implementation
 *   \author Oleh Sharuda
 */

#include <iostream>
#include "testtool.hpp"
#include <csetjmp>
#include "timer_tests.hpp"
#include "text_tests.hpp"
#include "circbuffer_tests.hpp"
#include "sync_tests.hpp"
#include "misc_tests.hpp"

jmp_buf jmpbuf;
int g_assert_param_count = 0;


void SIGFPE_HANDLER(int signum) {
    g_assert_param_count++;
    longjmp(jmpbuf, 1);
}

void SIGABRT_HANDLER(int signum) {
    g_assert_param_count++;
    longjmp(jmpbuf, 1);
}

int main()
{
    std::cout << "------------------  T E S T    T O O L  ------------------" << std::endl;



    /// Multithreading tests
    test_seq_lock_multithread();
    test_safe_mutex();
    test_circ_buffer_multithreaded();

    /// Circular buffer tests
    test_circbuffer_initialization();
    test_circbuffer_failed_initialization();
    test_circbuffer_single_byte();
    test_circbuffer_byte_mode();
    test_circbuffer_byte_mode_with_status();
    test_circbuffer_single_block();
    test_circbuffer_block_mode_work();
    test_circ_buffer_warning();
    test_circbuffer_block_mode_work_with_status();
    test_circbuffer_asserts();
    test_circbuffer_single_block();
    test_circbuffer_asserts();
    test_circbuffer_block_mode_work();
    test_circbuffer_byte_mode_with_status();
    test_circbuffer_block_mode_work_with_status();
    test_circ_buffer_warning();

    /// Text tests
    test_icu_regex_group();
    test_check_prefix();
    test_append_vector();
    test_split_and_trim();
    test_trim_string();
    test_buffer_to_hex();
    test_buffer_from_hex();
    test_hex_val();

    /// Timer tests
    test_stm32_timer_params_integer();
    test_stm32_timer_params();
    test_StopWatch();

    std::cout << std::endl << "[    S U C C E S S    ]" << std::endl;
    return 0;
}
