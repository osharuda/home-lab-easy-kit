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
 *   \brief UART bus implementation header
 *   \author Oleh Sharuda
 */

#pragma once
#include "ekit_bus.hpp"
#include "ekit_error.hpp"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_uart_impl EKitUARTBus
/// \brief UART implementation
/// @{
/// \page page_communication_uart_impl
/// \tableofcontents
///
/// \section sect_communication_uart_impl_01 UART bus implementation
///
/// EKitUARTBus implements EKitBus with UART support.
///

/// \class EKitUARTBus
/// \brief Direct (w/o firmware) UART bus implementation
class EKitUARTBus final : public EKitBus {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitBus super;

    std::string bus_name;  ///< Name of the bus.
    int uart_descriptor;   ///< UART bus descriptor.

    /// \brief  Helper function for opening uart bus descriptor.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR open_internal(EKitTimeout& to);

   public:
    enum EKitUARTOptions : uint8_t { UART_TO_BE_IMPLEMENTED___ = 0xff };

    /// \brief Copy construction is forbidden
    EKitUARTBus(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitUARTBus& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param file_name - device file name.
    explicit EKitUARTBus(const std::string& file_name);

    /// \brief Destructor (virtual)
    /// \note Important note: Destructor may need to lock bus to terminate communication;
    ///       in this case override destructor must lock bus, close method also require protection from
    ///       race conditions. (#BusLocker may be used)
    ~EKitUARTBus() override;

    /// \brief Implementation of the EKitBus#open() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR open(EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#close() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR close() override;

    /// \brief Implementation of the EKitBus#suspend() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR suspend(EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#resume() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR resume(EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#read() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR read(void* ptr, size_t len, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#read_all() virtual function.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR read_all(std::vector<uint8_t>& buffer, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#write() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR write(const void* ptr, size_t len, EKitTimeout& to) override;

    /// \brief Does write and read by single operation, the first write with
    /// subsequent read.
    /// \param wbuf - memory to write.
    /// \param wlen - length of
    ///               the write buffer.
    /// \param rbuf - memory to read data (may be the same
    ///               pointer as write buffer, wbuf).
    /// \param rlen - length of the buffer to
    ///               read data into (amount of data to read).
    /// \param to - reference to timeout counting object.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR write_read(const uint8_t* wbuf,
                         size_t wlen,
                         uint8_t* rbuf,
                         size_t rlen,
                         EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#set_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitUARTOptions
    /// values. \param value - bus specific option value. \return Corresponding
    /// EKIT_ERROR error code.
    EKIT_ERROR set_opt(int opt, int value, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#get_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitUARTOptions
    /// values. \param value - reference to bus specific option value. \return
    /// Corresponding EKIT_ERROR error code.
    EKIT_ERROR get_opt(int opt, int& value, EKitTimeout& to) override;
};

/// @}

/// @}
