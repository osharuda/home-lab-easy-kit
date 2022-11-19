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
 *   \brief Testtool utility header
 *   \author Oleh Sharuda
 */

#pragma once
#include <cstddef>
#include "tools.hpp"

#define MDIFF(X,Y) (std::max((X),(Y)) - std::min((X),(Y)));
#define DECLARE_TEST(y) const char* func_name = #y; \
                        size_t _test_counter = 0;
#define REPORT_CASE tools::debug_print("%s : %d (%s:%d)", func_name, _test_counter++, __FILE__, __LINE__);

#define TRY_SIGNAL(s)                              \
    struct sigaction new_action;                   \
    struct sigaction old_action;                   \
    memset(&new_action, 0, sizeof(new_action));    \
    new_action.sa_handler = s ## _HANDLER;         \
    new_action.sa_flags = SA_NODEFER | SA_NOMASK;  \
    sigaction( s, &new_action, &old_action);      \
    if (setjmp(jmpbuf)==0) {

#define CATCH_SIGNAL(s) \
} else {                \
    sigaction((s), &old_action, &new_action);\
}
