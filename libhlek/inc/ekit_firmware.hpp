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
 *   \brief EKitFirmware class header
 *   \author Oleh Sharuda
 */

#pragma once
#include <memory>
#include "ekit_error.hpp"
#include "ekit_bus.hpp"
#include "i2c_proto.h"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_firmware EKitFirmware
/// \brief Firmware abstraction
/// @{
/// \page page_communication_firmware
/// \tableofcontents
///
/// \section sect_communication_firmware_01 Software to firmware communication protocol implementation
///
/// EKitFirmware implements software to firmware communication protocol.
///

/// \class EKitFirmware
/// \brief Software to firmware communication protocol implementation.
class EKitFirmware final : public EKitBus {
    /// \typedef super
    /// \brief Defines parent class
    typedef EKitBus super;

	std::shared_ptr<EKitBus> bus; ///< Shared pointer to a bus that communication is going on with.
	int vdev_addr = -1;           ///< Virtual device address (-1 means no device is currently locked).
	uint8_t flags = 0;            ///< Virtual device specific command flags.
	int firmware_addr;            ///< Firmware address on a bus

	/// \brief Helper function that converts virtual device communication status to #EKIT_ERROR.
	/// \param cs - virtual device communication status.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR status_to_ext_error(uint8_t cs) const;

public:

    /// \enum EKitFirmwareOptions
    enum EKitFirmwareOptions {
        FIRMWARE_OPT_FLAGS = 1      ///< Indicates that device specific command option flags are set
    };

    /// \brief Copy construction is forbidden
    EKitFirmware(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitFirmware& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param ebus - shared pointer with bus to be used.
    /// \param addr - address of the firmware for the bus specified by ebus.
	EKitFirmware(std::shared_ptr<EKitBus>& ebus, int addr);

	/// \brief Destructor (virtual)
	~EKitFirmware() override;

	/// \brief Check if the virtual device address is valid.
	/// \param dev_id - virtual device id
	/// \return true if address is ok, otherwise false.
	static bool check_address(int dev_id);

    /// \brief Implementation of the EKitBus#write() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR write(const void* ptr, size_t len, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#read() virtual function.
    /// \param ptr - pointer to the memory block.
    /// \param len - length of the memory block.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR read(void* ptr, size_t len, EKitTimeout& to) override;

    /// \brief Does write and read by single operation, the first write with subsequent read.
    /// \param wbuf - memory to write.
    /// \param wlen - length of the write buffer.
    /// \param rbuf - memory to read data (may be the same pointer as write buffer, wbuf).
    /// \param rlen - length of the buffer to read data into (amount of data to read).
    /// \param to - timeout counting object.
    /// \return Corresponding EKIT_ERROR error code.
    EKIT_ERROR write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to)  override;

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

    /// \brief Implementation of the EKitBus#lock() virtual function.
    /// \param address - Virtual device id
    /// \param to - optional time out to be used.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR lock(int address, EKitTimeout& to) override;

    /// \brief Lock a bus.
    /// \param to - optional time out to be used.
    /// \return Corresponding EKIT_ERROR error code.
    /// \note Overridden here to make it unusable, since firmware requires address to be locked.
    EKIT_ERROR lock(EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#unlock() virtual function.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR unlock() override;

    /// \brief Implementation of the EKitBus#read_all() virtual function.
    /// \param buffer - Reference to a vector that will receive data from a bus.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR read_all(std::vector<uint8_t>& buffer, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#set_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitFirmwareOptions values.
    /// \param value - bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR set_opt(int opt, int value, EKitTimeout& to) override;

    /// \brief Implementation of the EKitBus#get_opt() virtual function.
    /// \param opt - bus specific option. Must be one of the #EKitFirmwareOptions values.
    /// \param value - reference to bus specific option value.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR get_opt(int opt, int& value, EKitTimeout& to) override;

	/// \brief Reads virtual device status
	/// \param hdr - reference to command response header to be read
	/// \param wait_device - wait until virtual device will not clear #COMM_STATUS_BUSY.
    /// \return Corresponding EKIT_ERROR error code.
	EKIT_ERROR get_status(CommResponseHeader& hdr, bool wait_device, EKitTimeout& to);
};

/// @}
/// @}
