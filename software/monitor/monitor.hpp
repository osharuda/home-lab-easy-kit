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
 *   \brief Firmware to software communication test utility header
 *   \author Oleh Sharuda
 */

#include <string>
#include <vector>
#include "termui.hpp"

using namespace TERMUI;

/// \defgroup group_monitor Monitor
/// \brief Firmware to software communication test utility
/// @{
/// \page page_monitor
/// \tableofcontents
///
/// \section sect_monitor_01 This module is still under construction. Information specified here may be incomplete, inaccurate or be absent at all.
/// \image html under_construction.png
/// \image latex under_construction.eps
///

class CommandHandler;

class MonitorUI : public TUI {
    TUInputWndPtr arg_window;
    TUListWndPtr<std::shared_ptr<CommandHandler>> cmd_window;
    TUTextWndPtr log_window;

    typedef TUI super;

    public:
    MonitorUI();

    void on_log_event(wint_t ch, int err);
    void on_arg_event(wint_t ch, int err);
    void on_cmd_event(wint_t ch, int err);
    void on_event(wint_t ch, int err);

    void add_command(int index, std::shared_ptr<CommandHandler>& handler);
    void log(const TUTextLines& t);
    void log(const std::string& t);
    void on_command();
    void welcome();
    void on_help();
    void message_handler(int index, wint_t ch, int err) override;
};

/// @}