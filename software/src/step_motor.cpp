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
 *   \brief StepMotorDev software implementation
 *   \author Oleh Sharuda
 */

#include "step_motor.hpp"
#include <climits>
#include "ekit_firmware.hpp"

#ifdef STEP_MOTOR_DEVICE_ENABLED

SW_STEP_MOTOR_MOTOR_DESCRIPTORS
SW_STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS

const StepMotorDevice g_step_motor_devices[] = SW_STEP_MOTOR_DEVICE_DESCRIPTORS;
const StepMotorMicrostepTables g_step_motor_microstep_tables = STEP_MOTOR_MICROSTEP_TABLE;

StepMotorDev::StepMotorDev(std::shared_ptr<EKitBus>& ebus, int addr) : super(ebus, addr) {
	static const char* const func_name = "StepMotorDev::StepMotorDev";
	for (int i=0; i<SW_STEP_MOTOR_DEVICE_COUNT; i++) {
		if (addr==g_step_motor_devices[i].dev_id) {
			descr = const_cast<const StepMotorDevice*>(g_step_motor_devices + i);
			break;
		}
	}

	if (descr==nullptr) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "addr specified doesn't correspond to any of step motor devices");
	}

	clear();
}

StepMotorDev::~StepMotorDev() {
}

const PStepMotorDevice StepMotorDev::get_descriptor(size_t index) {
	if (index<SW_STEP_MOTOR_DEVICE_COUNT) {
		return const_cast<PStepMotorDevice>(g_step_motor_devices + index);
	} else {
		assert(false);
	}

	return nullptr;
}

std::string StepMotorDev::get_dev_name() const {
	return descr->dev_name;
}

size_t StepMotorDev::get_motor_count() const {
	return descr->motor_count;
}

std::vector<const StepMotorDescriptor*> StepMotorDev::get_motor_info() const {
	std::vector<const StepMotorDescriptor*> res;
	res.resize(descr->motor_count);
	for (size_t mindex=0; mindex<descr->motor_count; mindex++) {
		res[mindex] = descr->motor_descriptor[mindex];
	}
	return res;
}

uint64_t StepMotorDev::reset(size_t mindex) {
	enque_cmd(mindex, STEP_MOTOR_GENERAL, STEP_MOTOR_GENERAL_RESET, 0);
	return 0;
}

uint64_t StepMotorDev::wait(size_t mindex, double value_sec) {
	uint64_t us = double_to_us(value_sec);
	enque_cmd(mindex, STEP_MOTOR_GENERAL, STEP_MOTOR_GENERAL_WAIT, us);
	return us;
}

uint64_t StepMotorDev::enable(size_t mindex, bool on) {
	enque_cmd(mindex, STEP_MOTOR_GENERAL, on ? STEP_MOTOR_GENERAL_ENABLE : STEP_MOTOR_GENERAL_DISABLE, 0);
	return 0;
}

uint64_t StepMotorDev::sleep(size_t mindex, bool sleep) {
	enque_cmd(mindex, STEP_MOTOR_GENERAL, sleep ? STEP_MOTOR_GENERAL_SLEEP : STEP_MOTOR_GENERAL_WAKEUP, 0);
	return 0;
}

uint64_t StepMotorDev::configure(size_t mindex, uint32_t flags) {
    enque_cmd(mindex, STEP_MOTOR_GENERAL, STEP_MOTOR_GENERAL_CONFIG, STEP_MOTOR_CONFIG_TO_BYTE(flags));
    return 0;
}

uint64_t StepMotorDev::set_software_endstop(size_t mindex, bool cw, int64_t limit) {
    enque_cmd(mindex, STEP_MOTOR_SET, cw ? STEP_MOTOR_SET_CW_SFT_LIMIT : STEP_MOTOR_SET_CCW_SFT_LIMIT, limit);
    return 0;
}

uint64_t StepMotorDev::dir(size_t mindex, bool cw) {
	enque_cmd(mindex, STEP_MOTOR_SET, cw ? STEP_MOTOR_SET_DIR_CW : STEP_MOTOR_SET_DIR_CCW, 0);
	return 0;
}

