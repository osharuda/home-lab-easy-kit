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
 *   \brief EKitBus software implementation header
 *   \author Oleh Sharuda
 */

#pragma once
#include <string>
#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include "tools.hpp"
#include "ekit_error.hpp"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_bus EKitBus
/// \brief Bus abstraction
/// @{
/// \page page_group_communication_bus
/// \tableofcontents
///
/// \section sect_group_communication_bus_01 EKitBus abstraction
///
/// EKitBus is a simple abstraction on a bus. Below is a list of features a bus should expose to be supported with EKitBus:
/// - read from bus.
/// - write to bus.
///
/// Non-mandatory features are:
/// - Bus may have some flags, that may be set with EKitBus#set_opt() and read with EKitBus#get_opt().
/// - Bus may have several addressable devices connected. This feature is implemented with EKitBus#lock() and EKitBus#unlock()
///   calls.
///

/// \enum EKitBusState
/// \brief Describes bus state
enum EKitBusState {
    BUS_OPENED = 0,
    BUS_CLOSED = 1,
    BUS_PAUSED = 2
};

/// \enum EKitBusType
/// \brief Bus type identifiers
enum EKitBusType {
    BUS_I2C             = 1,    ///< I2C
    BUS_USART           = 2,    ///< UART (not currently implemented)
    BUS_SPI             = 3,    ///< SPI (not currently implemented)
    BUS_I2C_FIRMWARE    = 4     ///< Communication protocol between software and firmware.
};

// Bus features/properties bitmask flags
/// \enum EKitBusProperties
/// \brief Bus properties flags.
/// \note Several flags may be used as a bitmask.
enum EKitBusProperties {
    BUS_PROP_READALL = 1    ///< Bus supports EKitBus#read_all() operation.
};

enum EKitBusOptions: int {
    EKITBUS_TIMEOUT = -1      ///< Timeout in milliseconds
};



/// \class EKitBus
/// \brief Base bus abstraction
class EKitBus {
protected:

    tools::safe_mutex bus_lock;          ///< Guarding mutex.
    EKitBusState state = BUS_CLOSED;     ///< Bus state

public:

    /// \brief Copy construction is forbidden
    EKitBus(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitBus& operator=(const EKitBus&) = delete;

    /// \brief Bus constructor
	EKitBus();

    /// \brief Destructor (virtual)
	virtual ~EKitBus();

	/// \brief Open a bus.
	/// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR open();

    /// \brief Close a bus.
    /// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR close();

    /// \brief Lock a bus.
    /// \param address - Address of the device connected to the bus to work with. Ignored for buses that may not address
    ///        several devices.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Once bus is locked, bus_lock is owned, and just owner thread may work with it.
	virtual EKIT_ERROR lock(int address);

    /// \brief Unlock a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Once bus is unlocked, any other thread may lock it again.
	virtual EKIT_ERROR unlock();

	/// \brief Suspend a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \details This call will close bus descriptor, thus any other processes may access to the bus. This may be required
    ///          in the case of I2C bus where STM32F103x and some other I2C device are sharing the same bus.
	virtual EKIT_ERROR suspend();

    /// \brief Resume a previously suspended bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \details This call will reopen bus descriptor. This may be required in the case of I2C bus where STM32F103x and
    ///          some other I2C device are sharing the same bus.
	virtual EKIT_ERROR resume();

	/// \brief Set a bus specific option.
	/// \param opt - bus specific option.
	/// \param value - bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR set_opt(int opt, int value);

	/// \brief Reads a bus specific option.
    /// \param opt - bus specific option.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR get_opt(int opt, int& value);

	/// \brief Write data to a bus.
	/// \param ptr - pointer to the memory block.
	/// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR write(const void* ptr, size_t len) = 0;

    /// \brief Read data from a bus.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
	virtual EKIT_ERROR read(void* ptr, size_t len) = 0;

    /// \brief Read all the data available on a bus.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Note every bus may support it. In this case EKIT_NOT_SUPPORTED must be returned.
	virtual EKIT_ERROR read_all(std::vector<uint8_t>& buffer) = 0;

	/// \brief Read data into vector.
	/// \param buffer - vector with pre-allocated memory to be read.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR read(std::vector<uint8_t>& buffer);

    /// \brief Write data from vector.
    /// \param buffer - vector with memory block to be written.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR write(const std::vector<uint8_t>& buffer);

    /// \brief Returns information about implemented bus.
	/// \param busid - One of the #EKitBusType values.
	/// \return - a bit mask of #EKitBusProperties flags.
	virtual int bus_props(int& busid) const = 0;
};

/// \class BusLocker
/// \brief Bus blocker class to be used to automatically call EKitBus#lock() and EKitBus#unlock().
class BusLocker final {

	std::shared_ptr<EKitBus>& sp_ebus; ///< shared pointer to the sp_ebus

public:
    /// \brief Constructor
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param address - Address of the device connected to the bus to work with. Ignored for buses that may not address
    ///        several devices.
	BusLocker(std::shared_ptr<EKitBus>& ebus, int address) : sp_ebus(ebus){
		static const char* const func_name = "BusLocker::BusLocker";
		EKIT_ERROR err = sp_ebus->lock(address);
		if (err != EKIT_OK) {
			throw EKitException(func_name, err, "failed to lock bus.");
		}
	}

	/// \brief Destructor
	~BusLocker()  {
		EKIT_ERROR err = sp_ebus->unlock();
		assert(err == EKIT_OK);
	}
};

/// @}
/// @}
