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
EKitFirmware::EKitFirmware(std::shared_ptr<EKitBus>& ebus, int addr) : bus(ebus), firmware_addr(addr) {
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

EKIT_ERROR EKitFirmware::status_to_ext_error(uint8_t cs) const{
	if (vdev_addr!=(COMM_MAX_DEV_ADDR & cs)){
		return EKIT_WRONG_DEVICE;
	}

	if ((cs & COMM_STATUS_BUSY) != 0) {
		return EKIT_DEVICE_BUSY;
	}

	if ((cs & COMM_STATUS_CRC) != 0) {
		return EKIT_CRC_ERROR;
	}

	if ((cs & COMM_STATUS_FAIL) != 0) {
		return EKIT_COMMAND_FAILED;
	}

	if ((cs & COMM_STATUS_OVF) != 0) {
		return EKIT_OVERFLOW;
	}

	return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitFirmware::lock
// Purpose: Instructs active device. If device is acquired, access to other devices will wait
// on internal lock
// int vdev : device id to acquire
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::lock(int vdev){
	std::vector<uint8_t> buf;
	CommResponseHeader hdr;
    EKIT_ERROR err;

	// Attempt to lock bus first
	err = bus->lock(firmware_addr);
	if (err != EKIT_OK) {
		goto done;
	} 

	// Attempt to lock self
	err = EKitBus::lock(vdev);
	if (err != EKIT_OK) {
		bus->unlock();
		goto done;
	} 	

	assert(check_address(vdev));
	vdev_addr = vdev;

	// Prepare command byte to send
	buf.resize(1);
	buf[0]=vdev;

	do {
		err = bus->write(buf);
	} while (err == EKIT_WRITE_FAILED);

    if (err == EKIT_OK) {
		get_status(hdr, true);    	
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
EKIT_ERROR EKitFirmware::get_status(CommResponseHeader& hdr, bool wait_device){
	uint8_t last_op_crc;
	EKIT_ERROR err;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	do {
		// Read header until success
		err = bus->read(&hdr, sizeof(hdr));
		if (err == EKIT_OK) break;
		tools::sleep_ms(1);
	} while (true);

	last_op_crc = hdr.last_crc;

	if (wait_device && (hdr.comm_status & COMM_STATUS_BUSY) != 0) {
		// Device is busy, wait more
	    do {
	    	tools::sleep_ms(1);
	    	err = bus->read(&hdr, sizeof(hdr));
	    } while (err != EKIT_OK || (hdr.comm_status & COMM_STATUS_BUSY) != 0);
	}
	err = status_to_ext_error(hdr.comm_status);

	hdr.last_crc = last_op_crc;
	
    return err;	
}

EKIT_ERROR EKitFirmware::set_opt(int opt, int value) {
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

EKIT_ERROR EKitFirmware::get_opt(int opt, int& value) {
	CHECK_SAFE_MUTEX_LOCKED(bus_lock);
	if (opt == FIRMWARE_OPT_FLAGS) {
		value = flags;
		return EKIT_OK;
	}  else {
		return EKIT_NOT_SUPPORTED;
	}
}

int EKitFirmware::bus_props(int& busid) const {
	busid = BUS_I2C_FIRMWARE;
	return BUS_PROP_READALL;
}


//------------------------------------------------------------------------------------
// EKitFirmware::send
// Purpose: Sends data to firmware
// const void* ptr: buffer with data to send. May be an empty buffer
// uint8_t flags: optional flags to send to the device
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitFirmware::write(const void* ptr, size_t len){//(const std::vector<uint8_t>& data, uint8_t flags){
	std::vector<uint8_t> buf;
	CommResponseHeader rhdr;
	size_t buf_len = len + sizeof(CommCommandHeader);
	buf.resize(buf_len);
	uint8_t* pbuf = buf.data();
	EKIT_ERROR err;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);	

	// Prepare buffer
	PCommCommandHeader phdr = (PCommCommandHeader)pbuf;
	assert(vdev_addr>=0 && vdev_addr <= COMM_MAX_DEV_ADDR);
    phdr->command_byte = vdev_addr | flags;
    phdr->length = len;
    memcpy(pbuf + sizeof(CommCommandHeader), ptr, len);
    phdr->control_crc = tools::calc_contol_sum(pbuf, buf_len, sizeof(CommCommandHeader) - 1);

	do {
		err = bus->write(pbuf, buf_len);
	} while (err == EKIT_WRITE_FAILED);

	if (err == EKIT_OK) {
		// wait device since command may take a while
		// Note, don't bother with CRC here because it's firmware responsibility to check it
		err = get_status(rhdr, true);
	}	

    return err;
}

//------------------------------------------------------------------------------------
// EKitFirmware::read_dev
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
EKIT_ERROR EKitFirmware::read(void* ptr, size_t len){//read_dev(std::vector<uint8_t>& data){
	EKIT_ERROR err;
	std::vector<uint8_t> buf;
	size_t buf_len = len+sizeof(CommResponseHeader);
	uint8_t actual_crc;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	// prepare buffer
	buf.resize(buf_len);
	uint8_t* pbuf = buf.data();
	uint8_t* pdata = pbuf+sizeof(CommResponseHeader);
	PCommResponseHeader phdr = (PCommResponseHeader)pbuf;
	CommResponseHeader rhdr;

	do {
		// Read header until success
		err = bus->read(buf);
	} while (err == EKIT_READ_FAILED);

    if (err != EKIT_OK) {
        goto done;
    }

    // Copy data back
    memcpy(ptr, pdata, len);

    err = status_to_ext_error(phdr->comm_status);
    if (err != EKIT_OK) {
        goto done;
    }

    // Check CRC
    err = get_status(rhdr, false);
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
EKIT_ERROR EKitFirmware::read_all(std::vector<uint8_t>& buffer){
	EKIT_ERROR err;
	CommResponseHeader hdr;

	CHECK_SAFE_MUTEX_LOCKED(bus_lock);

	err = get_status(hdr, true);
    if (err != EKIT_OK) {
        goto done;
    }

    buffer.resize(hdr.length);
    err = EKitBus::read(buffer);

done:    
    return err;	
}

EKIT_ERROR EKitFirmware::open() {
	return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitFirmware::close() {
	return EKIT_NOT_SUPPORTED;
}

EKIT_ERROR EKitFirmware::suspend() {
	return bus->suspend();
}

EKIT_ERROR EKitFirmware::resume() {
	return bus->resume();
}
