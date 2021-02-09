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
 *   \brief GSMModem software implementation
 *   \author Oleh Sharuda
 */

#include "gsmmodem.hpp"
#include "texttools.hpp"
#include "ekit_error.hpp"

constexpr char* GSMModem::at_status_name[];

const char* gsm_call_direction_name(GSM_CALL_DIRECTION v) {
    static const char* const func_name = "gsm_call_direction_name";
    static const char* const names[] = {"OUTGOING", "INCOMING"};
    if (v>=sizeof(names)) {
        throw EKitException(func_name, EKIT_BAD_PARAM);
    }

    return names[v];
}

const char* gsm_call_state_name(GSM_CALL_STATE v) {
    static const char* const func_name = "gsm_call_state_name";
    static const char* const names[] = {"ACTIVE", "HELD", "DIALING", "ALERTING", "INCOMING", "WAITING", "DISCONNECT"};
    if (v>=sizeof(names)) {
        throw EKitException(func_name, EKIT_BAD_PARAM);
    }

    return names[v];
}


const char* gsm_call_mode_name(size_t v) {
    static const char* const func_name = "gsm_call_mode_name";
    static const char* const names[] = {"VOICE", "DATA", "FAX"};
    if (v>=sizeof(names)) {
        throw EKitException(func_name, EKIT_BAD_PARAM);
    }

    return names[v];
}

const char* gsm_call_mpty_name(size_t v) {
    static const char* const func_name = "gsm_call_mpty_name";
    static const char* const names[] = {"SINGLEPARTY", "MULTIPARTY"};
    if (v>=sizeof(names)) {
        throw EKitException(func_name, EKIT_BAD_PARAM);
    }

    return names[v];
}

std::string GSMCallData::to_string() const {
    return tools::format_string("[%d %s %s] %s %s (%s)", idx,
                                            gsm_call_direction_name(direction),
                                            gsm_call_mode_name(mode),
                                            number,
                                            gsm_call_state_name(state),
                                            gsm_call_mpty_name(mpty) );
}

bool GSMCallData::is_valid() const {
    return direction <= GSM_CALL_DIRECTION_INCOMING &&
           state <= GSM_CALL_STATE_DISCONNECT &&
           mode <= GSM_CALL_MODE_FAX &&
           mpty <= GSM_CALL_MPTY_MULTI;
}

GSMModem::GSMModem(std::shared_ptr<EKitBus>& ebus, int addr, const std::string& name) : super(ebus, addr), modem_name(name) {
    re_ussd = tools::g_unicode_ts.regex_pattern("\\+CUSD:\\s?(\\d+)\\s?,\\s?\\\"([^\\\"]*)\\\"\\s?,\\s?(\\d+)", 0);
    assert(re_ussd);
    re_read_sms = tools::g_unicode_ts.regex_pattern("\\+CMGL:\\s*(\\d+)\\s*,\\s*\\\"([^\\\"\\d]+)\\\"\\s*,\\s*\\\"([a-fA-F\\d]+)\\\"\\s*,\\s*\\\"([^\\\"]*)\\\"\\s*,\\s*\\\"(\\S+)\\\"", 0);
    assert(re_read_sms);
    re_list_call = tools::g_unicode_ts.regex_pattern("\\+CLCC:\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*\\\"([^\\\"]+)\\\".*", 0);
    assert(re_list_call);
}

GSMModem::GSMModem(std::shared_ptr<EKitBus>& ebus, int addr, int timeout_ms, const std::string& name) : GSMModem(ebus, addr, name){
    configure(timeout_ms);
}

GSMModem::~GSMModem(){

}

std::string GSMModem::get_dev_name() const {
    return modem_name;
}

void GSMModem::set_error_mode(GSM_CMEE_MODE cmee, int timeout_ms, unsigned int& status_mask) {
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    BusLocker blocker(bus, get_addr());
    set_error_mode(cmee, sw, status_mask);
}

