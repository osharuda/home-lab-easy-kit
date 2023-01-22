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
#include <atomic>

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
enum EKitBusType : uint8_t {
    BUS_I2C = 1,    ///< I2C
    BUS_UART = 2,    ///< UART (not currently implemented)
    BUS_SPI = 3,    ///< SPI (not currently implemented)
    BUS_I2C_FIRMWARE = 4     ///< Communication protocol between software and firmware.
};

using EKitTimeout = tools::StopWatch<std::chrono::milliseconds>;

/// \class EKitBus
/// \brief Base bus abstraction
class EKitBus {
protected:
     const EKitBusType bus_type;         ///< Bus type.
    tools::safe_mutex bus_lock;          ///< Guarding mutex.
    EKitBusState state = BUS_CLOSED;     ///< Bus state

public:

    /// \brief Copy construction is forbidden
    EKitBus(const EKitBus &) = delete;

    /// \brief Assignment is forbidden
    EKitBus &operator=(const EKitBus &) = delete;

    /// \brief Bus constructor
    EKitBus(const EKitBusType bt);

    /// \brief Destructor (virtual)
    /// \note Important note: Destructor may need to lock bus to terminate communication;
    ///       in this case override destructor must lock bus, close method also require protection from
    ///       race conditions. (#BusLocker may be used)
    virtual ~EKitBus();

    /// \brief Open a bus.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR open(EKitTimeout& to);

    /// \brief Close a bus.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR close();

    /// \brief Lock a bus.
    /// \param to - optional time out to be used.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Once bus is locked, bus_lock is owned, and just owner thread may work with it.
    virtual EKIT_ERROR lock(EKitTimeout& to);

    /// \brief Lock a bus with address.
    /// \param addr - address to be locked (some buses may require address to lock it)
    /// \param to - time out object to detect timeouts.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Once bus is locked, bus_lock is owned, and just owner thread may work with it.
    /// \note Some buses may require address to be set as active and locked as single operation.
    virtual EKIT_ERROR lock(int addr, EKitTimeout& to);

    /// \brief Unlock a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Once bus is unlocked, any other thread may lock it again.
    virtual EKIT_ERROR unlock();

    /// \brief Suspend a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \details This call will close bus descriptor, thus any other processes may access to the bus. This may be required
    ///          in the case of I2C bus where STM32F103x and some other I2C device are sharing the same bus.
    virtual EKIT_ERROR suspend(EKitTimeout& to);

    /// \brief Resume a previously suspended bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \details This call will reopen bus descriptor. This may be required in the case of I2C bus where STM32F103x and
    ///          some other I2C device are sharing the same bus.
    virtual EKIT_ERROR resume(EKitTimeout& to);

    /// \brief Set a bus specific option.
    /// \param opt - bus specific option.
    /// \param value - bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR set_opt(int opt, int value, EKitTimeout& to);

    /// \brief Reads a bus specific option.
    /// \param opt - bus specific option.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR get_opt(int opt, int &value, EKitTimeout& to);

    /// \brief Write data to a bus.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR write(const void *ptr, size_t len, EKitTimeout& to) = 0;

    /// \brief Read data from a bus.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR read(void *ptr, size_t len, EKitTimeout& to) = 0;

    /// \brief Read all the data available on a bus.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Note every bus may support it. In this case EKIT_NOT_SUPPORTED must be returned.
    virtual EKIT_ERROR read_all(std::vector<uint8_t> &buffer, EKitTimeout& to) = 0;

    /// \brief Does write and read by single operation, the first write with subsequent read.
    /// \param wbuf - memory to write.
    /// \param wlen - length of the write buffer.
    /// \param rbuf - memory to read data (may be the same pointer as write buffer, wbuf).
    /// \param rlen - length of the buffer to read data into (amount of data to read).
    /// \param to - timeout counting object.
    /// \return Corresponding EKIT_ERROR error code.
    virtual EKIT_ERROR write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to) = 0;

    /// \brief Returns information about implemented bus.
    /// \param busid - One of the #EKitBusType values identifying actual bus implementation.
    /// \throw Throws EKitException if incompatible bus is specified and some properties flags are not set.
    void check_bus(const EKitBusType busid) const;
};

/// \class BusLocker
/// \brief Bus blocker class to be used to automatically call EKitBus#lock() and EKitBus#unlock().
class BusLocker final {

    EKitBus* sp_ebus; ///< shared pointer to the sp_ebus
    bool locked;

public:
     /// \brief Constructor
     /// \param ebus - reference to shared pointer with EKitBus.
     BusLocker(std::shared_ptr<EKitBus> &ebus, EKitTimeout& to) :
          BusLocker(ebus.get(), to) {
     }

     /// \brief Constructor
     /// \param ebus - Raw pointer to EKitBus.
     BusLocker(EKitBus* ebus, EKitTimeout& to) :
     sp_ebus(ebus),
     locked(true) {
         static const char *const func_name = "BusLocker::BusLocker(no addr)";
         EKIT_ERROR err = sp_ebus->lock(to);
         if (err != EKIT_OK) {
             throw EKitException(func_name, err, "failed to lock bus.");
         }
     }


     /// \brief Constructor
     /// \param ebus - reference to shared pointer with EKitBus.
     BusLocker(std::shared_ptr<EKitBus> &ebus, int addr, EKitTimeout& to) :
        BusLocker(ebus.get(), addr, to) {}

    BusLocker(EKitBus* ebus, int addr, EKitTimeout& to) :
            sp_ebus(ebus),
            locked(true) {
        static const char *const func_name = "BusLocker::BusLocker(addr)";
        EKIT_ERROR err = sp_ebus->lock(addr, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "failed to lock bus.");
        }
    }

    /// \brief Destructor
    ~BusLocker() {
        if (locked) {
            EKIT_ERROR err = sp_ebus->unlock();
            assert(err == EKIT_OK);
        }

    }

    void unlock() {
        assert(locked);
        if (locked) {
            EKIT_ERROR err = sp_ebus->unlock();
            assert(err == EKIT_OK);
        }
    }
};

/// @}
/// @}
