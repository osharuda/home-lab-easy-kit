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
 *   \brief I2C bus implementation
 *   \author Oleh Sharuda
 */

#include "ekit_i2c_bus.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <mutex>
#include "tools.hpp"

//------------------------------------------------------------------------------------
// EKitI2CBus::EKitI2CBus
// Purpose: EKitI2CBus class constructor
//------------------------------------------------------------------------------------
EKitI2CBus::EKitI2CBus(const std::string& file_name) : bus_name(file_name){
    i2c_descriptor = 0;
	state = BUS_CLOSED;
	address = -1;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::EKitI2CBus
// Purpose: EKitI2CBus class destructor
//------------------------------------------------------------------------------------
EKitI2CBus::~EKitI2CBus() {
	close();
}

//------------------------------------------------------------------------------------
// EKitI2CBus::open_internal
// Purpose: opens bus handle internally. Private, for internal use only
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::open_internal() {
	if (state==BUS_OPENED) {
		return EKIT_ALREADY_CONNECTED;
	}

    if ((i2c_descriptor = ::open(bus_name.c_str(), O_RDWR)) < 0) {
        return EKIT_OPEN_FAILED;
    }

    // Uncomment if doubt that operational system driver support everything we need.
    // unsigned long data = 0;
    // int res = ioctl(i2c_descriptor, I2C_FUNCS, &data);
    // bool i2csupp = (data & I2C_FUNC_I2C)!=0;
    // bool repstart = (data & I2C_FUNC_NOSTART)!=0;
    // bool mangling = (data & I2C_FUNC_PROTOCOL_MANGLING)!=0;
    // bool tenbit = (data & I2C_FUNC_10BIT_ADDR)!=0;

    return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::open
// Purpose: opens bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::open() {
	EKIT_ERROR err;
	LOCK(bus_lock);

	if (state!=BUS_CLOSED) {
		return EKIT_ALREADY_CONNECTED;
	}

	err = open_internal();

	if (err == EKIT_OK)
		state = BUS_OPENED;

	return err;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::close
// Purpose: closes bus
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::close() {
	LOCK(bus_lock);

	if (state==BUS_CLOSED)
		return EKIT_DISCONNECTED;

	if (state==BUS_OPENED) {
		::close(i2c_descriptor);
	}

	state = BUS_CLOSED;

	return EKIT_OK;
}


bool EKitI2CBus::check_address(int addr) {
	return addr>=0 && addr<=255;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::lock
// Purpose: locks bus for some purposes operation that may require several reads/writes
// int addr: i2c device address
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::lock(int addr) {
	EKIT_ERROR res = EKIT_OK;

	if (!check_address(addr)) {
		res = EKIT_BAD_PARAM;
		assert(false);
		goto done;
	}

	super::lock(addr);

    // <CHECKIT> It seems like EKIT_LOCKED is not required. Get rid of it if true.
	if (address>=0) {
		res = EKIT_LOCKED;
		assert(false);
		super::unlock();
		goto done;
	}

	address=addr;
done:
	return res;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::unlock
// Purpose: unlocks bus when some operation is completed
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::unlock() {
	EKIT_ERROR err = EKIT_OK;

    // <CHECKIT> It seems like EKIT_LOCKED is not required. Get rid of it if true.
	if (address<0) {
		err = EKIT_UNLOCKED;
	}
	address = -1;
	super::unlock();
	return err;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::i2c_read_write
// Purpose: I2C read/write operation on Master. Private, for internal use only
// uint8_t addr: i2c address
// bool readop: true to send data to slave, false to receive data from slave
// uint8_t* buffer: pointer to the buffer. If len==0 buffer is not accessed
// size_t len : amount of bytes for operation
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::i2c_read_write(uint8_t addr, bool readop, void* buffer, size_t len) {
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];
    EKIT_ERROR err;

    // This assert should fail if there is an attempt to use read/write without locking bus first
    assert(check_address(addr));
    CHECK_SAFE_MUTEX_LOCKED(bus_lock);

    msgs[0].addr = addr;
	msgs[0].flags = readop ? I2C_M_RD : I2C_M_STOP;
    msgs[0].len = len;
    msgs[0].buf = (uint8_t*)buffer;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

	if (state==BUS_CLOSED) {
		err = EKIT_NOT_OPENED;
	} else if (state==BUS_PAUSED) {
		err = EKIT_SUSPENDED;
	} else 	if (len==0) {
		err = EKIT_OK; // Nothing to send, just success
	} else {
		int res;
		int ern;
		do {
			res = ioctl(i2c_descriptor, I2C_RDWR, &msgset);
			ern = errno;
		} while(res<0 && (ern==EINTR || ern==EAGAIN || ern==EWOULDBLOCK));
		if (res==1) {
			// must send 1 message
	    	err = EKIT_OK;
	    } else {
	    	err = readop ? EKIT_READ_FAILED : EKIT_WRITE_FAILED;
	    }
	}

    return err;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::read
// Purpose: Bus read operation
// uint8_t addr: bus address
// void* ptr: buffer to read data into. Buffer should be resized to appropriate amount of bytes to read
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::read(void* ptr, size_t len) {
	return i2c_read_write(address, true, ptr, len);	
}


//------------------------------------------------------------------------------------
// EKitI2CBus::read_all
// Purpose: Bus read operation
// uint8_t addr: bus address
// std::vector<uint8_t>& buffer: buffer to read data into. Buffer should be resized to appropriate amount of bytes to read
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::read_all(std::vector<uint8_t>& buffer){
    return EKIT_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::write
// Purpose: Bus write operation
// uint8_t addr: bus address
// const void* ptr: buffer to write data from. Function writes all bytes allocated in buffer
// Returns: corresponding EKIT_ERROR code
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::write(const void* ptr, size_t len) {
	return i2c_read_write(address, false, const_cast<void*>(ptr), len);
}

//------------------------------------------------------------------------------------
// EKitI2CBus::suspend
// Purpose: Suspend bus usage in the case bus is required to other processes
// Returns: corresponding EKIT_ERROR code
// Note: This is done by closing bus and reopening it later by resume() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::suspend() {
	LOCK(bus_lock);
	if (state==BUS_CLOSED)
		return EKIT_DISCONNECTED;

	if (state==BUS_PAUSED)
		return EKIT_SUSPENDED;

	::close(i2c_descriptor);
	state = BUS_PAUSED;

	return EKIT_OK;
}

//------------------------------------------------------------------------------------
// EKitI2CBus::resume
// Purpose: Resumes bus usage after suspend() call
// Returns: corresponding EKIT_ERROR code
// Note: This is done by reopening it after suspend() call
//------------------------------------------------------------------------------------
EKIT_ERROR EKitI2CBus::resume() {
	EKIT_ERROR err = EKIT_OK;

	LOCK(bus_lock);
	if (state==BUS_CLOSED)
		return EKIT_DISCONNECTED;

	if (state==BUS_OPENED)
		return EKIT_SUSPENDED;

	err = open_internal();
	if (err == EKIT_OK) {
		state = BUS_OPENED;
	}

	return err;
}

int EKitI2CBus::bus_props(int& busid) const {
	busid = BUS_I2C;
	return 0;
}