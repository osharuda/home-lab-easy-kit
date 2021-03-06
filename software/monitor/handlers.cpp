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
 *   \brief Firmware to software communication test utility implementation
 *   \author Oleh Sharuda
 */

#include <memory>
#include <climits>
#include "ekit_error.hpp"
#include "handlers.hpp"
#include "monitor.hpp"

//----------------------------------------------------------------------------------------------//
//                                    CommandHandlerException                                   //
//----------------------------------------------------------------------------------------------//
CommandHandlerException::CommandHandlerException(const std::string& description) : std::runtime_error(description){
}

CommandHandlerException::~CommandHandlerException() {
}

//----------------------------------------------------------------------------------------------//
//                                    CommandHandler                                            //
//----------------------------------------------------------------------------------------------//
CommandHandler::CommandHandler(std::shared_ptr<EKitDeviceBase> dev, std::shared_ptr<MonitorUI> _ui) : device(std::move(dev)), ui(std::move(_ui)) {
    arg_reset();
}

CommandHandler::~CommandHandler() {
}

std::string CommandHandler::help() const {
    return tools::format_string("Help is not provided for %s command",get_command_name());
}

void CommandHandler::arg_reset(){
    arg_index = 1;
}

size_t CommandHandler::check_arg_count(const std::vector<std::string>& args, size_t expected) const {
    size_t argc = args.size();
    if (expected+1 == argc) {
        return expected;
    }

    std::string err = tools::format_string("Wrong number of arguments given the for command %s",get_command_name());
    throw CommandHandlerException(err);    
}

size_t CommandHandler::check_arg_count_min(const std::vector<std::string>& args, size_t min) const {
    size_t argc = args.size();
    if (min+1 <= argc) {
        return argc-1;
    }

    std::string err = tools::format_string("Too few arguments given the for command %s",get_command_name());
    throw CommandHandlerException(err);    
}

std::string CommandHandler::arg_get(const std::vector<std::string>& args, const char* name) {
    std::string res;
    try {
        res = args.at(arg_index++);
    } catch (std::out_of_range& e) {
        std::string err = tools::format_string("Failed to read argument %s (out if range index). It is likely wrong number of arguments given the for command.", name);
        throw CommandHandlerException(err);
    }
    return res;
}

bool CommandHandler::arg_boolean(const std::vector<std::string>& args, const char* name, const std::list<std::string>& truevals, const std::list<std::string>& falsevals) {
    std::string v = arg_get(args, name);
    std::list<std::string> allowed_args;

    for (auto t=truevals.begin(); t!=truevals.end(); ++t) {
        if (*t==v) {
            return true;
        }

        allowed_args.push_back(*t);
    }

    for (auto f=falsevals.begin(); f!=falsevals.end(); ++f) {
        if (*f==v)
            return false;

        allowed_args.push_back(*f);
    }    

    std::string err = tools::format_string("Invalid argument specified for %s (valid values are: %s)", name, tools::join_strings(allowed_args, ", "));
    throw CommandHandlerException(err);
}

void CommandHandler::arg_options_check(const std::set<std::string> &opts, const std::set<std::string>& allowed_opts) {
    for (auto o=opts.begin(); o!=opts.end(); ++o) {
        if (allowed_opts.find(*o)==allowed_opts.end()) {
            std::string err = tools::format_string("Invalid option specified: %s", *o);
            throw CommandHandlerException(err);
        }
    }
}

std::string CommandHandler::arg_unit(const std::vector<std::string>& args, const char* name, const std::list<std::string>&allowed_units, std::string& unit) {
    std::string v = arg_get(args, name);
    size_t vlen = v.length();
    bool unit_found = false;

    // check unit first
    for (auto u=allowed_units.begin(); u!=allowed_units.end(); ++u) {
        size_t ulen = u->length();
        if (ulen<vlen && v.compare(vlen-ulen, ulen, *u)==0) {
            v.resize(vlen-ulen);
            unit = *u;
            unit_found = true;
            break;
        }
    } 

    if (!unit_found) {
        std::string err = tools::format_string("Unit is not specified for %s (valid units are: %s)", name, tools::join_strings(allowed_units, ", "));
        throw CommandHandlerException(err);
    }

    return v;
}

double CommandHandler::arg_time_to_sec(double val, const std::string& unit) {
    if (unit=="us") {
        return val * 1.0e-6;
    } else
    if (unit=="ms") {
        return val * 1.0e-3;
    } else
    if (unit=="s") {
        return val;
    } else
    if (unit=="min") {
        return val * 0.6e2;
    } else
    if (unit=="hr") {
        return val * 0.36e4;
    } else
    if (unit=="day") {
        return val * 0.864e5;
    }

    throw CommandHandlerException(tools::format_string("Unknown unit for time %s", unit));
}

double CommandHandler::arg_double(  const std::vector<std::string>& args, 
                                    const char* name, 
                                    double min_val, 
                                    double max_val, 
                                    const std::list<std::string>&allowed_units, 
                                    std::string& unit, 
                                    const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    double res;
    std::string err;

    // convert to double
    try {
        res = std::stod(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);                
    }     

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }

    return res;
}

int CommandHandler::arg_int(    const std::vector<std::string>& args, 
                                const char* name, 
                                int min_val, 
                                int max_val, 
                                const std::list<std::string>& allowed_units, 
                                std::string& unit, 
                                const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    int res;
    std::string err;

    // convert to double
    try {
        res = std::stoi(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);                
    }    

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }

    return res;
}
/*
unsigned int CommandHandler::arg_uint(   const std::vector<std::string>& args,
                                const char* name,
                                unsigned int min_val,
                                unsigned int max_val,
                                const std::list<std::string>& allowed_units,
                                std::string& unit,
                                const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    unsigned int res;
    std::string err;

    // convert to double
    try {
        res = std::stol(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);
    }

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %d%s <= v <= %d%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);
    }

    return res;
}
*/
unsigned int CommandHandler::arg_unsigned_int(  const std::vector<std::string>& args, 
                                                const char* name, 
                                                unsigned int min_val, 
                                                unsigned int max_val, 
                                                const std::list<std::string>& allowed_units, 
                                                std::string& unit, 
                                                const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    unsigned int res;
    std::string err;

    // convert to double
    try {
        res = std::stoul(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %u%s <= v <= %u%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %u%s <= v <= %u%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);                
    }

    // check passed value was read correctly
    if (v!=std::to_string(res)) {
        err = tools::format_string( "Invalid value is specified (negative value) for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);                        
    }    

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %u%s <= v <= %u%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %u%s <= v <= %u%s)", name, min_val, default_unit, max_val, default_unit);
        throw CommandHandlerException(err);        
    }

    return res;
}


