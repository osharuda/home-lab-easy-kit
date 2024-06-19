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
 *   \brief Support for hlekio kernel module.
 *   \author Oleh Sharuda
 */
#pragma once

#include <time.h>
#include <memory>
#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include "tools.hpp"
#include "hlekio_ioctl.h"

/// \defgroup group_hlekio hlekio
/// \brief HLEKIO support
/// @{
/// \page page_hlekio
/// \tableofcontents
///
/// \section sect_hlekio_01 Work with HLEKIO
///
/// Documentation to be written
/// <CHECKIT> complete documentation ...
///

enum HLEKIODevType : uint8_t {
    Input = HLEKIO_INPUT_DEV,
    Output = HLEKIO_OUTPUT_DEV
};

class HLEKIOBase {
    public:

    /// \brief Reset interrupt line counters
    void reset();

    protected:
    HLEKIOBase(const std::string& dev_name, HLEKIODevType dev_type);
    virtual ~HLEKIOBase();

    /// \brief Set binary mode
    void set_binary();

    /// \brief Close all opened device descriptors
    void close_all();

    /// \brief Get device type
    HLEKIODevType get_type();

    /// \brief Resets file descriptor to the beginning
    void reset_fp(int f);

    int nb_fd = -1; /// File descriptor for non-blocking calls only(inputs only).
    int fd = -1; /// File descriptor for blocking calls.
};

/// \class HLEKIOInput
/// \brief HLEKIO input line support.
class HLEKIOInput final : public HLEKIOBase {
public:
    /// \brief No default constructor
    HLEKIOInput()                              = delete;

    /// \brief Copy construction is forbidden
    HLEKIOInput(const HLEKIOInput&)            = delete;

    /// \brief Assignment is forbidden
    HLEKIOInput& operator=(const HLEKIOInput&) = delete;

    /// \brief Constructor to be used
    /// \param dev_name - Name of the corresponding device.
    HLEKIOInput(const std::string dev_name);

    /// \brief Destructor (virtual)
    ~HLEKIOInput() override;

    /// \brief Get current value
    /// \param info - Driver's input line information (optional)
    /// \return Current pin value
    uint8_t get(hlekio_input_info* info);

    /// \brief Wait interrupt event on the pin
    /// \param to - Timeout object
    /// \param info - Driver's input line information (optional)
    /// \note This method may block caller thread.
    int wait(EKitTimeout& to, hlekio_input_info* info);

    /// \brief Set interrupt debounce value
    /// \param d - new debounce value
    /// \note In order to read current debounce value use \ref HLEKIOInput::read() method with hlekio_input_info.
    void set_debounce(unsigned long d);
};

/// \class HLEKIOOutput
/// \brief HLEKIOOutput input line support.
class HLEKIOOutput final : public HLEKIOBase  {
public:
    /// \brief No default constructor
    HLEKIOOutput()                              = delete;

    /// \brief Copy construction is forbidden
    HLEKIOOutput(const HLEKIOOutput&)            = delete;

    /// \brief Assignment is forbidden
    HLEKIOOutput& operator=(const HLEKIOOutput&) = delete;

    /// \brief Constructor to be used
    /// \param dev_name - Name of the corresponding device.
    HLEKIOOutput(const std::string dev_name);

    /// \brief Destructor.
    ~HLEKIOOutput() override;

    /// \brief Get current value
    /// \param info - Driver's input line information (optional)
    /// \return Current pin value
    uint8_t get();

    /// \brief Wait interrupt event on the pin
    /// \param v - new state of the pin
    void set(uint8_t v);
};

/// @}