void GSMModem::set_error_mode(GSM_CMEE_MODE cmee, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::set_error_mode";
    if (cmee < GSM_CMEE_DISABLE || cmee > GSM_CMEE_TEXT) {
        throw EKitException(func_name, "Invalid cmee value");
    }

    std::string command = std::string("AT+CMEE=") + std::to_string(cmee);
    std::vector<std::string> lines;
    unsigned int status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    status_mask = 0;

    at(command, lines, sw, status);
    status_mask |= status;
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status_mask, "AT+CMEE didn't return successfully");
    }

    cmee_mode = cmee;
}

//------------------------------------------------------------------------------------
// GSMModem::at
// Purpose: Executes arbitrary AT command
// const std::string& cmd : [in] command, terminator is not required
// std::vector<std::string>& response: [out] vector of lines returned by modem
// int timeout_ms: [in] specifies timeout handling: timeout <=0  : wait indefinitely, otherwise
//              wait specified amount of milliseconds
// unsigned int& completion_status_mask: [in/out] on input specifies a bitmask of statuses (1 << AT_STATUS_XXXX) that should terminate command,
//                            on out specifies actuall statuses read in the input
// Notes: 1. Statuses are excluded from the response, this information put into completion_status_mask
//        2. All commands should be represented in ASCII, including those in UCS2 format
//------------------------------------------------------------------------------------
void GSMModem::at(const std::string& cmd, std::vector<std::string>& response, int timeout_ms, unsigned int &completion_status_mask) {
    static const char* const func_name = "GSMModem::at";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    BusLocker blocker(bus, get_addr());
    at(cmd, response, sw, completion_status_mask);
    if (completion_status_mask & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, completion_status_mask, "\"" + cmd + "\" command failed");
    }
}

void GSMModem::ussd(const std::string& ussd, std::string& result, int timeout_ms, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::ussd";
    EKIT_ERROR err;
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    std::string command = std::string("AT+CUSD=1,\"") + ussd + std::string("\"");
    std::vector<std::string> lines;
    unsigned int status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    status_mask = 0;
    bool output = false;
    std::string tmp;
    int dcs = 0;

    // acquire device
    BusLocker blocker(bus, get_addr());

    at("AT+CSCS=\"GSM\"", lines, sw, status);
    status_mask |= status;
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status_mask, "AT+CSCS=\"GSM\" didn't return successfully");
    }

    lines.clear();
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at(command, lines, sw, status);
    status_mask |= status;
    if (status_mask & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status_mask, "AT+CUSD didn't return successfully");
    }

    // Wait for CUSD message
    lines.clear();
    status = 0;
    unsigned int response_status = 0;
    err = wait_at_response("+CUSD:", lines, sw, status);
    status_mask |= status;
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "wait_at_response() failed");
    }

    for (auto l = lines.begin(); l!=lines.end(); ++l) {
        if (!tools::check_prefix(*l, "+CUSD:")) continue;

        std::vector<std::string> groups;
        if (!tools::g_unicode_ts.regex_groups(*re_ussd, *l, groups)) {
            throw EKitException(func_name, "unsupported output of +CUSD");
        }

        size_t dcs = std::atol(groups[3].c_str());
        if (dcs==15) {
            result = groups[2];
        } else 
        if (dcs==72) {
            result = UCS2_to_string(groups[2]);
        } else {
            throw EKitException(func_name, "bad response format: wrong <dcs> value");
        }

        break;
    }
}
void GSMModem::sms(const std::string& number, const std::string& text, int timeout_ms, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::sms";

    bool ascii = tools::g_unicode_ts.is_ascii(text);
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    std::vector<std::string> lines;
    unsigned int status=0;
    status_mask = 0;

    std::string at_cmgs = "AT+CMGS=\"";
    std::string at_text;

    if (!ascii) {
        // Unicode encoding
        at_cmgs += string_to_UCS2(number);
        at_text = string_to_UCS2(text);
    } else {
        // ASCII
        at_cmgs += number;
        at_text = text;
    }
    at_cmgs+="\"";
    at_text+="\x1A";

    // acquire device
    BusLocker blocker(bus, get_addr());

    if (ascii!=sms_ascii_mode) {
        status = GSMModem::AT_STATUS_PROMPT | GSMModem::AT_STATUS_ERROR;        
        configure_sms(ascii, sw, status);
        if (status & GSMModem::AT_STATUS_ERROR) {
            throw_at_error(func_name, status, "configure_sms() failed");
        }
    }    

    status = GSMModem::AT_STATUS_PROMPT | GSMModem::AT_STATUS_ERROR;
    lines.clear();
    at(at_cmgs, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CMGS number failed");
    }
    status_mask |= status;

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    lines.clear();
    at(at_text, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CMGS text failed");
    }
    status_mask |= status;
}