long long CommandHandler::arg_long_long(    const std::vector<std::string>& args, 
                                            const char* name, 
                                            long long min_val, 
                                            long long max_val, 
                                            const std::list<std::string>&allowed_units, 
                                            std::string& unit, 
                                            const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    long long res;
    std::string err;

    // convert to double
    try {
        res = std::stoll(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string( "Invalid value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string( "Invalid value is specified (out of range) for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);                
    }    

    if (res < min_val) {
        err = tools::format_string( "Less than minimal value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    } else if (res > max_val) {
        err = tools::format_string( "Greater than maximum value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    }

    return res;
}

unsigned long long CommandHandler::arg_unsigned_long_long(  const std::vector<std::string>& args, 
                                                            const char* name,  
                                                            unsigned long long min_val, 
                                                            unsigned long long max_val, 
                                                            const std::list<std::string>&allowed_units, 
                                                            std::string& unit,
                                                            const char* default_unit) {
    std::string v = arg_unit(args, name, allowed_units, unit);
    unsigned long long res;
    std::string err;

    // convert to double
    try {
        res = std::stoull(v);
    } catch (std::invalid_argument& iae) {
        err = tools::format_string( "Invalid value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string( "Invalid value is specified (out of range) for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);                
    }

    // check passed value was read correctly
    if (v!=std::to_string(res)) {
        err = tools::format_string( "Invalid value is specified (negative value) for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);                        
    }


    if (res < min_val) {
        err = tools::format_string( "Less than minimal value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    } else if (res > max_val) {
        err = tools::format_string( "Greater than maximum value is specified for %s (value must be between: %s%s <= v <= %s%s)", 
                                    name, 
                                    std::to_string(min_val), 
                                    default_unit, 
                                    std::to_string(max_val), 
                                    default_unit);

        throw CommandHandlerException(err);        
    }

    return res;
}

#ifdef INFO_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    InfoDevHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(InfoDevHandler,"info::", "::print")
std::string InfoDevHandler::help() const {
    return tools::format_string("# %s shows information about firmware and features available.\n",
                                get_command_name());
}
void InfoDevHandler::handle(const std::vector<std::string>& args) {

    check_arg_count(args, 0);
    auto info_dev = dynamic_cast<INFODev*>(device.get());
    info_dev->check();

    std::string pname = info_dev->get_dev_name();

    std::map<uint8_t, std::string> hint_map = {
            {INFO_DEV_HINT_NONE,      ""},
            {INFO_DEV_HINT_GSM_MODEM, "GSM MODEM"}};

    std::map<uint8_t, std::string> device_type_map = {
            {INFO_DEV_TYPE_NONE,      "<none>"},
            {INFO_DEV_TYPE_INFO,      "INFODev"},
            {INFO_DEV_TYPE_DESKDEV,  "DESKDev"},
            {INFO_DEV_TYPE_IRRC,      "IRRCDev"},
            {INFO_DEV_TYPE_LCD1602a,  "LCD1602ADev"},
            {INFO_DEV_TYPE_RTC,       "RTCDev"},
            {INFO_DEV_TYPE_UART_PROXY,"UARTDev"},
            {INFO_DEV_TYPE_GPIO,      "GPIODev"},
            {INFO_DEV_TYPE_SPWM,      "SPWMDev"},
            {INFO_DEV_TYPE_ADC,       "ADCDev"},
            {INFO_DEV_TYPE_STEP_MOTOR,"StepMotorDev"}};

    ui->log(tools::str_format("Project: %s", pname.c_str()));

    size_t li = 0;
    for (size_t i=0; i<INFO_DEVICES_NUMBER; i++) {
        const InfoDeviceDescriptor* di = INFODev::get_device_info(i);
        if (di->type==INFO_DEV_TYPE_NONE) {
            continue;
        }

        try {
            std::string dev_type = device_type_map.at(di->type);
            std::string hint = hint_map.at(di->hint);
            std::string name =di->name;
            ui->log(tools::str_format("%d) name=%s dev_id=%d, type=%s, hint=%s", li++, name.c_str(), i, dev_type.c_str(), hint.c_str()));
        } catch(const std::out_of_range& e) {
            ui->log("error");
        }
    }
}
#endif


#ifdef UART_PROXY_DEVICE_ENABLED

//----------------------------------------------------------------------------------------------//
//                                    UartDevInfo                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(UartDevInfo,"uart::", "::info")
std::string UartDevInfo::help() const {
    return tools::format_string("# %s prints device information. No parameters are required.\n",
                                get_command_name());
}
void UartDevInfo::handle(const std::vector<std::string>& args) {
    auto uart = dynamic_cast<UARTDev*>(device.get());
    const UartProxyDevInstance* descr = uart->get_descriptor();

    ui->log(tools::str_format("UART Proxy for: \"%s\" (%d)", descr->dev_name, descr->dev_id));
    ui->log(tools::str_format("    Baud rate: %d", descr->baud_rate));
    ui->log(tools::str_format("    Buffer size (bytes): %d", descr->dev_buffer_len));
}

//----------------------------------------------------------------------------------------------//
//                                    UartDevRead                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(UartDevRead,"uart::", "::read")
std::string UartDevRead::help() const {
    return tools::format_string("# %s read data from device. No parameters are required.\n",
                                get_command_name());
}
void UartDevRead::handle(const std::vector<std::string>& args) {
    auto uart = dynamic_cast<UARTDev*>(device.get());

    std::vector<uint8_t> data;
    uart->read(data);

    std::string s = tools::format_buffer(16, data.data(), data.size(), " ", " | ");
    ui->log(s);
}

//----------------------------------------------------------------------------------------------//
//                                    UartDevWrite                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(UartDevWrite,"uart::", "::write")
std::string UartDevWrite::help() const {
    return tools::format_string("# %s writes to device..\n"
                                "# usage: <buffer>\n"
                                "#        <buffer> either text string (will be sent as UTF-8 bytes) or sequence of bytes in hex (for example: \"00 11 22 AB CD EF\")\n",
                                get_command_name());
}
void UartDevWrite::handle(const std::vector<std::string>& args) {
    auto uart = dynamic_cast<UARTDev*>(device.get());
    std::string in = args[0];
    std::vector<uint8_t> data(in.begin(), in.end());

    // figure out if text entered is sequance of bytes by regular expression
    std::unique_ptr<RegexPattern> re = tools::g_unicode_ts.regex_pattern("^([0-9,a-f,A-F]{2})(\\\\s[0-9,a-f,A-F]{2})*$", 0);
    bool hex_buffer = tools::g_unicode_ts.regex_match(*re, in);

    if (hex_buffer) {
        data = tools::buffer_from_hex(in);
    }

    uart->write(data);
}

//----------------------------------------------------------------------------------------------//
//                                    ATHandler                                                 //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ATHandler,"gsm::", "::at")
std::string ATHandler::help() const {
    return tools::format_string("# %s instructs GSM modem to execute AT command.\n"
                                "# usage: <atcmd>\n"
                                "#        <atcmd> - at command to execute\n",
                                get_command_name());
}   
void ATHandler::handle(const std::vector<std::string>& args) {
    unsigned int status = GSMModem::AT_STATUS_OK | GSMModem::AT_STATUS_ERROR;
    std::vector<std::string> response;
    check_arg_count_min(args, 1);
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->at(args.at(0), response, 30000, status);

    for (const auto& i : response) {
        ui->log(i);
    }

    ui->log("[status=" + GSMModem::status_description(status) + "]");
}

//----------------------------------------------------------------------------------------------//
//                                    SMSHandler                                                //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SMSHandler,"gsm::", "::send_sms")
std::string SMSHandler::help() const {
    return tools::format_string("# %s instructs GSM modem to send SMS.\n"
                                "# usage:  <number>,<text>\n"
                                "#        <number> - phone number\n"
                                "#        <text> - text in double quotes\n",
                                get_command_name());
}       
void SMSHandler::handle(const std::vector<std::string>& args) {
    unsigned int status = 0;
    size_t arg_c = args.size();
    check_arg_count(args, 2);
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->sms(arg_get(args, "number"), arg_get(args, "text"), 0, status);
    ui->log("[status=" + GSMModem::status_description(status) + "]");
}

//----------------------------------------------------------------------------------------------//
//                                    USSDHandler                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(USSDHandler,"gsm::", "::ussd")
std::string USSDHandler::help() const {
    return tools::format_string("# %s instructs GSM modem to USSD request.\n"
                                "# usage:  <ussd>\n"
                                "#        <ussd> - ussd request\n",
                                get_command_name());
}       
void USSDHandler::handle(const std::vector<std::string>& args) {
    std::string result;
    unsigned int status = 0;
    check_arg_count_min(args, 1);
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->ussd(args.at(0), result, 0, status);
    ui->log(result);
    ui->log("[status=" + GSMModem::status_description(status) + "]");
}

//----------------------------------------------------------------------------------------------//
//                                    ReadSMSHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ReadSMSHandler,"gsm::", "::read_sms")
std::string ReadSMSHandler::help() const {
    return tools::format_string("# %s reads all available SMS in GSM modem. No argumentes are required.\n",
                                get_command_name());
}  
void ReadSMSHandler::handle(const std::vector<std::string>& args){
    unsigned int status = 0;
    std::vector<GSMSmsData> messages;
    check_arg_count(args, 0);
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->read_sms(messages, 30000, status);
    size_t n_msg = messages.size();
    for (size_t i=0; i<n_msg; ) {
        const GSMSmsData& sms = messages.at(i);
        ui->log(tools::str_format("[%d] %s | %s | %s", sms.id, sms.phone_number.c_str(), sms.status.c_str(), sms.timestamp.c_str()));
        ui->log(sms.message);
        if (++i!=n_msg) {
            ui->log("----------------------------------------------------------------------------------");
        }
    }

    ui->log("[status=" + GSMModem::status_description(status) + "]");
}

//----------------------------------------------------------------------------------------------//
//                                    DeleteSMSHandler                                          //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(DeleteSMSHandler,"gsm::", "::delete_sms")
std::string DeleteSMSHandler::help() const {
    return tools::format_string("# %s deletes SMS(s) in GSM modem.\n"
                                "# usage:  <*> or <id>\n"
                                "#         <*> - delete all messages\n"
                                "#         <id> - id of the message to be deleted\n",
                                get_command_name());
}  
void DeleteSMSHandler::handle(const std::vector<std::string>& args) {

    unsigned int status = 0;
    auto modem = dynamic_cast<GSMModem*>(device.get());
    check_arg_count(args, 1);
    std::string unit;

    if (args.at(0)=="*") {
        modem->delete_sms(-1, 30000, status);
    } else {
        unsigned int id = arg_unsigned_int(args, "id", 0, UINT_MAX, {""}, unit, "");
        modem->delete_sms(static_cast<int>(id), 30000, status);
    }

    ui->log("[status=" + GSMModem::status_description(status) + "]");
}

//----------------------------------------------------------------------------------------------//
//                                    DialHandler                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(DialHandler,"gsm::", "::dial")
std::string DialHandler::help() const {
    return tools::format_string("# %s dials a number.\n"
                                "# usage:  <phone>\n"
                                "#         <phone> - phone number\n",
                                get_command_name());
} 
void DialHandler::handle(const std::vector<std::string>& args){
    unsigned int status = 0;
    check_arg_count(args, 1);
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->dial(arg_get(args, "phone"), 30000, status);
    ui->log("[status=" + GSMModem::status_description(status) + "]");
}


//----------------------------------------------------------------------------------------------//
//                                    ActiveCallsHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ActiveCallsHandler,"gsm::", "::active_calls")
std::string ActiveCallsHandler::help() const {
    return tools::format_string("# %s shows active calls list. No argumentes are required.\n",
                                get_command_name());
}  
void ActiveCallsHandler::handle(const std::vector<std::string>& args){
    unsigned int status = 0;
    check_arg_count(args, 0);
    std::vector<GSMCallData> act_calls;
    std::list<std::string> act_calls_descr;
    auto modem = dynamic_cast<GSMModem*>(device.get());
    modem->active_calls(act_calls, 30000, status);

    for (auto c = act_calls.begin(); c!=act_calls.end(); ++c) {
        act_calls_descr.push_back(c->to_string());
    }

    std::string res = tools::join_strings(act_calls_descr, "----------------------------------------------------------------------------------");
    ui->log(res);
    ui->log("[status=" + GSMModem::status_description(status) + "]");    
}

//----------------------------------------------------------------------------------------------//
//                                    AnswerCallHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(AnswerCallHandler,"gsm::", "::answer")
std::string AnswerCallHandler::help() const {
    return tools::format_string("# %s answers to an active call.\n"
                                "# usage:  <action>\n"
                                "#         <action> - one of the following actions:\n"
                                "#                    answer - answer an incoming call\n"
                                "#                    hang - disconnect existing connection\n"
                                "#                    hold - place all active calls on hold (if any) and accept the other (held or waiting) call\n"
                                "#                    release - releases all active calls (if any exist) and accepts the other (held or waiting) call.\n",
                                get_command_name());
}
    void AnswerCallHandler::handle(const std::vector<std::string>& args){
        unsigned int status = 0;
        check_arg_count(args, 1);
        auto modem = dynamic_cast<GSMModem*>(device.get());

        std::string arg = arg_get(args, "action");
        tools::g_unicode_ts.to_case(arg, true);

        if (arg=="answer") {
            modem->answer(GSM_CALL_ACTION_ANSWER, 30000, status);
        } else if (arg=="hang") {
            modem->answer(GSM_CALL_ACTION_HANG, 30000, status);
        } else if (arg=="hold") {
            modem->answer(GSM_CALL_ACTION_HOLD, 30000, status);
        } else if (arg=="release") {
            modem->answer(GSM_CALL_ACTION_RELEASE, 30000, status);
        } else {
            throw CommandHandlerException("Invalid argument specified for action (valid values are: answer, hang, hold, release)");
        }

        ui->log("[status=" + GSMModem::status_description(status) + "]");            
    }    
#endif

#ifdef LCD1602a_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    LCDPrintHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(LCDPrintHandler,"lcd::", "::print")
std::string LCDPrintHandler::help() const {
    auto lcd = dynamic_cast<LCD1602ADev*>(device.get());
    return tools::format_string("# %s prints text on LCD1602ADev screen (%d lines supported).\n"
                                "# usage:  <l1>,...<ln>\n"
                                "#         <ln> - line in double quotes (make sure length and number of lines is correct is right)\n",
                                get_command_name(),
                                lcd->nlines());
}    
void LCDPrintHandler::handle(const std::vector<std::string>& args) {
    auto lcd = dynamic_cast<LCD1602ADev*>(device.get());
    check_arg_count(args, lcd->nlines());
    int nlines = lcd->nlines();
    lcd->write(args.begin()+1, args.end());
}

//----------------------------------------------------------------------------------------------//
//                                    LCDLightHandler                                           //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(LCDLightHandler,"lcd::", "::light")
std::string LCDLightHandler::help() const {
    return tools::format_string("# %s controls LCD screen backlight.\n"
                                "# usage:  <state>\n"
                                "#         <state> one of the following states:\n"
                                "#                 on - backlight is permanently on\n"
                                "#                 off - backlight is permanently off\n"
                                "#                 blink - backlight is blinking\n",
                                get_command_name());
}
    void LCDLightHandler::handle(const std::vector<std::string>& args) {
        check_arg_count(args, 1);
        auto lcd = dynamic_cast<LCD1602ADev*>(device.get());

        std::string p = arg_get(args, "state");
        tools::g_unicode_ts.to_case(p, true);

        if (p=="on") {
            lcd->light(LCD1602a_LIGHT);
        } else if (p=="off") {
            lcd->light(LCD1602a_OFF);
        } else if (p=="blink") {
            lcd->light(LCD1602a_BLINK);
        } else {
            throw CommandHandlerException("Invalid argument specified for state (valid values are: on, off, blink)");
        }
    }
#endif

#ifdef DESKDEV_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    DeskDevStatusHandler                                          //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(DeskDevStatusHandler,"desk::", "::status")
std::string DeskDevStatusHandler::help() const {
    return tools::format_string("# %s reports desk device status. No parameters are required.\n",
                                get_command_name());
}
void DeskDevStatusHandler::handle(const std::vector<std::string>& args) {
    bool up,down,left,right;
    int8_t ncodeder;
    check_arg_count(args, 0);
    auto kbd = dynamic_cast<DESKDev*>(device.get());

    kbd->get(up, down, left, right, ncodeder);

    std::string keys;
    if (up) keys = "up,";
    if (down) keys += "down,";
    if (left) keys += "left,";
    if (right) keys += "right,";
    size_t len = keys.length();
    if (len!=0) {
        keys.erase(len-1);
        ui->log(keys);
    }

    if (ncodeder!=0) {
        ui->log(tools::str_format("encoder: %i", ncodeder));
    }
}
#endif

#ifdef IRRC_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    IRRCHandler                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(IRRCHandler,"irrc::", "::status")
std::string IRRCHandler::help() const {
    return tools::format_string("# %s reports IR remote control status. No parameters are required.\n",
                                get_command_name());
}
void IRRCHandler::handle(const std::vector<std::string>& args) {
    check_arg_count(args, 0);
    auto irrc = dynamic_cast<IRRCDev*>(device.get());
    bool ovf;
    std::vector<IR_NEC_Command> irrc_data;

    irrc->get(irrc_data,ovf);

    if (ovf) {
        ui->log("Warning: IRRCDev data was overwritten in device circular buffer");
    }

    size_t n_cmd = irrc_data.size();
    if (n_cmd!=0) {
        int n_rpt = 0;
        IR_NEC_Command prev_cmd={0};
        for (size_t i=0; i<n_cmd; i++) {
            if (i==0) {
                // first element
                prev_cmd = irrc_data[i];
                n_rpt = 1;
            } else
            if (i>0 && irrc_data[i-1]==irrc_data[i]) {
                // repeated element
                n_rpt++;
            } else {
                // new element
                ui->log(tools::str_format("[N=%d], Address: 0x%X, Command: 0x%X", n_rpt, prev_cmd.address, prev_cmd.command));
                n_rpt = 1;
                prev_cmd = irrc_data[i];
            }
        }
        ui->log(tools::str_format("[N=%d], Address: 0x%X, Command: 0x%X", n_rpt, prev_cmd.address, prev_cmd.command));
    }
}
#endif

#ifdef RTC_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    RTCGetHandler                                             //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(RTCGetHandler,"rtc::", "::now")
std::string RTCGetHandler::help() const {
    return tools::format_string("# %s reports current RTCDev and system time. No parameters are required.\n",
                                get_command_name());
}
void RTCGetHandler::handle(const std::vector<std::string>& args) {
    check_arg_count(args, 0);
    auto rtc = dynamic_cast<RTCDev*>(device.get());
    std::time_t t = rtc->now();
    std::time_t h = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());   
    std::string str_rtc_time = tools::g_unicode_ts.dtime_to_utf8(t);
    std::string str_host_time = tools::g_unicode_ts.dtime_to_utf8(h);
    ui->log(tools::str_format("RTCDev time: %s", str_rtc_time.c_str()));
    ui->log(tools::str_format("HOST time: %s", str_host_time.c_str()));
}


//----------------------------------------------------------------------------------------------//
//                                    RTCSyncRtcHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(RTCSyncRtcHandler,"rtc::", "::sync_rtc")
std::string RTCSyncRtcHandler::help() const {
    return tools::format_string("# %s synchronizes RTCDev with host system time. No parameters are required.\n",
                                get_command_name());
}
void RTCSyncRtcHandler::handle(const std::vector<std::string>& args) {
    auto rtc = dynamic_cast<RTCDev*>(device.get());\
    check_arg_count(args, 0);
    std::time_t t = rtc->sync_rtc();
    std::string str_time = tools::g_unicode_ts.dtime_to_utf8(t);
    ui->log(tools::str_format("RTCDev time updated to : %s", str_time.c_str()));
}


//----------------------------------------------------------------------------------------------//
//                                    RTCSyncHostHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(RTCSyncHostHandler,"rtc::", "::sync_host")
std::string RTCSyncHostHandler::help() const {
    return tools::format_string("# %s synchronizes host system time with RTCDev. No parameters are required.\n",
                                get_command_name());
}
void RTCSyncHostHandler::handle(const std::vector<std::string>& args) {
    auto rtc = dynamic_cast<RTCDev*>(device.get());
    check_arg_count(args, 0);
    std::time_t t = rtc->sync_host();
    std::string str_time = tools::g_unicode_ts.dtime_to_utf8(t);
    ui->log(tools::str_format("HOST time updated to : %s", str_time.c_str()));
}
#endif

#ifdef GPIODEV_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    GPIOHandler                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(GPIOHandler,"gpio::", "::sync")
std::string GPIOHandler::help() const {
    return tools::format_string("# %s synchronizes with GPIO device.\n"
                                "# usage:  <state> or no parameters\n"
                                "#         <state> to be set. state should be a sequance of 0 and 1 (%u bits total) to set corresponding gpio:\n"
                                "#         if no parameters given gpio is read and reported\n",
                                GPIO_PIN_COUNT,
                                get_command_name());
}
void GPIOHandler::handle(const std::vector<std::string>& args) {

    size_t arg_c = args.size();
    auto gpiodev = dynamic_cast<GPIODev*>(device.get());
    bool bad_param = false;
    std::bitset<GPIO_PIN_COUNT> bits;
    std::string p = args[0];
    std::string t;
    size_t arg_len = p.length();
    bits.reset();

    if (arg_c==1 && arg_len==0) {
        gpiodev->read(bits);
        t = bits.to_string();
        ui->log(tools::str_format("READ: %s", t.c_str()));
    } else if (arg_c==2 && arg_len==GPIO_PIN_COUNT) {
        for (size_t i=0; i<GPIO_PIN_COUNT; i++) {
            if (p[i]=='1') {
                bits.set(GPIO_PIN_COUNT - i - 1);
            } else if (p[i]!='0') {
                throw CommandHandlerException(tools::format_string( "Invalid argument specified for state (valid value should be a sequance of 0 and 1, %d symbols in length)",
                                                                    GPIO_PIN_COUNT));
            }
        }

        gpiodev->write(bits);
        t = bits.to_string();
        ui->log(tools::str_format("SET: %s", t.c_str()));
    }
}
#endif


#ifdef SPWM_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    SPWMListHandler                                           //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPWMListHandler,"spwm::", "::list")
std::string SPWMListHandler::help() const {
    return tools::format_string("# %s Lists SPWM outputs. No parameters are required.\n",
                                get_command_name());
}
void SPWMListHandler::handle(const std::vector<std::string>& args) {
    auto spwm = dynamic_cast<SPWMDev*>(device.get());
    SPWM_STATE state;
    check_arg_count(args, 0);
    spwm->set(state);

    for (auto it=state.begin(); it!=state.end(); ++it) {
        size_t channel_index = it->first;
        
        uint16_t val = it->second;

        const SPWM_SW_DESCRIPTOR* cdescr = spwm->get_channel_info(channel_index);
        size_t port_index = cdescr->port_index;
        size_t pin_number = cdescr->pin_number;
        const char* name = cdescr->channel_name;
        float cur_val = 1.0f-float(val)/float(0xFFFF);
        float def_val = cdescr->def_val ? 1.0f : 0.0f;

        ui->log(tools::str_format("%d) %s=%f [Port=%d, Pin=%d, %f by default]", channel_index, name, cur_val, port_index, pin_number, def_val));
    }
}

//----------------------------------------------------------------------------------------------//
//                                    SPWMSetHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPWMSetHandler,"spwm::", "::set")
std::string SPWMSetHandler::help() const {
    auto spwm = dynamic_cast<SPWMDev*>(device.get());
    return tools::format_string("# %s Set SPWM outputs.\n"
                                "# usage:  <index>=<value>,...\n"
                                "#         <index> index of the SPWM output. Must be in range: [0, %d)\n"
                                "#         <value> value of the output. It should be floating point value in range 0.0 ... 1.0\n",
                                get_command_name(),
                                spwm->get_channel_count());
}
void SPWMSetHandler::handle(const std::vector<std::string>& args) {
    auto spwm = dynamic_cast<SPWMDev*>(device.get());
    SPWM_STATE state;
    size_t arg_c = args.size();
    std::unique_ptr<RegexPattern> re = tools::g_unicode_ts.regex_pattern("^(\\d+)\\s*=\\s*(\\d(\\.\\d+)?)$", 0);
    std::vector<std::string> groups;
    size_t pin;
    float val;
    std::string t;

    if (arg_c<1) {
        goto done; // incorrect number of parameters
    }

    for (size_t i=1; i<arg_c; i++) {
        groups.clear();
        bool res = tools::g_unicode_ts.regex_groups(*re, args[i], groups);
        if (!res) goto done;

        t = groups[1];
        try {
            pin = std::stoi(t);
        } catch (std::invalid_argument& e) {
            goto done;
        }
        if (pin>=spwm->get_channel_count()) goto done;

        t = groups[2];
        try {
            val = std::stof(t);
        } catch (std::invalid_argument& e) {
            goto done;
        }

        if (val<0.0f || val>1.0f)
            goto done;

        state[pin] = ((float)0xFFFF)*(1.0f-val);
    }

    spwm->set(state);
done:
    return;
}

//----------------------------------------------------------------------------------------------//
//                                    SPWMResetHandler                                          //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPWMResetHandler,"spwm::", "::reset")
std::string SPWMResetHandler::help() const {
    return tools::format_string("# %s Reset SPWM devices to defaults. No parameters are required.\n",
                                get_command_name());
}
void SPWMResetHandler::handle(const std::vector<std::string>& args) {
    auto spwm = dynamic_cast<SPWMDev*>(device.get());
    check_arg_count(args, 0);    
    spwm->reset();
}
#endif

#ifdef ADCDEV_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    ADCDevStartHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevStartHandler,"adc::", "::start")
std::string ADCDevStartHandler::help() const {
    return tools::format_string("# %s Starts ADC conversion.\n"
                                "# usage: <count>,<period><unit>\n"
                                "#        <count> number of samples to sample\n"
                                "#        <period> time period between samples\n"
                                "#        <unit> 'us' - microseconds, 'ms' - milliseconds, 's' - seconds\n"
                                "# note: actual delay between samples may be inaccurate, especially if very little delays specified\n",
                                get_command_name());
}
void ADCDevStartHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    std::string unit;
    check_arg_count(args, 2);
    size_t sample_count = arg_int(args, "count", 0, USHRT_MAX, {""}, unit, "");
    double delay_f = arg_double(args, "period", 0.0, DBL_MAX, {"us", "ms", "s"}, unit, "s");
    delay_f = arg_time_to_sec(delay_f, unit);
    adc->start(sample_count, delay_f);
}


//----------------------------------------------------------------------------------------------//
//                                    ADCDevStopHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevStopHandler,"adc::", "::stop")
std::string ADCDevStopHandler::help() const {
    return tools::format_string("# %s stops ADC conversion.\n",
                                "# usage: either no arguments or \"reset\" must be specified\n"
                                "#        if \"reset\" is specified, all buffered data for the device will be cleared\n",
                                get_command_name());
}
void ADCDevStopHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    size_t argc = check_arg_count_min(args, 0);
    if (argc==1 && arg_get(args, "reset")=="reset") {
        adc->stop(true);
    } else if (argc==0) {
        adc->stop(false);
    }

    throw CommandHandlerException("Invalid argument specified");
}

//----------------------------------------------------------------------------------------------//
//                                    ADCDevReadHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevReadHandler,"adc::", "::read")
std::string ADCDevReadHandler::help() const {
    return tools::format_string("# %s reads ADC data. No arguments are required\n",
                                get_command_name());
}
void ADCDevReadHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    bool overflow = false;
    size_t argc = check_arg_count(args, 0);
    size_t channel_count = adc->get_input_count();
    std::vector<std::vector<double>> data;
    adc->get(data, overflow);

    if (overflow) {
        ui->log("*** Warning: overflow detected");
    }

    // print data
    for (size_t i=0; i<channel_count; i++) {
        const std::vector<double>& v = data.at(i);
        std::string l = adc->get_input_name(i, false) + ":";
        for (auto s = v.begin(); s!=v.end(); ++s) {
            l+= " " + std::to_string(*s);
        }
        ui->log(l);
    }
}

//----------------------------------------------------------------------------------------------//
//                                    ADCDevReadMeanHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevReadMeanHandler,"adc::", "::read_mean")
std::string ADCDevReadMeanHandler::help() const {
    return tools::format_string("# %s reads ADC data and print averages. No arguments are required\n",
                                get_command_name());
}
void ADCDevReadMeanHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    bool overflow = false;
    size_t argc = check_arg_count(args, 0);
    std::vector<double> data;
    adc->get(data, overflow);

    size_t channel_count = adc->get_input_count();

    if (overflow) {
        ui->log("*** Warning: overflow detected");
    }

    // print data
    for (size_t i=0; i<channel_count; i++) {
        std::string l = adc->get_input_name(i, false) + ": " + std::to_string(data.at(i));
        ui->log(l);
    }
}

#endif

#ifdef STEP_MOTOR_DEVICE_ENABLED
//----------------------------------------------------------------------------------------------//
//                                    StepMotorInfoHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorInfoHandler,"step_motor::", "::info")
std::string StepMotorInfoHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s shows information for %s device. No parameters are required.\n",
                                get_command_name(), 
                                smd->get_dev_name());
}
void StepMotorInfoHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    auto res = smd->get_motor_info();
    size_t mcount = res.size();
    for (size_t mindex=0; mindex<mcount; mindex++) {
        const StepMotorDescriptor* mdescr = res[mindex];
        ui->log(tools::str_format("Step motor index: %d (%s)", mindex, mdescr->motor_name));

        std::map<uint32_t, std::pair<std::string, std::string>> config_flag_map = {
            {STEP_MOTOR_M1_IN_USE, {"M1 is used", "M1 is NOT used"}},
            {STEP_MOTOR_M2_IN_USE, {"M2 is used", "M2 is NOT used"}},
            {STEP_MOTOR_M3_IN_USE, {"M3 is used", "M3 is NOT used"}},
            {STEP_MOTOR_ENABLE_IN_USE, {"Enable pin is used", "Enable pin is NOT used"}},
            {STEP_MOTOR_RESET_IN_USE, {"Reset pin is used", "Reset pin is NOT used"}},
            {STEP_MOTOR_SLEEP_IN_USE, {"Sleep pin is used", "Sleep pin is NOT used"}},
            {STEP_MOTOR_FAULT_IN_USE, {"Fault pin is used", "Fault pin is NOT used"}},
            {STEP_MOTOR_CWENDSTOP_IN_USE, {"CW endstop is used", "CW endstop is NOT used"}},
            {STEP_MOTOR_CCWENDSTOP_IN_USE, {"CCW endstop is used", "CCW endstop is NOT used"}},
            {STEP_MOTOR_DIR_IN_USE, {"Direction pin is used", "Direction pin is NOT used"}},
            {STEP_MOTOR_FAULT_ACTIVE_HIGH, {"Fault triggers on HIGH", "Fault triggers on LOW"}},
            {STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH, {"CW endstop triggers on HIGH", "CW endstop triggers on LOW"}},
            {STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH, {"CCW endstop triggers on HIGH", "CCW endstop triggers on LOW"}},
            {STEP_MOTOR_M1_DEFAULT, {"By default M1=1", "By default M1=0"}},
            {STEP_MOTOR_M2_DEFAULT, {"By default M2=1", "By default M2=0"}},
            {STEP_MOTOR_M3_DEFAULT, {"By default M3=1", "By default M3=0"}},
            {STEP_MOTOR_DIRECTION_CW, {"By default DIRECTION is CW", "By default is DIRECTION is CCW"}},
            {STEP_MOTOR_DISABLE_DEFAULT, {"By default is DISABLED", "By default is ENABLED"}},
            {STEP_MOTOR_WAKEUP_DEFAULT, {"By default is NOT in SLEEP mode", "By default in SLEEP mode"}}};

        std::string conftext = tools::flags_to_string(mdescr->config_flags, config_flag_map, "\n    ");
        if (conftext.empty()) conftext="0";
        else conftext = "\n    " + conftext;

        ui->log(tools::str_format("    Configuration: %d %s", mdescr->config_flags, conftext.c_str()));
        ui->log(tools::str_format("    Buffer size              : %d", mdescr->buffer_size));
        ui->log(tools::str_format("    Steps per revolution     : %d", mdescr->steps_per_revolution));
    }

}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorEnableHandler                                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorEnableHandler,"step_motor::", "::enable")
std::string StepMotorEnableHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s enables or disables step motor driver.\n"
                                "# usage: <mindex>,<state>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"
                                "#        <state> - state of the motor. To enable motor specify either of : on, 1, true; To disable : off, 0, false",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}
void StepMotorEnableHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;
    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    bool val = arg_boolean(args, "state", {"on", "1", "true"}, {"off", "0", "false"});
    smd->enable(mindex, val);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorSleepHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorSleepHandler,"step_motor::", "::sleep")
std::string StepMotorSleepHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s puts one of step motor driver into sleep state or wakes it up.\n"
                                "# usage: <mindex>,<state>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"
                                "#        <state> - state of the motor. To enable motor specify either of : sleep, 1, true; To disable : wakeup, 0, false",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorSleepHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;
    
    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    bool val = arg_boolean(args, "state", {"sleep", "1", "true"}, {"wakeup", "0", "false"});
    smd->sleep(mindex, val);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorWaitHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorWaitHandler,"step_motor::", "::wait")
std::string StepMotorWaitHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s instructs step motor driver to wait.\n"
                                "# usage: <mindex>,<value><unit>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <value> - integer value that specifies amount ot time to wait\n"
                                "#        <unit> - unit to measure wait value: 'us' - microseconds, 'ms' - milliseconds, 's' - seconds",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorWaitHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;        
    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    double wperiod = arg_double(args, "value", 0.0, (double)ULLONG_MAX/1.0e6, {"us", "ms", "s"}, unit, "s");
    wperiod = arg_time_to_sec(wperiod, unit);
    smd->wait(mindex, wperiod);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorDirHandler                                       //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorDirHandler,"step_motor::", "::dir")
