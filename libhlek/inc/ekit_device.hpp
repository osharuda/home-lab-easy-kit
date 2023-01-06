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
 *   \brief Device classes header
 *   \author Oleh Sharuda
 */

#pragma once

#include <thread>
#include <atomic>
#include "ekit_bus.hpp"
#include "ekit_error.hpp"

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_device EKitVirtualDevice
/// \brief Device abstraction
/// @{
/// \page page_communication_device
/// \tableofcontents
///
/// \section sect_communication_device_01 Virtual device abstraction and implementation
///
/// #EKitDeviceBase provides basic abstraction for device to work with. Each device has:
/// - Address on a bus
/// - Name
///
/// #EKitVirtualDevice provides generic implementation for virtual devices.
///
/// Most of the devices are derived from #EKitVirtualDevice. These devices may communicate by EKitFirmware only. If device
/// is derived from #EKitDeviceBase it is not limited by this bus only.
///

/// \class EKitDeviceBase
/// \brief Abstraction of the device
class EKitDeviceBase {
	const std::string dev_name;     ///< Device name.
        int dev_timeout;                    ///< Device timeout.
protected:
	std::shared_ptr<EKitBus> bus;   ///< Shared pointer to the bus this device use for communication.

public:

    /// \brief Copy construction is forbidden
    EKitDeviceBase(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitDeviceBase& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param name - device name.
    EKitDeviceBase(std::shared_ptr<EKitBus>& ebus, const char* name) :
    bus(ebus),
    dev_name(name),
    dev_timeout(0) {
    }

    /// \brief Destructor (virtual)
    virtual ~EKitDeviceBase() {
    }

    /// \brief Returns device name.
    /// \return String with name.
	virtual std::string get_dev_name() const { // <CHECKIT> Do we need virtual function here???
		return dev_name;
	}

    virtual void set_timeout(int to) final { dev_timeout = to;
    }

    virtual int get_timeout() final {
        return dev_timeout;
    }

    /// \brief Returns underlying bus
    /// \return reference to shared pointer for underlying bus.
    std::shared_ptr<EKitBus>& get_bus() {
        return bus;
    }
};


/// \class EKitVirtualDevice
/// \brief Generic implementation for virtual devices.
class EKitVirtualDevice : public EKitDeviceBase{

    ///< \brief Device address
    int dev_addr = 0;

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitDeviceBase super;
public:

    /// \brief Copy construction is forbidden
    EKitVirtualDevice(const EKitBus&) = delete;

    /// \brief Assignment is forbidden
    EKitVirtualDevice& operator=(const EKitBus&) = delete;

    /// \brief Constructor
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class.
    /// \param name - name of the device.
	EKitVirtualDevice(std::shared_ptr<EKitBus>& ebus, int addr, const char* name) : EKitDeviceBase(ebus, name), dev_addr(addr) {
		static const char* const func_name = "EKitVirtualDevice::EKitVirtualDevice";
                bus->check_bus(EKitBusType::BUS_I2C_FIRMWARE);
	}

    /// \brief Returns device address.
    /// \return Device address.
    int get_addr() const {
        return dev_addr;
    }

	/// \brief Destructor (virtual)
	~EKitVirtualDevice() override {
	}
};

/// @}
/// @}