void GSMModem::read_sms(std::vector<GSMSmsData>& messages, int timeout_ms, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::read_sms";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    unsigned int status = 0;
    std::vector<std::string> lines;
    status_mask = 0;

    // acquire device
    BusLocker blocker(bus, get_addr());

    // Configure if required
    if (sms_ascii_mode!=false) {
        status = GSMModem::AT_STATUS_PROMPT | GSMModem::AT_STATUS_ERROR;
        configure_sms(false, sw, status);
        if (status & GSMModem::AT_STATUS_ERROR) {
            throw_at_error(func_name, status, "configure_sms() failed");
        }
    }
    status_mask |= status;

    // Read all messages
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at("AT+CMGL=\"ALL\"", lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CMGL failed");
    }
    status_mask |= status;

    // Parse them
    // +CMGL: 59,"REC READ","002B","","20/08/06,16:29:57+12"
    // 0406
    messages.clear();
    size_t n_lines = lines.size();
    for (size_t i=0; i<n_lines; i++) {
        std::vector<std::string> groups;
        GSMSmsData sms;

        // parse sms header
        bool res = tools::g_unicode_ts.regex_groups(*re_read_sms, lines[i], groups);
        if (!res) continue;

        sms.id = std::atol(groups[1].c_str());
        sms.phone_number = UCS2_to_string(groups[3]);
        sms.status = groups[2];
        sms.timestamp = groups[5];

        i++;
        if (i<n_lines) {
            sms.message = UCS2_to_string(lines[i]);
            messages.push_back(sms);
        }
    }
}

void GSMModem::delete_sms(int id, int timeout_ms, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::read_sms";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    unsigned int status = 0;
    std::vector<std::string> lines;
    status_mask = 0;

        // acquire device
    BusLocker blocker(bus, get_addr());

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;

    if (id < 0) {
        at("AT+CMGDA=\"DEL ALL\"", lines, sw, status);
        if (status & GSMModem::AT_STATUS_ERROR) {
            throw_at_error(func_name, status, "AT+CMGDA=\"DEL ALL\" failed");
        }
    } else {
        std::string del_msg = tools::format_string("AT+CMGD=%d,0", id);
        at(del_msg, lines, sw, status);
        if (status & GSMModem::AT_STATUS_ERROR) {
            throw_at_error(func_name, status, "AT+CMGD failed");
        }        
    }
    status_mask |= status;
}

