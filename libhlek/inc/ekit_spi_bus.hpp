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
#include "ekit_error.hpp"
#include "ekit_bus.hpp"

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

	std::string bus_name;           ///< Name of the bus.
	int spi_descriptor;             ///< SPI bus descriptor.
	std::vector<uint8_t> miso_data; ///< SPI data (MISO line, reply from slave)
	size_t miso_read_offset = 0;
    size_t miso_data_size = 0;
    uint32_t mode = 0;
    uint32_t frequency;
    uint8_t word_size = 0;
    bool cs_change = 0;

	/// \brief  Helper function for opening spi bus descriptor.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR open_internal();

	/// \brief Performs SPI read/write operation
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR spi_read_write(void* buffer, size_t len);

    EKIT_ERROR spi_update_mode();
    EKIT_ERROR spi_update_frequency();
    EKIT_ERROR spi_update_word_size();


public:

    /// \enum EKitSPIOptions
    enum EKitSPIOptions {
        SPI_OPT_CLOCK_PHASE = 1,      ///< Clock phase ( default: 0 ):
                                      ///< 0 - sample on leading edge
                                      ///< non-zero - sample on trailing edge

        SPI_OPT_CLOCK_POLARITY = 2,   ///< Clock polarity ( default: 0 ):
                                      ///< 0 - idle low
                                      ///< non-zero - idle high

        SPI_OPT_CS_HIGH = 3,          ///< Chip select active ( default: 0 ):
                                      ///< 0 - chip select active low
                                      ///< non-zero - chip select active high

        SPI_OPT_LSB_FIRST = 4,        ///< Least significant byte goes first ( default: 0 ):
                                      ///< 0 - most significant bit goes first
                                      ///< non-zero - least significan bit goes first

        SPI_OPT_NO_CS = 5,            ///< Use chip select signal ( default: 0 ):
                                      ///< 0 - XXX
                                      ///< non-zero - XXX

        SPI_OPT_CLOCK_FREQUENCY = 6,  ///< SPI clock frequency XXX ( default: XXX )

        SPI_OPT_WORD_SIZE = 7,         ///< Word size in bits ( default: 8 )

        SPI_OPT_CS_CHANGE = 8         ///< True to deselect device before starting the next transfer. ( default: 0 )
    };

    /// \brief Copy construction is forbidden
    EKitSPIBus(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitSPIBus& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param file_name - device file name.
	explicit EKitSPIBus(const std::string& file_name);

	/// \brief Destructor (virtual)
	~EKitSPIBus() override;

	/// \brief Implementation of the EKitBus#open() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR open() override;

    /// \brief Implementation of the EKitBus#close() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR close() override;

    /// \brief Implementation of the EKitBus#suspend() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR suspend() override;

    /// \brief Implementation of the EKitBus#resume() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR resume() override;

    /// \brief Implementation of the EKitBus#read() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR read(void* ptr, size_t len) override;

    /// \brief Implementation of the EKitBus#read_all() virtual function.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR read_all(std::vector<uint8_t>& buffer) override;

    /// \brief Implementation of the EKitBus#write() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR write(const void* ptr, size_t len) override;

    /// \brief Implementation of the EKitBus#lock() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR lock() override;

    /// \brief Implementation of the EKitBus#unlock() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR unlock() override;

    /// \brief Implementation of the EKitBus#bus_props() virtual function.
    /// \param busid - One of the #EKitBusType values.
    /// \return - a bit mask of #EKitBusProperties flags.
	int bus_props(int& busid) const override;

    /// \brief Implementation of the EKitBus#set_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitSPIOptions values.
    /// \param value - bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR set_opt(int opt, int value) override;

    /// \brief Implementation of the EKitBus#get_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitSPIOptions values.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR get_opt(int opt, int& value) override;
};

/// @}

/// @}