std::string StepMotorDirHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s sets direction for step motor driver.\n"
                                "# usage: <mindex>,<dir>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <dir> - direction, may be either 'cw' - clockwise or 'ccw' - counter clockwise",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorDirHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;

    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    bool val = arg_boolean(args, "dir", {"cw"}, {"ccw"});
    smd->dir(mindex, val);
}


//----------------------------------------------------------------------------------------------//
//                                    StepMotorSpeedHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorSpeedHandler,"step_motor::", "::speed")
std::string StepMotorSpeedHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s sets speed of step motor rotation.\n"
                                "# usage: <mindex>,<value><uint>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <value> - floating point value that specifies either:\n"
                                "#                  * amount of time required for each step\n"
                                "#                  * or number of revolutions per minute\n"
                                "#        <unit> - step duration units: 'us' - microseconds, 'ms' - milliseconds, 's' - seconds\n"
                                "#                 revolutions per minute: 'rpm' - indicates value is rpm",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorSpeedHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;        
    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    double value = arg_double(args, "value", 0.0, (double)ULLONG_MAX/1.0e6, {"us", "ms", "s", "rpm"}, unit, "s");
    if (unit=="rpm") {
        smd->speed(mindex, value, true);
    } else {
        value = arg_time_to_sec(value, unit);
        smd->speed(mindex, value, false);
    }
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorMicroStepHandler                                 //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorMicroStepHandler,"step_motor::", "::microstep")
std::string StepMotorMicroStepHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s sets speed of step motor rotation.\n"
                                "# usage: <mindex>,<m1>,<m2>,<m3>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <m1> - value for m1\n"
                                "#        <m2> - value for m2\n"
                                "#        <m3> - value for m3",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorMicroStepHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;
            
    check_arg_count(args, 4);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    bool m1 = arg_boolean(args, "m1", {"set", "1", "true"}, {"clear", "0", "false"});
    bool m2 = arg_boolean(args, "m2", {"set", "1", "true"}, {"clear", "0", "false"});
    bool m3 = arg_boolean(args, "m3", {"set", "1", "true"}, {"clear", "0", "false"});
    smd->microstep(mindex, m1, m2, m3);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorConfigHandler                                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorConfigHandler,"step_motor::", "::config")
