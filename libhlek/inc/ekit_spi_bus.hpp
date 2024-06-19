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
 *   \brief SPI bus implementation header
 *   \author Oleh Sharuda
 */

#pragma once
#include "ekit_bus.hpp"
#include "ekit_error.hpp"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_spi_impl EKitSPIBus
/// \brief SPI implementation
/// @{
/// \page page_communication_spi_impl
/// \tableofcontents
///
/// \section sect_communication_spi_impl_01 SPI bus implementation
///
/// EKitSPIBus implements EKitBus with SPI support.
///

/// \class EKitSPIBus
/// \brief Direct (w/o firmware) SPI bus implementation
class EKitSPIBus final : public EKitBus {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitBus super;

    ///< Name of the bus.
    std::string bus_name;

    ///< SPI bus descriptor.
    int spi_descriptor;

    ///< SPI data from the latest transaction (MISO line, reply from slave)
    ///< Reserved memory for this buffer is always increasing to avoid extra memory allocations.
    std::vector<uint8_t> miso_data;

    ///< Read offset in miso_data. Once SPI transaction is done it is set to 0.
    ///< Any read increases miso_read_offset.
    size_t miso_read_offset = 0;

    ///< Amount of data inside miso_data.
    size_t miso_data_size = 0;

    uint32_t mode = 0;
    /// SPI bus frequency, 100KHz by default.
    uint32_t frequency = 100000;

    /// SPI transaction length in bits (8 bits (0 means 8 bits) by default)
    uint8_t word_size = 0;

    /// Specifies when CS must change (deselect device) after SPI transaction.
    bool cs_change = false;

    /// \brief  Helper function for opening spi bus descriptor.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR open_internal(EKitTimeout& to);

    /// \brief Performs SPI read/write operation
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR spi_read_write(void* buffer, size_t len, EKitTimeout& to);
    EKIT_ERROR spi_update_mode(EKitTimeout& to);
    EKIT_ERROR spi_update_frequency(EKitTimeout& to);
    EKIT_ERROR spi_update_word_size(EKitTimeout& to);

   public:
    /// \enum EKitSPIOptions
    enum EKitSPIOptions {
        ///< Clock phase ( default: 0 ):
        ///< 0 - sample on leading edge
        ///< non-zero - sample on trailing edge
        SPI_OPT_CLOCK_PHASE = 201,

        ///< Clock polarity ( default: 0 ):
        ///< 0 - idle low
        ///< non-zero - idle high
        SPI_OPT_CLOCK_POLARITY = 202,

        ///< Chip select active ( default: 0 ):
        ///< 0 - chip select active low
        ///< non-zero - chip select active high
        SPI_OPT_CS_HIGH = 203,

        ///< Least significant byte goes first ( default: 0 ):
        ///< 0 - most significant bit goes first
        ///< non-zero - least significan bit goes first
        SPI_OPT_LSB_FIRST = 204,

        ///< Use chip select signal ( default: 0 ):
        ///< 0 - XXX
        ///< non-zero - XXX
        SPI_OPT_NO_CS = 205,

        ///< SPI clock frequency XXX ( default: XXX )
        SPI_OPT_CLOCK_FREQUENCY = 206,

        ///< Word size in bits ( default: 8 )
        SPI_OPT_WORD_SIZE = 207,

        ///< True to deselect device before starting the
        ///< next transfer. ( default: 0 )
        SPI_OPT_CS_CHANGE = 208
    };

    /// \brief Copy construction is forbidden
    EKitSPIBus(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitSPIBus& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param file_name - device file name.
    explicit EKitSPIBus(const std::string& file_name);

    /// \brief Destructor (virtual)
    /// \note Important note: Destructor may need to lock bus to terminate communication;
    ///       in this case override destructor must lock bus, close method also require protection from
    ///       race conditions. (#BusLocker may be used)
    ~EKitSPIBus() override;

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
    /// \param opt - bus specific option. Must be one of the #EKitSPIOptions
    /// values. \param value - bus specific option value. \return Corresponding
    /// EKIT_ERROR error code.
    EKIT_ERROR set_opt(int opt, int value, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#get_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitSPIOptions
    ///              values.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR get_opt(int opt, int& value, EKitTimeout& to) override;
};

/// @}

/// @}
