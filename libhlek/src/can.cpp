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
 *   \brief Can device software implementation
 *   \author Oleh Sharuda
 */

#include "can.hpp"
#include "ekit_firmware.hpp"
#include "texttools.hpp"

CanDev::CanDev(std::shared_ptr<EKitBus>& ebus, const CANConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
}

CanDev::~CanDev() {
}

void CanDev::can_start() {
    static const char* const func_name = "CanDev::can_start";

    // send command
    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, CAN_START, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Write data
        err = bus->write(nullptr, 0, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void CanDev::can_stop() {
    static const char* const func_name = "CanDev::can_stop";

    // send command
    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, CAN_STOP, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Write data
        err = bus->write(nullptr, 0, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void CanDev::can_filter_priv(CanFilterCommand filter) {
    static const char* const func_name = "CanDev::can_filter_priv";
    // send command
    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, CAN_FILTER, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Write data
        err = bus->write((uint8_t*)(&filter), sizeof(CanFilterCommand), to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void CanDev::can_filter_std(   bool enabled,
                        uint8_t index,
                        uint16_t msb_id,
                        uint16_t lsb_id,
                        uint16_t msb_id_mask,
                        uint16_t lsb_id_mask,
                        bool fifo1,
                        bool mask_mode) {
    static const char* const func_name = "CanDev::can_filter_std";

    if (index>CAN_FLT_MAX_INDEX) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "index is out of range");
    }

    // prepare data
    CanFilterCommand filter;
    filter.flags    = enabled ? CAN_FLT_ENABLE : 0;
    filter.flags   |= fifo1 ? CAN_FLT_FIFO : 0;
    filter.flags   |= mask_mode ? 0 : CAN_FLT_LIST_MODE;
    filter.flags   |= (index & CAN_FLT_INDEX_MASK);

    filter.id_msb   = msb_id << 5;
    filter.id_lsb   = lsb_id << 5;
    filter.mask_msb = msb_id_mask << 5;
    filter.mask_lsb = lsb_id_mask << 5;

    can_filter_priv(filter);
}

void CanDev::can_filter_std_32(    bool enabled,
                    uint8_t index,
                    uint32_t id,
                    uint32_t id_mask,
                    bool fifo1,
                    bool mask_mode) {
    static const char* const func_name = "CanDev::can_filter_std";

    if (index>CAN_FLT_MAX_INDEX) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "index is out of range");
    }

    // prepare data
    CanFilterCommand filter;
    filter.flags    = enabled ? CAN_FLT_ENABLE : 0;
    filter.flags   |= fifo1 ? CAN_FLT_FIFO : 0;
    filter.flags   |= CAN_FLT_SCALE;
    filter.flags   |= mask_mode ? 0 : CAN_FLT_LIST_MODE;
    filter.flags   |= (index & CAN_FLT_INDEX_MASK);

    filter.id_msb   = static_cast<uint16_t>(id) << 5;
    filter.id_lsb   = 0;
    filter.mask_msb = static_cast<uint16_t>(id_mask) << 5;
    filter.mask_lsb = 0;

    can_filter_priv(filter);
}

void CanDev::can_filter_ext(    bool enabled,
                                uint8_t index,
                                uint32_t id,
                                uint32_t id_mask,
                                bool fifo1,
                                bool mask_mode) {
    static const char* const func_name = "CanDev::can_filter_ext";

    if (index>CAN_FLT_MAX_INDEX) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "index is out of range");
    }

    // prepare data
    CanFilterCommand filter;
    filter.flags    = enabled ? CAN_FLT_ENABLE : 0;
    filter.flags   |= fifo1 ? CAN_FLT_FIFO : 0;
    filter.flags   |= CAN_FLT_SCALE;
    filter.flags   |= mask_mode ? 0 : CAN_FLT_LIST_MODE;
    filter.flags   |= (index & CAN_FLT_INDEX_MASK);

    filter.id_msb   = static_cast<uint16_t>(id >> 13);
    filter.id_lsb   = static_cast<uint16_t>(id << 3)  | 4;
    filter.mask_msb = static_cast<uint16_t>(id_mask >> 13);
    filter.mask_lsb = static_cast<uint16_t>(id_mask << 3)  | 4;

    can_filter_priv(filter);
}