void GSMModem::active_calls(std::vector<GSMCallData>& active_calls, int timeout_ms, unsigned int& status_mask){
    static const char* const func_name = "GSMModem::active_calls";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    unsigned int status = 0;
    std::vector<std::string> lines;
    status_mask = 0;

    // acquire device
    BusLocker blocker(bus, get_addr());

    // Read all messages
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at("AT+CLCC", lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CLCC failed");
    }
    status_mask |= status;

    // Parse them
    // +CLCC: 1,1,4,0,0,"+38<number>",145,""
    // 0406
    active_calls.clear();
    size_t n_lines = lines.size();
    for (size_t i=0; i<n_lines; i++) {
        std::vector<std::string> groups;
        GSMCallData act_call;

        // parse sms header
        bool res = tools::g_unicode_ts.regex_groups(*re_list_call, lines[i], groups);
        if (!res) continue;

        act_call.idx = std::atol(groups[1].c_str());
        act_call.direction = static_cast<GSM_CALL_DIRECTION>(std::atol(groups[2].c_str()));
        act_call.state = static_cast<GSM_CALL_STATE>(std::atol(groups[3].c_str()));
        act_call.mode = static_cast<GSM_CALL_MODE>(std::atol(groups[4].c_str()));
        act_call.mpty = static_cast<GSM_CALL_MPTY>(std::atol(groups[5].c_str()));
        act_call.number = groups[6];

        assert(act_call.is_valid());
        active_calls.push_back(act_call);
    }
}
void GSMModem::dial(const std::string& number, int timeout_ms, unsigned int& status_mask){
    static const char* const func_name = "GSMModem::dial";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    unsigned int status = 0;
    std::vector<std::string> lines;
    status_mask = 0;

    // acquire device
    BusLocker blocker(bus, get_addr());

    // Read all messages
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    std::string atd = tools::format_string("ATD %s;", number);
    at(atd, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "ATD failed");
    }
    status_mask |= status;

}
void GSMModem::answer(GSM_CALL_ACTION action, int timeout_ms, unsigned int& status_mask) {
    static const char* const func_name = "GSMModem::answer";
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms);
    unsigned int status = 0;
    std::vector<std::string> lines;
    status_mask = 0;

    // acquire device
    BusLocker blocker(bus, get_addr());

    // Read all messages
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    std::string at_cmd;
    switch (action) {
        case GSM_CALL_ACTION_ANSWER:
            at_cmd = "ATA";
        break;

        case GSM_CALL_ACTION_HANG:
            at_cmd = "ATH";
        break;

        case GSM_CALL_ACTION_HOLD:
            at_cmd = "AT+CHLD=2";
        break;

        case GSM_CALL_ACTION_RELEASE:
            at_cmd = "AT+CHLD=1";
        break;

        default:
            throw EKitException(func_name, status, "wrong action passed");
    }

    at(at_cmd, lines, sw, status);
    status_mask |= status;
}

bool GSMModem::is_terminator(char c) noexcept {
    return c=='\r' || c=='\n';
}

std::string GSMModem::status_description(unsigned int status_mask) {
    std::string res;
    bool empty = true;
    for (size_t i=0; i<sizeof(at_status_name)/sizeof(const char*); ++i){
        if ((status_mask & (1 << i)) != 0) {
            if (!empty) res.append(",");
            res.append(at_status_name[i]);
            empty = false;
        }
    }
    return res;
}

//------------------------------------------------------------------------------------
// GSMModem::UCS2_to_string
// Purpose: Converts UCS2 hex string into std::string
// const std::string& hex : input string
// Returns: converted string
//------------------------------------------------------------------------------------
std::string GSMModem::UCS2_to_string(const std::string& hex) const {
    static const char* const func_name = "GSMModem::UCS2_to_string";
    std::string result;
    std::vector<uint8_t> buffer = tools::buffer_from_hex(hex);
    std::basic_string<char16_t> utf16((const char16_t*)buffer.data(), buffer.size()/sizeof(char16_t));
    if (!tools::g_unicode_ts.utf16_to_utf8(utf16, result, false)) {
        throw EKitException(func_name, "unicode conversion failed");
    }
    return result;
}

//------------------------------------------------------------------------------------
// GSMModem::string_to_UCS2
// Purpose: Converts string into UCS2 hex representation
// const std::string& s : input string
// Returns: hex representation of the converted string
//------------------------------------------------------------------------------------
std::string GSMModem::string_to_UCS2(const std::string& s) const {
    static const char* const func_name = "GSMModem::string_to_UCS2";
    std::string result;
    std::basic_string<char16_t> utf16;

    if (!tools::g_unicode_ts.utf8_to_utf16(s, utf16, false)) {
        throw EKitException(func_name, "unicode conversion failed");
    }

    result = tools::buffer_to_hex((const uint8_t*)utf16.c_str(), utf16.length()*sizeof(char16_t), false, "");
    return result;
}