std::string StepMotorConfigHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s sets configuration for step motor.\n"
                                "# usage: <mindex>,<opts>...\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <opts> - are one of the following values(several opts may be specified\n"
                                "#                 * icw - ignore clockwise endstop\n"
                                "#                 * acw - clockwise endstop stops all step motors (by default just this step motor is stopped)\n"
                                "#                 * iccw - ignore counter clockwise endstop\n"
                                "#                 * accw - counter clockwise endstop stops all step motors (by default just this step motor is stopped)\n"
                                "#                 * if - ignore fault\n"
                                "#                 * af - fault stops all step motors (by default just this step motor is stopped)\n"
                                "#                 * ea - any error must stop all motors (by default just this step motor is stopped)\n"
                                "#                 note: 'all step motors' options have higher priority\n",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorConfigHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;
    uint32_t flags = 0;
    check_arg_count_min(args, 1);

    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");

    std::set<std::string> opts(std::next(std::next(args.begin())), args.end());
    arg_options_check(opts, {"icw","acw","iccw","accw","if","af", "ea"});

    flags += (opts.find("icw")!=opts.end()) * STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE;
    flags += (opts.find("acw")!=opts.end()) * STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL;
    flags += (opts.find("iccw")!=opts.end()) * STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE;
    flags += (opts.find("accw")!=opts.end()) * STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL;
    flags += (opts.find("if")!=opts.end()) * STEP_MOTOR_CONFIG_FAILURE_IGNORE;
    flags += (opts.find("af")!=opts.end()) * STEP_MOTOR_CONFIG_FAILURE_ALL;
    flags += (opts.find("ea")!=opts.end()) * STEP_MOTOR_CONFIG_ERROR_ALL;

    smd->configure(mindex, flags);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorResetHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorResetHandler,"step_motor::", "::reset")