void CanDev::can_send(uint32_t id, std::vector<uint8_t>& data, bool remote_frame, bool extended) {
    static const char* const func_name = "CanDev::can_send";
    size_t data_len = data.size();
    if (data_len>CAN_MSG_MAX_DATA_LEN) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "data length may not exceed 8 bytes");
    }
    size_t message_len = sizeof(CanSendCommand) + data_len;
    std::vector<uint8_t> buffer(message_len);
    CanSendCommand* message = (CanSendCommand*)buffer.data();

    // Check ids
    if (extended && (id >= (1 << 29))) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "extended id may not exceed 29 bits");
    } else if (!extended && (id >= (1 << 11))) {
        throw EKitException(func_name, EKIT_OUT_OF_RANGE, "standard id may not exceed 11 bits");
    }

    // send command
    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        EKIT_ERROR err = bus->set_opt(EKitFirmware::FIRMWARE_OPT_FLAGS, CAN_SEND, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "set_opt() failed");
        }

        // Fill data

        message->extra = static_cast<uint8_t>(data_len);

        // Handle remote frame
        if (remote_frame) {
            message->extra |= CAN_MSG_REMOTE_FRAME;
        }

        // Handle extended id
        if (extended) {
            message->extra |= CAN_MSG_EXTENDED_ID;
            message->ext_id = id;
            message->id = 0;
        } else {
            message->ext_id = 0;
            message->id = id;
        }

        // Copy data
        for (size_t i=0; i<data_len; i++) {
            message->data[i] = data[i];
        }

        // Write data
        err = bus->write((uint8_t*)(message), message_len, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "write() failed");
        }
    }
}

void CanDev::can_status_priv(CanStatus& status, EKitTimeout& to) {
    static const char* const func_name = "CanDev::can_status_priv";

    EKIT_ERROR err = bus->read((uint8_t*)(&status), sizeof(CanStatus), to);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "read() failed");
    }
}
std::map<uint16_t, std::pair<std::string, std::string>> CanDev::state_flag_map = {
        {CAN_STATE_STARTED, {"CAN STARTED: ", "CAN STOPPED: "}},
        {CAN_STATE_SLEEP, {"SLEEP", ""}},
        {CAN_STATE_MB_0_BUSY, {"[MB 0]", ""}},
        {CAN_STATE_MB_1_BUSY, {"[MB 1]", ""}},
        {CAN_STATE_MB_2_BUSY, {"[MB 2]", ""}},
        {CAN_ERROR_OVERFLOW, {"CIRC_OVF", ""}},
        {CAN_ERROR_FIFO_0_FULL, {"[FIFO 0 FULL]", ""}},
        {CAN_ERROR_FIFO_0_OVERFLOW, {"[FIFO 0 OVF]", ""}},
        {CAN_ERROR_FIFO_1_FULL, {"[FIFO 1 FULL]", ""}},
        {CAN_ERROR_FIFO_1_OVERFLOW, {"[FIFO 1 OVF]", ""}},
        {CAN_ERROR_WARNING, {"ERR_WARNING", ""}},
        {CAN_ERROR_PASSIVE, {"ERR_PASSIVE", ""}},
        {CAN_ERROR_BUS_OFF, {"BUS_OFF", ""}},
        {CAN_ERROR_NO_MAILBOX, {"[NO MAILBOX]", ""}}};