//------------------------------------------------------------------------------------
// GSMModem::at
// Purpose: Executes arbitrary AT command (protected, for internal use in sequance of AT commands)
// const std::string& cmd : [in] command, terminator is not required
// std::vector<std::string>& response: [out] vector of lines returned by modem
// tools::StopWatch<std::chrono::milliseconds>& sw : stop watch object used to calculate timeouts
// unsigned int& completion_status_mask: [in/out] on input specifies a bitmask of statuses (1 << AT_STATUS_XXXX) that should terminate command,
//                            on out specifies actuall statuses read in the input
// Notes: 1. Statuses are excluded from the response, this information put into completion_status_mask
//        2. All commands should be represented in ASCII, including those in UCS2 format
//------------------------------------------------------------------------------------
void GSMModem::at(const std::string& cmd, std::vector<std::string>& response, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int &completion_status_mask) {
    static const char* const func_name = "GSMModem::at";
    EKIT_ERROR err;

    // Prepare command
    std::string at_command = cmd + at_terminator;
    const uint8_t* ptr = (const uint8_t*)at_command.c_str();
    size_t at_command_len = at_command.length();
    std::vector<uint8_t> buffer = std::vector<uint8_t>(ptr, ptr+at_command_len);
    response.clear();

    // Send command
    err = bus->write(buffer);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "write() failed");
    }

    // Read output until status found
    err = wait_at_status(response, sw, completion_status_mask);
    if (err != EKIT_OK) {
        throw EKitException(func_name, err, "wait_at_status() failed");
    }
}

void GSMModem::configure_sms(bool ascii, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask) {
    // Configure SMS messages sent/receive
    static const char* const func_name = "GSMModem::configure_sms";

    std::vector<std::string> lines;
    unsigned int status;
    status_mask = 0;

    std::string at_cmgf = "AT+CMGF=1";  // Text mode
    std::string at_csmp;
    std::string at_cscs;

    if (!ascii) {
        // Unicode encoding
        at_csmp = "AT+CSMP=17,167,2,25";
        at_cscs = "AT+CSCS=\"UCS2\"";
    } else {
        // ASCII
        at_csmp = "AT+CSMP=17,167,0,0";
        at_cscs = "AT+CSCS=\"GSM\"";
    }

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at(at_cmgf, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CMGF failed" );
    }
    status_mask |= status;

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    lines.clear();
    at(at_csmp, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CSMP failed");
    }
    status_mask |= status;

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    lines.clear();
    at(at_cscs, lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "AT+CSCS failed");
    }
    status_mask |= status;

    // update GSM modem ascii mode
    sms_ascii_mode = ascii;
}

void GSMModem::configure(int timeout_ms) {
    tools::StopWatch<std::chrono::milliseconds> sw(timeout_ms); // Do not use timeout
    static const char* const func_name = "GSMModem::configure";
    unsigned int status = 0;
    std::vector<std::string> lines;

    BusLocker blocker(bus, get_addr());

    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;

    // Issue AT command until success
    do {
        status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
        at("AT", lines, sw, status);
    } while ((status & GSMModem::AT_STATUS_OK)==0);

    // setup line terminator
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at("ATS3=13", lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "ATS3=13 failed");
    }

    // setup default echo mode
    status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    at("ATE0", lines, sw, status);
    if (status & GSMModem::AT_STATUS_ERROR) {
        throw_at_error(func_name, status, "ATE0 failed");
    }

    set_error_mode(cmee_mode, sw, status);

    configure_sms(false, sw, status);
}