uint8_t StepMotorDev::microstep_divider(size_t mindex) {
    uint8_t driver_type = descr->motor_descriptor[mindex]->motor_driver;
    uint8_t mstep = motors_data[mindex].microstep;
    uint8_t m1 = mstep & 1;
    uint8_t m2 = (mstep & 2) >> 1;
    uint8_t m3 = (mstep & 4) >> 2;

    uint8_t ms_shift = STEP_MOTOR_MICROSTEP_VALUE(g_step_motor_microstep_tables[driver_type], m1, m2, m3);
    assert(ms_shift!=STEP_MOTOR_BAD_STEP);
    return STEP_MOTOR_MICROSTEP_DIVIDER(ms_shift);
}

uint64_t StepMotorDev::speed(size_t mindex, double value, bool rpm) {
	static const char* const func_name = "StepMotorDev::speed";
    if (mindex>=get_motor_count()) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "mindex is higher than allowed.");
    }

	if (rpm) {
        if (value == 0.0) {
            throw EKitException(func_name, EKIT_BAD_PARAM, "Number of revolutions per minute can't be 0");
        }

        double spr = (double)(descr->motor_descriptor[mindex]->steps_per_revolution);
        double divider = (double)(microstep_divider(mindex));
        value = (60.0 / (value * spr * divider));
	}	

	uint64_t us = double_to_us(value);
	if (us < STEP_MOTOR_MIN_STEP_WAIT) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "Step duration must not be shorter than STEP_MOTOR_MIN_STEP_WAIT");
	}
    motors_data[mindex].speed = us;
	// we are setting time duration between STEP pulses, therefore step timing must be accounted
    enque_cmd(mindex, STEP_MOTOR_SET, STEP_MOTOR_SET_STEP_WAIT, us);

	return 0;
}

uint64_t StepMotorDev::move(size_t mindex) {
    enque_cmd(mindex, STEP_MOTOR_MOVE_NON_STOP, 0, 0);
	return ULLONG_MAX;
}

uint64_t StepMotorDev::move(size_t mindex, uint64_t n_steps) {
    enque_cmd(mindex, STEP_MOTOR_MOVE, 0, n_steps);
    return n_steps*motors_data[mindex].speed;
}

uint64_t StepMotorDev::microstep(size_t mindex, bool m1, bool m2, bool m3) {
    static const char* const func_name = "StepMotorDev::microstep";
    uint8_t ms = (uint8_t)m1 + ((uint8_t)m2 << 1) + ((uint8_t)m3 << 2);
    uint8_t driver_type = descr->motor_descriptor[mindex]->motor_driver;
    uint8_t val = STEP_MOTOR_MICROSTEP_VALUE(g_step_motor_microstep_tables[driver_type], m1, m2, m3);

    if (val==STEP_MOTOR_BAD_STEP) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "This driver doesn't support specified microstep value");
    }

    motors_data[mindex].microstep = ms;
    enque_cmd(mindex, STEP_MOTOR_SET, STEP_MOTOR_SET_MICROSTEP, ms);
    return 0;
}

uint8_t StepMotorDev::status(std::vector<StepMotorStatus>& mstatus) {
	static const char* const func_name = "StepMotorDev::status";
	EKIT_ERROR err;
    size_t bufsize = descr->motor_count*sizeof(StepMotorStatus) + sizeof(StepMotorDevStatus);
    std::vector<uint8_t> data(bufsize);
    StepMotorDevStatus* pstatus = reinterpret_cast<StepMotorDevStatus*>(data.data());

    BusLocker blocker(bus, get_addr());

    err = bus->read(data);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }

    mstatus.resize(descr->motor_count);
    for (size_t i=0; i<descr->motor_count;i++) {
        mstatus[i] = pstatus->mstatus[i];
    }

    return pstatus->status;
}

void StepMotorDev::start() {
    static const char* const func_name = "StepMotorDev::start";
    BusLocker blocker(bus, get_addr());

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, STEP_MOTOR_START);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write({});
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }
}

void StepMotorDev::stop() {
	static const char* const func_name = "StepMotorDev::stop";
    BusLocker blocker(bus, get_addr());

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, STEP_MOTOR_STOP);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write({});
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }

    clear();
}

void StepMotorDev::feed() {
    static const char* const func_name = "StepMotorDev::feed";
    size_t mcount = get_motor_count();
    std::vector<uint8_t> data;
    uint8_t swcmd;

    // Form a buffer
    for (size_t mindex=0; mindex<mcount; mindex++){
        std::vector<uint8_t>& mbuffer = motors_data[mindex].buffer;

        if (mbuffer.empty()) continue;

        // push command to switch motor
        swcmd = STEP_MOTOR_SELECT | ((~STEP_MOTOR_SELECT) & ((uint8_t)mindex));
        data.push_back(swcmd);

        tools::join_containers(data, mbuffer);
    }

    // Send data
    BusLocker blocker(bus, get_addr());

    EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, STEP_MOTOR_NONE);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "set_opt() failed");
    }

    err = bus->write(data);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }

    // Data sent, drop all motors buffers
    for (size_t mindex=0; mindex<mcount; mindex++){
        motors_data[mindex].buffer.clear();
    }
}