std::string CanDev::can_status_to_str(CanStatus& status) {
    std::string res = tools::flags_to_string(status.state, state_flag_map, " ");
    res += tools::str_format("data_len         = %d\n", status.data_len);
    std::string slec = can_last_err_to_str(status.last_error);
    res += tools::str_format("last_error       = %d => %s\n", status.last_error, slec.c_str());
    res += tools::str_format("recv_error_count = %d\n", status.recv_error_count);
    res += tools::str_format("lsb_trans_count  = %d\n", status.lsb_trans_count);

    return res;
}
std::string CanDev::can_msg_to_str(CanRecvMessage& msg) {
    std::string res;

    if (msg.extra & CAN_MSG_EXTENDED_ID) {
        res = tools::str_format("EXT: 0x%X: ", msg.id);
    } else {
        res = tools::str_format("STD: 0x%X: ", msg.id);
    }
    uint8_t n_bytes = std::min(msg.extra & CAN_MSG_MAX_DATA_LEN_MASK, CAN_MSG_MAX_DATA_LEN);

    for (size_t i=0; i<n_bytes; i++) {
        res += tools::str_format("0x%X ", msg.data[i]);
    }

    res += tools::str_format("   | fltid: %d", msg.fmi);

    if (msg.extra & CAN_MSG_REMOTE_FRAME) {
        res += " REMOTE";
    }

    return res;
}

std::string CanDev::can_last_err_to_str(uint8_t lec) {
    std::string res;

    if (lec & CAN_ESR_FLAG_WARNING) {
        res += "Warning ";
    }

    if (lec & CAN_ESR_FLAG_PASSIVE) {
        res += "Passive ";
    }

    if (lec & CAN_ESR_FLAG_BUSOFF) {
        res += "Bus-off ";
    }

    switch (lec & CAN_ESR_LEC_MASK) {
        case CAN_ESR_LEC_OK:
        break;

        case CAN_ESR_LEC_STUFF_ERR:
            res += "Stuff error;";
        break;

        case CAN_ESR_LEC_FORM_ERR:
            res += "Form error;";
        break;

        case CAN_ESR_LEC_ACK_ERR:
            res += "Acknowledgment error;";
        break;

        case CAN_ESR_LEC_REC_ERR:
            res += "Bit recessive error;";
        break;

        case CAN_ESR_LEC_DOM_ERR:
            res += "Bit dominant error;";
        break;

        case CAN_ESR_LEC_CRC_ERR:
            res += "CRC error;";
        break;

        case CAN_ESR_LEC_SFT_ERR:
            res += "Software error;";
        break;
    }

    return res;
}

void CanDev::can_status(CanStatus& status) {
    static const char* const func_name = "CanDev::can_status";
    EKitTimeout to(get_timeout());
    BusLocker blocker(bus, get_addr(), to);
    can_status_priv(status, to);
}

void CanDev::can_read(CanStatus& status, std::vector<CanRecvMessage>& messages) {
    static const char* const func_name = "CanDev::can_read";
    std::vector<uint8_t> data;
    size_t msg_count = 0;
    CanRecvMessage* pmsgs = nullptr;
    uint8_t* pdata = nullptr;

    {
        EKitTimeout to(get_timeout());
        BusLocker blocker(bus, get_addr(), to);

        can_status_priv(status, to);

        assert(status.data_len >= sizeof(CanStatus));
        assert(((status.data_len - sizeof(CanStatus)) % sizeof(CanRecvMessage))==0);
        msg_count = (status.data_len - sizeof(CanStatus)) / sizeof(CanRecvMessage);

        data.resize(status.data_len);
        EKIT_ERROR err = bus->read(data.data(), status.data_len, to);
        if (err != EKIT_OK) {
            throw EKitException(func_name, err, "read() failed");
        }
    }

    // Process data
    pdata = data.data();
    memcpy(&status, pdata, sizeof(CanStatus));
    messages.clear();
    pmsgs = (CanRecvMessage*)(pdata+sizeof(CanStatus));
    for (size_t i = 0; i<msg_count; i++) {
        messages.push_back(pmsgs[i]);
    }
}