EKIT_ERROR GSMModem::read_lines(std::vector<std::string>& lines, tools::StopWatch<std::chrono::milliseconds>& sw) {
    const size_t pooling_wait_ms = 10;
    EKIT_ERROR err;
    std::vector<uint8_t> data;
    size_t datalen = 0;
    bool non_terminator = false;
    bool line_terminated = false;
    size_t size = 0;
    size_t prev_size = 0;
    std::vector<uint8_t> buffer;

    // Phase one: read at least something by line separator
    do {
        buffer.clear();
        err = bus->read_all(buffer);
        if (err != EKIT_OK &&
            err != EKIT_READ_FAILED &&
            err != EKIT_WRITE_FAILED &&
            err != EKIT_SUSPENDED)
        {
            // something unexpected happened on the bus, otherwise ignore
            break;
        }

        // Did we get any non-null terminator?
        size = buffer.size();
        if (!non_terminator) {
            non_terminator = std::find_if_not(  buffer.begin(), 
                                        buffer.end(), 
                                        is_terminator)!=buffer.end();
        }

        // Is there any non terminator character and the last character is terminator? If so, we may stop reading from device
        line_terminated = size>0 && is_terminator(buffer[size-1]);

        if (size>0) {
            //data = tools::append_vector(data, buffer);
            tools::join_containers(data, buffer);
            datalen+=size;
        }

        // special case: take care of prompt, it allways start from the begin of the line and modem doesn't send anything after prompt, it expects some data
        constexpr size_t prompt_len = tools::const_strlen(at_status_name[AT_PROMPT]);
        if (datalen>=prompt_len) {
            const char* pdata = (const char*)data.data();
            if (std::memcmp(pdata + datalen - prompt_len, at_status_name[AT_PROMPT], prompt_len)==0) {
                line_terminated = datalen==prompt_len || (datalen>prompt_len && is_terminator(pdata[datalen - prompt_len - 1]));
            }
        }

        if (sw.expired()) {
            err = EKIT_TIMEOUT;
            break;
        }

        tools::sleep_ms(pooling_wait_ms); // it is hardware modem - slow device, we shouldn't consume CPU to much here -> sleep
    } while (!(non_terminator && line_terminated));

    // Phase two: split data and trim lines
    std::string text( (const char*)data.data(), datalen);
    lines = tools::split_and_trim(  text, 
                                    [](char c){return is_terminator((uint8_t)c);},
                                    [](char c){return is_terminator((uint8_t)c);});

    return err;
}

EKIT_ERROR GSMModem::wait_at_status(std::vector<std::string>& result, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& completion_status_mask) {
    EKIT_ERROR err = EKIT_OK;
    unsigned int stop_status = completion_status_mask;
    completion_status_mask = 0;
    std::vector<std::string> lines;

    do {
        if (sw.expired()) {
            err = EKIT_TIMEOUT;
            break;
        }

        err = read_lines(lines, sw);
        if (err != EKIT_OK) {
            break;
        }

        // Check for AT status in all lines, if status, mark it in status mask, otherwise append to result
        for (std::vector<std::string>::const_iterator i=lines.begin(); i!=lines.end(); ++i) {
            unsigned int status = get_status(*i);
            if (status==0) {
                result.push_back(*i);
            } else {
                completion_status_mask|=status;
            }
        }
    } while ((completion_status_mask & stop_status) == 0);

    return err;
}

EKIT_ERROR GSMModem::wait_at_response(const std::string& prefix, std::vector<std::string>& result, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask) {
    EKIT_ERROR err = EKIT_OK;
    status_mask = 0;
    std::vector<std::string> lines;
    bool done = false;

    do {
        if (sw.expired()) {
            err = EKIT_TIMEOUT;
            break;
        }

        err = read_lines(lines, sw);
        if (err != EKIT_OK) {
            break;
        }

        // Check for AT status all the lines, if status mark it in status mask, otherwise append to result
        for (std::vector<std::string>::const_iterator i=lines.begin(); i!=lines.end(); ++i) {
            unsigned int status = get_status(*i);
            if (status==0) {
                result.push_back(*i);
                done = tools::check_prefix(*i, prefix);
            } else {
                status_mask|=status;
            }
        }
    } while (!done);

    return err;
}

unsigned int GSMModem::get_status(const std::string& line) {
    for (size_t i=0; i<sizeof(at_status_name)/sizeof(const char*); ++i) {
        if (    i==AT_ERROR &&
                cmee_mode!=GSM_CMEE_DISABLE &&
                tools::check_prefix(line, cmee_error_header)) {
                size_t pref_len = tools::const_strlen(cmee_error_header);
                last_cmee_error = line.substr(pref_len);
                return AT_STATUS_ERROR;
        } else if (line.compare(at_status_name[i])==0) {
            return 1<<i;
        }
    }

    return 0;
}

void GSMModem::throw_at_error(const char* func_name, unsigned  int status, const std::string& description) {
    if (cmee_mode==GSM_CMEE_DISABLE) {
        throw EKitException(func_name, status, description);
    } else {
        std::string cmee_error = description + " (CMEE: " + last_cmee_error + ")";
        throw EKitException(func_name, status, cmee_error);
    }
}
