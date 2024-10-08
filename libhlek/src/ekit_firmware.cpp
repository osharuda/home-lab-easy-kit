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

#include <cstring>
#include <cassert>
#include "ekit_firmware.hpp"
#include "i2c_proto.h"
#include "tools.hpp"

//------------------------------------------------------------------------------------
// EKitFirmware::EKitFirmware
// std::shared_ptr<EKitBus*>& ebus : shared pointer for EKitBus implementation
// Purpose: EKitFirmware class constructor
//------------------------------------------------------------------------------------
EKitFirmware::EKitFirmware(std::shared_ptr<EKitBus>& ebus, int addr) :
    super(EKitBusType::BUS_I2C_FIRMWARE),
    bus(ebus),
    firmware_addr(addr) {
    ebus->check_bus(EKitBusType::BUS_I2C);
}

//------------------------------------------------------------------------------------
// EKitFirmware::EKitFirmware
// Purpose: EKitFirmware class destructor
//------------------------------------------------------------------------------------
EKitFirmware::~EKitFirmware(){
}

bool EKitFirmware::check_address(int dev_id) {
	bool res = (dev_id >= 0 && dev_id <= COMM_MAX_DEV_ADDR);
	return res;
}

EKIT_ERROR EKitFirmware::process_comm_status(uint8_t cs) {
    EKIT_ERROR err_fail = EKIT_OK;
    EKIT_ERROR err_crc = EKIT_OK;
    EKIT_ERROR err_ovf = EKIT_OK;
    EKIT_ERROR err_busy = EKIT_OK;

	if (vdev_addr!=(COMM_MAX_DEV_ADDR & cs)){
		return EKIT_WRONG_DEVICE;
	}

	if ((cs & COMM_STATUS_FAIL) != 0) {
        err_fail = on_status_fail();
	}

    if ((cs & COMM_STATUS_CRC) != 0) {
        err_crc = on_status_crc();
    }

	if ((cs & COMM_STATUS_OVF) != 0) {
        err_ovf = on_status_ovf();
	}

    if ((cs & COMM_STATUS_BUSY) != 0) {
        err_busy = on_status_busy();
    }

    if (err_fail != EKIT_OK) { return err_fail; }
    else if (err_crc != EKIT_OK) { return err_crc; }
    else if (err_ovf != EKIT_OK) { return err_ovf; }
    else if (err_busy != EKIT_OK) { return err_busy; }

	return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitFirmware::lock
// Purpose: EKitFirmware requires address during the lock. Override to avoid use of this method.
// Returns: EKIT_NOT_SUPPORTED
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::lock(EKitTimeout& to) {
    assert(false);
    return EKIT_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------------
// EKitFirmware::lock
// Purpose: Instructs active device. If device is acquired, access to other devices will wait
// on internal lock
// int vdev : device id to acquire
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::lock(int vdev, EKitTimeout& to){
	std::vector<uint8_t> buf;
    const size_t buf_len = 1;
	CommResponseHeader hdr;
    EKIT_ERROR err;

	// Attempt to lock bus first
	err = bus->lock(firmware_addr, to);
	if (err != EKIT_OK) {
		goto done;
	} 

	// Attempt to lock self
	err = super::lock(to);
	if (err != EKIT_OK) {
		bus->unlock();
		goto done;
	} 	

	assert(check_address(vdev));
	vdev_addr = vdev;

	// Prepare command byte to send
	buf.resize(buf_len);
	buf[0]=vdev;

    // <TODO> Make sure we actually need to communicate with firmware here. If not, remove write to bus
	do {
		err = bus->write(buf.data(), buf_len, to);
	} while (err == EKIT_WRITE_FAILED);

    if (err == EKIT_OK) {
		get_status(hdr, true, to);
    } else {
        EKitBus::unlock();
        bus->unlock();
    }

done:
	return err;
}


//------------------------------------------------------------------------------------
// EKitFirmware::unlock
// Purpose: Instructs to release device. Once device is released, any attempt to read/write/wait device will fail. 
//          To work with device it should be acquired first
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::unlock(){
	vdev_addr = -1;
	EKitBus::unlock();
	bus->unlock();

	return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitFirmware::get_status
// Purpose: Wait until device completes command
// CommResponseHeader& hdr: response header read from firmware
// bool wait_device: true if read until COMM_STATUS_BUSY is cleared, otherwise false
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
/// \brief Returns status of the device
EKIT_ERROR EKitFirmware::get_status(CommResponseHeader& hdr, bool wait_device, EKitTimeout& to){
	uint8_t last_op_crc;
	EKIT_ERROR err;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	do {
		// Read header until success
		err = bus->read(&hdr, sizeof(hdr), to);
		if (err == EKIT_OK) break;
		tools::sleep_ms(1);
	} while (true);

	last_op_crc = hdr.last_crc;
    assert(hdr.dummy == COMM_DUMMY_BYTE);

	if (wait_device && (hdr.comm_status & COMM_STATUS_BUSY) != 0) {
        wait_vdev(hdr, false, to);
	}
	err = process_comm_status(hdr.comm_status);

	hdr.last_crc = last_op_crc;
	
    return err;	
}

EKIT_ERROR EKitFirmware::set_opt(int opt, int value, EKitTimeout& to) {
	CHECK_SAFE_MUTEX_LOCKED(bus_lock);
	if (opt == FIRMWARE_OPT_FLAGS) {
		assert(value>=0 && value<256);
		assert((value & COMM_MAX_DEV_ADDR) == 0);
		flags = value;
		return EKIT_OK;
	}  else {
		return EKIT_NOT_SUPPORTED;
	}
}

EKIT_ERROR EKitFirmware::get_opt(int opt, int& value, EKitTimeout& to) {
	CHECK_SAFE_MUTEX_LOCKED(bus_lock);
	if (opt == FIRMWARE_OPT_FLAGS) {
		value = flags;
		return EKIT_OK;
	}  else {
		return EKIT_NOT_SUPPORTED;
	}
}

//------------------------------------------------------------------------------------
// EKitFirmware::send
// Purpose: Sends data to firmware
// const void* ptr: buffer with data to send. May be an empty buffer
// uint8_t flags: optional flags to send to the device
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::write(const void* ptr, size_t len, EKitTimeout& to){
    std::vector<uint8_t> buf;
    CommResponseHeader rhdr;
    size_t buf_len = len + sizeof(CommCommandHeader);
    buf.resize(buf_len);
    uint8_t* pbuf = buf.data();
    EKIT_ERROR err;

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    // Prepare buffer
    CommCommandHeader* phdr = (CommCommandHeader*)pbuf;
    assert(vdev_addr>=0 && vdev_addr <= COMM_MAX_DEV_ADDR);
    phdr->command_byte = vdev_addr | flags;
    phdr->length = len;
    if (len > 0) {
        memcpy(pbuf + sizeof(CommCommandHeader), ptr, len);
    }
    phdr->control_crc = tools::calc_contol_sum(pbuf, buf_len, sizeof(CommCommandHeader) - 1);

    do {
            err = bus->write(pbuf, buf_len, to);
    } while (err == EKIT_WRITE_FAILED);


    if (err == EKIT_OK) {
        // wait device since command may take a while
        // Note, don't bother with CRC here because it's firmware responsibility to check it
        err = get_status(rhdr, true, to);
    }

    return err;
}

//------------------------------------------------------------------------------------
// EKitFirmware::read
// Purpose: Read from the device fixed amount of bytes
// void* ptr: buffer with data to send. May be an empty buffer.
// bool check_crc: true - to check CRC, false - do not check CRC
// Returns: corresponding EKIT_ERROR code
// Notes: CRC byte is passed when master reads a byte beyond the buffer last byte. 
// If check_crc==true, and call is made to read less bytes that device buffer actually have, 
// then CRC byte will not be returned. Instead of this actual extra byte will be read
// from device buffer. In this case EKIT_CRC_ERROR will be return. Avoid this situation -
// do not use crc check when there is a chance that device buffer has more data than you
// expect to read. Use check_crc == true when you know amount of data in the buffer for sure, otherwise false. 
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::read(void* ptr, size_t len, EKitTimeout& to){
	EKIT_ERROR err;
	std::vector<uint8_t> buf;
	size_t buf_len = len+sizeof(CommResponseHeader);
	uint8_t actual_crc;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	// prepare buffer
	buf.resize(buf_len);
	uint8_t* pbuf = buf.data();
	uint8_t* pdata = pbuf+sizeof(CommResponseHeader);
	CommResponseHeader* phdr = (CommResponseHeader*)pbuf;
	CommResponseHeader rhdr;

	do {
		// Read header until success
		err = bus->read(pbuf, buf_len, to);
	} while (err == EKIT_READ_FAILED);

    if (err != EKIT_OK) {
        goto done;
    }


    // It is possible to request more data the available in device buffer.
    // This check is made to make this situation visible, because it is a logical error:
    // software must be sure device has required amount of data before read it.
    assert(len <= phdr->length);

    // Copy data back
    memcpy(ptr, pdata, len);

    err = process_comm_status(phdr->comm_status);
    if (err != EKIT_OK) {
        goto done;
    }

    // Check CRC
    err = get_status(rhdr, false, to);
	actual_crc = tools::calc_contol_sum(pbuf, buf_len, -1);
	if (actual_crc!=rhdr.last_crc) {
    	err = EKIT_CRC_ERROR;
	} 

done:
    return err;	
}

//------------------------------------------------------------------------------------
// EKitFirmware::read_all
// Purpose: Read from the device fixed amount of bytes
// std::vector<uint8_t> buffer: buffer with data to send. all previous buffer data is discarded and buffer is
//                              filed by the data read from device.
// Returns: corresponding EKIT_ERROR code
// Notes: 1) This function doesn't check CRC because interface imply that buffer contains unknown amount of bytes, so
//           there is no place to pass CRC
//        2) It it possible that device will have unread data after this call, because device may write new data between reading status
//           and actual read from the device
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::read_all(std::vector<uint8_t>& buffer, EKitTimeout& to){
	EKIT_ERROR err;
	CommResponseHeader hdr;
    size_t data_len;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	err = get_status(hdr, true, to);
    if (err != EKIT_OK) {
        goto done;
    }

    data_len = hdr.length+sizeof(CommResponseHeader);
    buffer.resize(data_len);
    err = bus->read(buffer.data(), data_len, to);

    // Move data to remove CommResponseHeader header
    memmove(buffer.data(), buffer.data() + sizeof(CommResponseHeader), hdr.length);
    buffer.resize(hdr.length);

done:
    return err;	
}

EKIT_ERROR EKitFirmware::write_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen, EKitTimeout& to) {
  static const char* const func_name = "EKitUARTBus::write_read";
  assert(false); // MUST BE IMPLEMENTED
  return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitFirmware::open(EKitTimeout& to) {
	return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitFirmware::close() {
	return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitFirmware::suspend(EKitTimeout& to) {
	return bus->suspend(to);
}

EKIT_ERROR EKitFirmware::resume(EKitTimeout& to) {
	return bus->resume(to);
}

EKIT_ERROR EKitFirmware::register_vdev(int dev_id, EKitFirmwareCallbacks* vdev) {
    std::lock_guard<std::mutex> lock(data_lock);
    auto ins = registered_devices.emplace(dev_id, vdev);
    return ins.second ? EKIT_OK : EKIT_ALREADY_CONNECTED;
}

EKIT_ERROR EKitFirmware::unregister_vdev(int dev_id, EKitFirmwareCallbacks* vdev) {
    std::lock_guard<std::mutex> lock(data_lock);
    registered_devices.erase(dev_id);
    return EKIT_OK;
}

EKIT_ERROR EKitFirmware::wait_vdev(CommResponseHeader& hdr, bool yield, EKitTimeout& to) {
    EKIT_ERROR err;
    bool do_again;

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    do {
        err = bus->read(&hdr, sizeof(hdr), to);
        do_again = (err != EKIT_OK) || ((hdr.comm_status & COMM_STATUS_BUSY) != 0);
        assert(hdr.dummy == COMM_DUMMY_BYTE);

        if (yield && do_again) {
            sched_yield();
        }
    } while (do_again);

    return err;
}

EKIT_ERROR EKitFirmware::sync_vdev(CommResponseHeader& hdr, bool yield, EKitTimeout& to) {
    uint8_t dev_id = vdev_addr;
    assert(dev_id >= 0 && dev_id <= COMM_MAX_DEV_ADDR);
    size_t buf_len = sizeof(dev_id);
    EKIT_ERROR err;

    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    do {
        err = bus->write(&dev_id, buf_len, to);
    } while (err == EKIT_WRITE_FAILED);

    err = wait_vdev(hdr, yield, to);
    return err;
}

EKIT_ERROR EKitFirmware::on_status_ovf() {
    std::lock_guard<std::mutex> lock(data_lock);
    EKitFirmwareCallbacks* dev_callbacks = registered_devices.at(vdev_addr);
    EKIT_ERROR err = dev_callbacks->on_status_ovf();
    return err;
}
EKIT_ERROR EKitFirmware::on_status_crc() {
    std::lock_guard<std::mutex> lock(data_lock);
    EKitFirmwareCallbacks* dev_callbacks = registered_devices.at(vdev_addr);
    EKIT_ERROR err = dev_callbacks->on_status_crc();
    return err;
}
EKIT_ERROR EKitFirmware::on_status_fail() {
    std::lock_guard<std::mutex> lock(data_lock);
    EKitFirmwareCallbacks* dev_callbacks = registered_devices.at(vdev_addr);
    EKIT_ERROR err = dev_callbacks->on_status_fail();
    return err;
}
EKIT_ERROR EKitFirmware::on_status_busy() {
    std::lock_guard<std::mutex> lock(data_lock);
    EKitFirmwareCallbacks* dev_callbacks = registered_devices.at(vdev_addr);
    EKIT_ERROR err = dev_callbacks->on_status_busy();
    return err;
}
