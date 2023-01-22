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
 *   \brief I2C bus implementation header
 *   \author Oleh Sharuda
 */

#pragma once
#include "ekit_error.hpp"
#include "ekit_bus.hpp"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_i2c_impl EKitI2CBus
/// \brief I2C implementation
/// @{
/// \page page_communication_i2c_impl
/// \tableofcontents
///
/// \section sect_communication_i2c_impl_01 I2C bus implementation
///
/// EKitI2CBus implements EKitBus with I2C support.
///

/// \class EKitI2CBus
/// \brief Direct (w/o firmware) I2C bus implementation
class EKitI2CBus final : public EKitBus {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitBus super;

	std::string bus_name;         ///< Name of the bus.
	int i2c_descriptor;           ///< I2C bus descriptor.
	int address;                  ///< Address of the I2C slave device.

	/// \brief  Helper function for opening i2c bus descriptor.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR open_internal(EKitTimeout& to);

	/// \brief Performs I2C read/write operation
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR i2c_read_write(uint8_t addr, bool readop, void* buffer, size_t len, EKitTimeout& to);


public:
    /// \brief Check if address is suitable for a bus.
    /// \param addr - address to be checked.
    /// \return true if address is ok, otherwise false.
	static bool check_address(int addr);

    /// \brief Copy construction is forbidden
    EKitI2CBus(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitI2CBus& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param file_name - device file name.
	explicit EKitI2CBus(const std::string& file_name);

	/// \brief Destructor (virtual)
        /// \note Important note: Destructor may need to lock bus to terminate communication;
        ///       in this case override destructor must lock bus, close method also require protection from
        ///       race conditions. (#BusLocker may be used)
	~EKitI2CBus() override;

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

        /// \brief Does write and read by single operation, the first write with subsequent read.
        /// \param wbuf - memory to write.
        /// \param wlen - length of the write buffer.
        /// \param rbuf - memory to read data (may be the same pointer as write buffer, wbuf).
        /// \param rlen - length of the buffer to read data into (amount of data to read).
        /// \param to - timeout counting object.
        /// \return Corresponding EKIT_ERROR error code.
        EKIT_ERROR write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to)  override;

    /// \brief Implementation of the EKitBus#lock() virtual function.
    /// \param addr - Address of the device connected to the bus to work with. Ignored for buses that may not address
    ///        several devices.
    /// \param to - optional time out to be used.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR lock(int addr, EKitTimeout& to) override;

        /// \brief Implementation of the EKitBus#lock() virtual function.
        /// \param to - optional time out to be used.
        /// \return EKIT_NOT_SUPPORTED.
        /// \note This function is not supported and causes failed assertion since i2c bus is addressable.
        EKIT_ERROR lock(EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#unlock() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR unlock() override;

};

/// @}

/// @}