void StepMotorDev::clear() {
    static const char* const func_name = "StepMotorDev::clear";
	size_t mcount = get_motor_count();

	motors_data.clear();
	motors_data.resize(mcount);

	for (size_t i=0; i<mcount; i++) {
		motors_data[i].speed = descr->motor_descriptor[i]->default_speed;
		motors_data[i].microstep = STEP_MOTOR_MICROSTEP_STATUS_TO_VALUE(descr->motor_descriptor[i]->config_flags);
	}
}

void StepMotorDev::enque_param(std::vector<uint8_t>& mbuffer, uint64_t param, size_t len) {
	assert(len <= sizeof(param));
	uint8_t* p = reinterpret_cast<uint8_t*>(&param);

	// copy in little endian order
	for (size_t b = 0; b<len; b++, p++) {
		mbuffer.push_back(*p);
	}
}

void StepMotorDev::enque_cmd(size_t mindex, uint8_t cmd, uint8_t subcmd, uint64_t param) {
	static const char* const func_name = "StepMotorDev::enque_cmd";
	assert((cmd & STEP_MOTOR_CMD_MASK)==cmd);
    assert((subcmd & STEP_MOTOR_ARG_MASK)==subcmd);
	bool set_cmd = (cmd == STEP_MOTOR_SET);
    subcmd &= STEP_MOTOR_ARG_MASK;
    cmd |= subcmd;

    if (mindex>=get_motor_count()) {
        throw EKitException(func_name, EKIT_BAD_PARAM, "mindex is higher than allowed.");
    }

	bool use_motor_command_byte_argument = false;

	std::vector<uint8_t>& mbuffer = motors_data.at(mindex).buffer;
	mbuffer.push_back(cmd);
	size_t cmd_index = mbuffer.size()-1;

    switch (cmd) {
        case STEP_MOTOR_SET | STEP_MOTOR_SET_MICROSTEP:
        case STEP_MOTOR_SET | STEP_MOTOR_SET_STEP_WAIT:
        case STEP_MOTOR_SET | STEP_MOTOR_SET_CW_SFT_LIMIT:
        case STEP_MOTOR_SET | STEP_MOTOR_SET_CCW_SFT_LIMIT:
        case STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_CONFIG:
        case STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_WAIT:
            // requires argument: do not use argument part of the motor command byte
        break;

        case STEP_MOTOR_MOVE:
            // requires argument: it is possible to use argument part of the motor command byte
            use_motor_command_byte_argument = true;
        break;

        default:
            mbuffer[cmd_index] |= STEP_MOTOR_PARAM_NONE;
            goto done;
    };

	if (use_motor_command_byte_argument && (param <= STEP_MOTOR_ARG_MASK)) {
        mbuffer[cmd_index] &= ~STEP_MOTOR_ARG_MASK;
        mbuffer[cmd_index] |= STEP_MOTOR_PARAM_NONE | (uint8_t)param;
	} else
    if (param <= UCHAR_MAX) {
    	mbuffer[cmd_index] |= STEP_MOTOR_PARAM_8;
    	enque_param(mbuffer, param, sizeof(uint8_t));
    } else if (param <= USHRT_MAX) {
		mbuffer[cmd_index] |= STEP_MOTOR_PARAM_16;
		enque_param(mbuffer, param, sizeof(uint16_t));
	} else {
		mbuffer[cmd_index] |= STEP_MOTOR_PARAM_64;
		enque_param(mbuffer, param, sizeof(uint64_t));		
	}

done:
    return;
}

uint64_t StepMotorDev::double_to_us(double v) const {
	static const char* const func_name = "StepMotorDev::double_to_us";
	v = v*1.0e6;
	if (v<0 || v>(double)ULLONG_MAX) {
		throw EKitException(func_name, EKIT_BAD_PARAM, "Time period can't be negative or longer than ULLONG_MAX micro seconds.");
	}

	return (uint64_t)v;
}

#endif