std::string StepMotorResetHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s resets step motor driver.\n"
                                "# usage: <mindex>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorResetHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
            
    smd->reset(mindex);
}    

//----------------------------------------------------------------------------------------------//
//                                    StepMotorStatusHandler                                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorStatusHandler,"step_motor::", "::status")
std::string StepMotorStatusHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s reads status of the %s device. No parameters are required.\n",
                                get_command_name(), 
                                smd->get_dev_name());
}     
void StepMotorStatusHandler::handle(const std::vector<std::string>& args) {
    check_arg_count(args, 0);
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::vector<StepMotorStatus> mstatus;
    std::string dev_status;

    std::map<uint32_t, std::pair<std::string, std::string>> motor_state_power_map = {
            {STEP_MOTOR_DISABLE_DEFAULT,            {"DISABLED",            "ENABLED"}},
            {STEP_MOTOR_WAKEUP_DEFAULT,             {"WAKEUP",              "SLEEP"}}};

    std::map<uint32_t, std::pair<std::string, std::string>> motor_state_status_map = {
            {STEP_MOTOR_FAILURE,                    {"FAILURE",             ""}},
            {STEP_MOTOR_CW_ENDSTOP_TRIGGERED,       {"CW END",              ""}},
            {STEP_MOTOR_CCW_ENDSTOP_TRIGGERED,      {"CCW END",             ""}},
            {STEP_MOTOR_ERROR,                      {"ERROR",               ""}},
            {STEP_MOTOR_DONE,                       {"DONE",                ""}},
            {STEP_MOTOR_SUSPENDING,                 {"SUSPENDING",          ""}}};

    std::map<uint32_t, std::pair<std::string, std::string>> motor_state_config_map = {
            {STEP_MOTOR_CONFIG_FAILURE_IGNORE,      {"FAILURE: IGNORE",     ""}},
            {STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE,   {"CW END: IGNORE",      ""}},
            {STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE,  {"CCW END: IGNORE",     ""}},
            {STEP_MOTOR_CONFIG_FAILURE_ALL,         {"FAILURE: STOP ALL",   ""}},
            {STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL,      {"CW END: STOP ALL",    ""}},
            {STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL,     {"CCW END: STOP ALL",   ""}},
            {STEP_MOTOR_CONFIG_ERROR_ALL,           {"ERROR: STOP ALL",     ""}}};

    std::map<uint32_t, std::pair<std::string, std::string>> motor_state_microstep_map = {
            {STEP_MOTOR_M1_DEFAULT,                 {"M1=1",                "M1=0"}},
            {STEP_MOTOR_M2_DEFAULT,                 {"M2=1",                "M2=0"}},
            {STEP_MOTOR_M3_DEFAULT,                 {"M3=1",                "M3=0"}}};

    uint8_t dev_flags = smd->status(mstatus);
    size_t mcount = smd->get_motor_count();
    std::vector<const StepMotorDescriptor*> minfo = smd->get_motor_info();

    switch (dev_flags) {
        case STEP_MOTOR_DEV_STATUS_IDLE:  dev_status = "Idle"; break;
        case STEP_MOTOR_DEV_STATUS_RUN:   dev_status = "Running"; break;
        case STEP_MOTOR_DEV_STATUS_ERROR: dev_status = "Error"; break;
        default:
            dev_status = "UNKNOWN";
    }

    ui->log("==================================================================================");
    ui->log(tools::str_format("Device status=%s", dev_status.c_str()));
    for (size_t i=0; i<mcount; i++) {
        const char* dir = STEP_MOTOR_DIRECTION(mstatus[i].motor_state) ? "CW" : "CCW";

        std::string motor_power, motor_status, motor_config, motor_microstep;
        motor_power = tools::flags_to_string(mstatus[i].motor_state, motor_state_power_map, " ");
        motor_status = tools::flags_to_string(mstatus[i].motor_state, motor_state_status_map, " | ");
        motor_config = tools::flags_to_string(mstatus[i].motor_state, motor_state_config_map, " | ");
        motor_microstep = tools::flags_to_string(mstatus[i].motor_state, motor_state_microstep_map, ", ");
        std::string comment;

        ui->log("----------------------------------------------------------------------------------");
        ui->log(tools::str_format("#%d %s", i, minfo.at(i)->motor_name));
        ui->log(tools::str_format("Power: %s", motor_power.c_str()));
        ui->log(tools::str_format("Direction: %s", dir));
        comment = (mstatus[i].motor_state & STEP_MOTOR_CWENDSTOP_IN_USE) ? "ignored" : "in use";
        ui->log(tools::str_format("CW software limit: %lld (%s)", mstatus[i].cw_sft_limit, comment.c_str()));
        comment = (mstatus[i].motor_state & STEP_MOTOR_CCWENDSTOP_IN_USE) ? "ignored" : "in use";
        ui->log(tools::str_format("CCW software limit: %lld (%s)", mstatus[i].ccw_sft_limit, comment.c_str()));
        ui->log(tools::str_format("Status: %s", motor_status.c_str()));
        ui->log(tools::str_format("Configuration: %s", motor_config.c_str()));
        ui->log(tools::str_format("Position: %lld", mstatus[i].pos));
        ui->log(tools::str_format("Microstep: %s", motor_microstep.c_str()));
        ui->log(tools::str_format("Buffer: %d bytes", mstatus[i].bytes_remain));
    }
    ui->log("==================================================================================");
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorStartHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorStartHandler,"step_motor::", "::start")
std::string StepMotorStartHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s starts all motors of the %s device. No parameters are required.\n",
                                get_command_name(), 
                                smd->get_dev_name());
}     
void StepMotorStartHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    smd->start();
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorStopHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorStopHandler,"step_motor::", "::stop")
std::string StepMotorStopHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s stops all motors of the %s device. No parameters are required.\n",
                                get_command_name(), 
                                smd->get_dev_name());
}     
void StepMotorStopHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    smd->stop();
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorMoveHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorMoveHandler,"step_motor::", "::move")
std::string StepMotorMoveHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s instructs step motor driver to step N times.\n"
                                "# usage: <mindex>,<value>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"                                    
                                "#        <value> - integer value that specifies amount ot times to step\n",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorMoveHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;        
    check_arg_count(args, 2);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    uint64_t n = arg_unsigned_long_long(args, "value", 1, ULLONG_MAX, {""}, unit, "");
    smd->move(mindex, n);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorMoveNonstopHandler                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorMoveNonstopHandler,"step_motor::", "::move_nonstop")
std::string StepMotorMoveNonstopHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s instructs step motor driver to step indefinitely.\n"
                                "# usage: <mindex>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n",
                                get_command_name(), 
                                smd->get_motor_count()-1);
}     
void StepMotorMoveNonstopHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;        
    check_arg_count(args, 1);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    smd->move(mindex);
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorFeedHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorFeedHandler,"step_motor::", "::feed")
std::string StepMotorFeedHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s feed all enqueued commands to %s device. No parameters are required.\n",
                                get_command_name(),
                                smd->get_dev_name());
}
void StepMotorFeedHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    smd->feed();
}

//----------------------------------------------------------------------------------------------//
//                                    StepMotorSoftwareEndstopHandler                           //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(StepMotorSoftwareEndstopHandler,"step_motor::", "::sft_endstop")
std::string StepMotorSoftwareEndstopHandler::help() const {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    return tools::format_string("# %s Limits motor movement on software level.\n"
                                "# usage: <mindex>,<dir>,<limit>\n"
                                "#        <mindex> - step motor index of the corresponding motor; May be integer from 0 to %d\n"
                                "#        <dir> - direction of the endstop, may be either 'cw' - clockwise or 'ccw' - counter clockwise\n"
                                "#        <value> - end stop value\n",
                                get_command_name(),
                                smd->get_motor_count()-1);
}
void StepMotorSoftwareEndstopHandler::handle(const std::vector<std::string>& args) {
    auto smd = dynamic_cast<StepMotorDev*>(device.get());
    std::string unit;

    check_arg_count(args, 3);
    size_t mindex = arg_unsigned_int(args, "mindex", 0, smd->get_motor_count()-1, {""}, unit, "");
    bool dir = arg_boolean(args, "dir", {"cw"}, {"ccw"});
    int64_t limit = arg_long_long(args, "limit", INT64_MIN, INT64_MAX, {""},unit,"");

    smd->set_software_endstop(mindex, dir, limit);
}

#endif
