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
#include <algorithm>
#include <vector>
#include <libhlek/ekit_error.hpp>
#include <libhlek/info_dev.hpp>
#include <libhlek/deskdev.hpp>
#include <libhlek/irrc.hpp>
#include <libhlek/gpio_dev.hpp>
#include <libhlek/spwm.hpp>
#include <libhlek/adcdev.hpp>
#include <libhlek/ad9850dev.hpp>
#include <libhlek/spidac.hpp>
#include <libhlek/spiproxy.hpp>
#include <libhlek/gsmmodem.hpp>
#include <libhlek/uartdev.hpp>
#include <libhlek/lcd1602a.hpp>
#include <libhlek/rtc.hpp>
#include <libhlek/step_motor.hpp>
#include <libhlek/can.hpp>
// -> INCLUDE_HEADER | HASH: 9BA7E83CC589F7CED899199627E31E91C4ED136F
#include <libhlek/pacemakerdev.hpp>
// -> INCLUDE_HEADER | HASH: 9BA7E83CC589F7CED899199627E31E91C4ED136F
// -> INCLUDE_HEADER | HASH: 9BA7E83CC589F7CED899199627E31E91C4ED136F
#include <libhlek/timetrackerdev.hpp>
// -> INCLUDE_HEADER | HASH: 9BA7E83CC589F7CED899199627E31E91C4ED136F
// INCLUDE_HEADER

#include "handlers.hpp"
#include "monitor.hpp"
#include <math.h>

using namespace LIBCONFIG_NAMESPACE;

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
CommandHandler::CommandHandler(std::shared_ptr<EKitDeviceBase> dev, std::shared_ptr<HLEKMON> _ui) : device(std::move(dev)), ui(std::move(_ui)) {
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

double CommandHandler::arg_angle_to_rad(	double val, const std::string& unit) {
    if (unit=="rad") {
        return val;
    } else
    if (unit=="deg") {
        return val * M_PI / 1.8e2;
    }

    throw CommandHandlerException(tools::format_string("Unknown unit for angle %s", unit));
}

double CommandHandler::arg_frequency_to_hz(	double val, const std::string& unit) {
    if (unit=="khz") {
        return val * 1.0e3;
    } else
    if (unit=="mhz") {
        return val * 1.0e6;
    } else
    if (unit=="ghz") {
        return val * 1.0e9;
    } else
    if (unit=="hz") {
        return val;
    }

    throw CommandHandlerException(tools::format_string("Unknown unit for frequency %s", unit));
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

double CommandHandler::arg_double(  const std::vector<std::string>& args,
                                    const char* name,
                                    double min_val,
                                    double max_val) {
    std::string v = arg_get(args, name);
    double res;
    std::string err;

    // convert to double
    try {
        res = std::stod(v);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %f <= v <= %f)", name, min_val, max_val);
        throw CommandHandlerException(err);
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %f <= v <= %f)", name, min_val, max_val);
        throw CommandHandlerException(err);
    }

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, max_val);
        throw CommandHandlerException(err);
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %f%s <= v <= %f%s)", name, min_val, max_val);
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

std::vector<uint8_t>
CommandHandler::arg_hex_buffer(    const std::vector<std::string>& args,
                               const char* name,
                               size_t min_len,
                               size_t max_len) {
    std::string v = arg_get(args, name);
    auto erase_from = std::remove_if(v.begin(), v.end(), [](char c){return ::isspace(c);});
    v.erase(erase_from, v.end());
    return tools::buffer_from_hex(v);
}

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

unsigned int CommandHandler::arg_unsigned_int(  const std::vector<std::string>& args,
                                                const char* name,
                                                unsigned int min_val,
                                                unsigned int max_val) {
    std::string v = arg_get(args, name);
    unsigned int res;
    std::string err;

    // convert to long
    try {
        res = std::stoul(v, nullptr, 0);
    } catch (std::invalid_argument& e) {
        err = tools::format_string("Invalid value is specified for %s (value must be between: %u <= v <= %u)", name, min_val, max_val);
        throw CommandHandlerException(err);
    }
    catch (std::out_of_range& oore) {
        err = tools::format_string("Invalid value is specified (out of range) for %s (value must be between: %u <= v <= %u)", name, min_val, max_val);
        throw CommandHandlerException(err);
    }

    if (res < min_val) {
        err = tools::format_string("Less than minimal value is specified for %s (value must be between: %u <= v <= %u)", name, min_val, max_val);
        throw CommandHandlerException(err);
    } else if (res > max_val) {
        err = tools::format_string("Greater than maximum value is specified for %s (value must be between: %u <= v <= %u)", name, min_val, max_val);
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
            {INFO_DEV_HINT_GSM_MODEM, "GSM MODEM"},
            {INFO_DEV_HINT_25LC640, "FLASH MEMORY (25LC640)"},
            {INFO_DEV_HINT_ADXL350, "3AXIS ACCELR. SECNSOR (ADXL350)"}};

    std::map<uint8_t, std::string> device_type_map;
    for (size_t i=0; i<INFO_DEVICE_ADDRESSES; i++) {
        const InfoDeviceDescriptor* dev = LIBCONFIG_NAMESPACE::info_config_ptr->devices + i;
        if (dev->type != INFO_DEV_TYPE_NONE) {
            device_type_map[dev->type] = dev->name;
        }
    }

    ui->log(tools::str_format("Project: %s", pname.c_str()));

    size_t li = 0;
    for (size_t i=0; i<INFO_DEVICE_ADDRESSES; i++) {
        const InfoDeviceDescriptor* di = LIBCONFIG_NAMESPACE::info_config_ptr->devices + i;
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

//----------------------------------------------------------------------------------------------//
//                                    UartDevInfo                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(UartDevInfo,"uart::", "::info")
std::string UartDevInfo::help() const {
    return tools::format_string("# %s prints device information. No parameters are required.\n",
                                get_command_name());
}
void UartDevInfo::handle(const std::vector<std::string>& args) {
    auto uart = dynamic_cast<UARTProxyDev*>(device.get());
    const UARTProxyConfig* descr = uart->config;

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
    auto uart = dynamic_cast<UARTProxyDev*>(device.get());

    std::vector<uint8_t> data;
    EKitTimeout to(EKIT_STD_TIMEOUT);
    BusLocker blocker(dynamic_cast<EKitBus*>(uart), to);
    uart->read_all(data, to);

    std::string s = tools::format_buffer(16, data.data(), data.size(), " ", " | ");
    ui->log(s);
}

//----------------------------------------------------------------------------------------------//
//                                    UartDevWrite                                              //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(UartDevWrite,"uart::", "::write")
std::string UartDevWrite::help() const {
    return tools::format_string("# %s writes to device..\n"
                                "# usage: <buffer>\n"
                                "#        <buffer> either text string (will be sent as UTF-8 bytes) or sequence of bytes in hex (for example: \"00 11 22 AB CD EF\")\n",
                                get_command_name());
}
void UartDevWrite::handle(const std::vector<std::string>& args) {
    auto uart = dynamic_cast<UARTProxyDev*>(device.get());
    std::string in = args[0];
    std::vector<uint8_t> data(in.begin(), in.end());

    // figure out if text entered is sequance of bytes by regular expression
    std::unique_ptr<RegexPattern> re = tools::g_unicode_ts.regex_pattern("^([0-9,a-f,A-F]{2})*$", 0);
    bool hex_buffer = tools::g_unicode_ts.regex_match(*re, in);

    if (hex_buffer) {
        data = tools::buffer_from_hex(in);
    }

    EKitTimeout to(EKIT_STD_TIMEOUT);
    BusLocker blocker(dynamic_cast<EKitBus*>(uart), to);
    uart->write(data.data(), data.size(), to);
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
    modem->at(args.at(0), response, status);

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
    modem->sms(arg_get(args, "number"), arg_get(args, "text"), status);
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
    modem->ussd(args.at(0), result, status);
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
    modem->read_sms(messages, status);
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
        modem->delete_sms(-1, status);
    } else {
        unsigned int id = arg_unsigned_int(args, "id", 0, UINT_MAX, {""}, unit, "");
        modem->delete_sms(static_cast<int>(id), status);
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
    modem->dial(arg_get(args, "phone"), status);
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
    modem->active_calls(act_calls, status);

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
            modem->answer(GSM_CALL_ACTION_ANSWER, status);
        } else if (arg=="hang") {
            modem->answer(GSM_CALL_ACTION_HANG, status);
        } else if (arg=="hold") {
            modem->answer(GSM_CALL_ACTION_HOLD, status);
        } else if (arg=="release") {
            modem->answer(GSM_CALL_ACTION_RELEASE, status);
        } else {
            throw CommandHandlerException("Invalid argument specified for action (valid values are: answer, hang, hold, release)");
        }

        ui->log("[status=" + GSMModem::status_description(status) + "]");            
    }    
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
//----------------------------------------------------------------------------------------------//
//                                    GPIOHandler                                               //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(GPIOHandler,"gpio::", "::sync")
std::string GPIOHandler::help() const {
    auto gpiodev = dynamic_cast<GPIODev*>(device.get());
    return tools::format_string("# %s synchronizes with GPIO device.\n"
                                "# usage:  <state> or no parameters\n"
                                "#         <state> to be set. state should be a sequance of 0 and 1 (%u bits total) to set corresponding gpio:\n"
                                "#         if no parameters given gpio is read and reported\n",
                                get_command_name(),
                                gpiodev->config->pin_number);
}
void GPIOHandler::handle(const std::vector<std::string>& args) {

    size_t arg_c = args.size();
    auto gpiodev = dynamic_cast<GPIODev*>(device.get());
    bool bad_param = false;
    std::vector<bool> bits(gpiodev->config->pin_number, false);
    std::string p = args[0];
    std::string t;
    size_t arg_len = p.length();

    if (arg_c==1 && arg_len==0) {
        gpiodev->read(bits);
        std::string t;
        for (auto b=bits.begin(); b!=bits.end(); ++b)
            t+= *b ? "1" : "0";
        ui->log(tools::str_format("READ: %s", t.c_str()));
    } else if (arg_c==2 && arg_len==gpiodev->config->pin_number) {
        for (size_t i=0; i<gpiodev->config->pin_number; i++) {
            if (p[i]=='1') {
                bits[gpiodev->config->pin_number - i - 1] = true;
            } else if (p[i]!='0') {
                throw CommandHandlerException(tools::format_string( "Invalid argument specified for state (valid value should be a sequance of 0 and 1, %d symbols in length)",
                                                                    gpiodev->config->pin_number));
            }
        }

        gpiodev->write(bits);
        std::string t;
        for (auto b=bits.begin(); b!=bits.end(); ++b)
            t+= *b ? "1" : "0";
        ui->log(tools::str_format("SET: %s", t.c_str()));
    }
}

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

        const SPWMChannel* cdescr = spwm->get_channel_info(channel_index);
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
//                                    SPWMSetFreqHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPWMSetFreqHandler,"spwm::", "::freq")
std::string SPWMSetFreqHandler::help() const {
    return tools::format_string("# %s Sets SPWM frequency.\n"
                                "# usage:  <value> value of the output. It should be floating point value.\n",
                                get_command_name());
}
void SPWMSetFreqHandler::handle(const std::vector<std::string>& args) {
    auto spwm = dynamic_cast<SPWMDev*>(device.get());
    check_arg_count(args, 1);
    double val;

    try {
        val = std::stod(args[1]);
    } catch (std::invalid_argument& e) {
        std::string err = tools::format_string("Invalid parameter specified, must be floating point value");
        throw CommandHandlerException(err);
    }

    spwm->set_pwm_freq(val);
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

//----------------------------------------------------------------------------------------------//
//                                    ADCDevStatusHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevStatusHandler,"adc::", "::status")
std::string ADCDevStatusHandler::help() const {
    return tools::format_string("# %s Prints ADC status. No parameters are required.\n",
                                get_command_name());
}
void ADCDevStatusHandler::handle(const std::vector<std::string>& args) {

    static const std::map<uint16_t , std::pair<std::string, std::string>> adc_status_flags = {
            {ADCDEV_STATUS_STARTED,      {"STARTED",    "STOPPED"}},
            {ADCDEV_STATUS_UNSTOPPABLE,  {"UNSTOPPABLE","LIMITED"}},
            {ADCDEV_STATUS_TOO_FAST,     {"TOO_FAST",   ""}},
            {ADCDEV_STATUS_SAMPLING,     {"SAMPLING",   "NOT_SAMPLING"}}};


    auto adc = dynamic_cast<ADCDev*>(device.get());
    check_arg_count(args, 0);
    uint16_t flags = 0;
    size_t sample_count = adc->status(flags);
    std::string text_status = tools::flags_to_string(flags, adc_status_flags, " ");
    ui->log(tools::format_string("%s status: %s", adc->get_dev_name(), text_status));
    ui->log(tools::format_string("%d samples in internal buffer.", sample_count));

}

//----------------------------------------------------------------------------------------------//
//                                    ADCDevStartHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevStartHandler,"adc::", "::start")
std::string ADCDevStartHandler::help() const {
    return tools::format_string("# %s Starts ADC conversion.\n"
                                "# usage: <count>,<period><unit>\n"
                                "#        <count> number of samples to sample\n"
                                "# note: actual delay between samples may be inaccurate, especially if very little delays specified\n",
                                get_command_name());
}
void ADCDevStartHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    std::string unit;
    check_arg_count(args, 1);
    size_t sample_count = arg_int(args, "count", 0, USHRT_MAX, {""}, unit, "");
    adc->start(sample_count);
}

//----------------------------------------------------------------------------------------------//
//                                    ADCDevConfigHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevConfigHandler,"adc::", "::configure")
std::string ADCDevConfigHandler::help() const {
    return tools::format_string("# %s Configures ADC conversion.\n"
                                "# usage: <count>,<period><unit>,<sample time>\n"
                                "#        <count> number of samples to be averaged per sample\n"
                                "#        <period> time period between samples\n"
                                "#        <unit> 'us' - microseconds, 'ms' - milliseconds, 's' - seconds\n"
                                "#        <sample time> - sample time, integers in range [0, 7]. Single value for all channels.\n"
                                "#                        See ADC_SampleTime_<N>Cycles<M> constants in CMSIS library.\n"
                                "# note: actual delay between samples may be inaccurate, especially if very little delays specified\n",
                                get_command_name());
}
void ADCDevConfigHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    std::string unit;
    check_arg_count(args, 3);
    size_t measurements_per_sample = arg_int(args, "count", 0, USHRT_MAX, {""}, unit, "");
    double delay_f = arg_double(args, "period", 0.0, DBL_MAX, {"us", "ms", "s"}, unit, "s");
    delay_f = arg_time_to_sec(delay_f, unit);
    size_t sample_time = arg_int(args, "sample time", 0, 7, {""}, unit, "");
    std::map<size_t, uint8_t> sampling_times;
    size_t num_channels = adc->get_input_count();
    for (size_t ch=0; ch<num_channels; ch++) {
        sampling_times[ch] = sample_time;
    }

    adc->configure(delay_f, measurements_per_sample, sampling_times);
}


//----------------------------------------------------------------------------------------------//
//                                    ADCDevStopHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevStopHandler,"adc::", "::stop")
std::string ADCDevStopHandler::help() const {
    return tools::format_string("# %s stops ADC conversion. No arguments are required.\n",
                                get_command_name());
}
void ADCDevStopHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    size_t argc = check_arg_count(args, 0);
    adc->stop();
}


//----------------------------------------------------------------------------------------------//
//                                    ADCDevResetHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(ADCDevResetHandler,"adc::", "::reset")
std::string ADCDevResetHandler::help() const {
    return tools::format_string("# %s resets ADC data. No arguments are required.\n",
                                get_command_name());
}
void ADCDevResetHandler::handle(const std::vector<std::string>& args) {
    auto adc = dynamic_cast<ADCDev*>(device.get());
    size_t argc = check_arg_count(args, 0);
    adc->reset();
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
    adc->get(data);

    // print data
    size_t sample_count = data.size();
    for (size_t ch=0; ch<channel_count; ch++) {
        std::string l = adc->get_input_name(ch, false) + ":";
        for (size_t s=0; s<sample_count; s++) {
            l += " " + std::to_string(data[s][ch]);
        }
        ui->log(l);
    }
}

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


//----------------------------------------------------------------------------------------------//
//                                    CanInfoHandler                                        //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanInfoHandler,"can::", "::info")
std::string CanInfoHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s shows information for %s device. No parameters are required.\n",
                                get_command_name(), 
                                d->get_dev_name());
}

void CanInfoHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    ui->log(tools::str_format("Not implemented"));
}

//----------------------------------------------------------------------------------------------//
//                                    CanStartHandler                                           //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanStartHandler,"can::", "::start")
std::string CanStartHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Starts CAN communication on %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanStartHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count(args, 0);
    d->can_start();
}

//----------------------------------------------------------------------------------------------//
//                                    CanStopHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanStopHandler,"can::", "::stop")
std::string CanStopHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Stops CAN communication on %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanStopHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count(args, 0);
    d->can_stop();
}

//----------------------------------------------------------------------------------------------//
//                                    CanStatusHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanStatusHandler,"can::", "::status")
std::string CanStatusHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Returns CAN status for %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanStatusHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count(args, 0);
    CanStatus status = {0};
    d->can_status(status);
    ui->log(CanDev::can_status_to_str(status));
}

//----------------------------------------------------------------------------------------------//
//                                    CanSendHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanSendHandler,"can::", "::send")
std::string CanSendHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Sends a message over CAN bus using %s device. The following format is used:\n"
                                "ER <id> <eid> <D0> <D8>\n"
                                "where:\n"
                                "E - optional specifier to use extended message (if not specified eid is not used)\n"
                                "R - optional specifier to use remote frame.\n"
                                "<id> - mandatory message identifier (standard or extended id).\n"
                                "<D0> ... <D8> - data bytes.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanSendHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count_min(args, 1);
    std::string er = arg_get(args, "param0");

    bool extended = ((er.find('e')!=std::string::npos) || (er.find('E')!=std::string::npos));
    bool remote_frame = ((er.find('r')!=std::string::npos) || (er.find('R')!=std::string::npos));

    if (false == (extended || remote_frame)) {
        // first parameter must be a number
        arg_index--;
    }

    // Get identifiers
    uint32_t id = static_cast<uint32_t>(arg_unsigned_int(args, "id", 0, 0xFFFFFFFF));

    // Get data
    size_t params_left = args.size() - arg_index;
    std::vector<uint8_t> data;
    for (size_t i=0; i<params_left; i++) {
        std::string var_name = std::string("D") + std::to_string(i);
        unsigned int x = arg_unsigned_int(args, var_name.c_str(), 0, 0xFF);
        data.push_back(static_cast<uint8_t>(x));
    }

    d->can_send(id, data, remote_frame, extended);
}

//----------------------------------------------------------------------------------------------//
//                                    CanFilterHandler                                          /
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanFilterHandler,"can::", "::filter")
std::string CanFilterHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Installs receiver CAN filter for %s device.  The following format is used:\n"
                                "<options> <index> <id> <mask>\n"
                                "where:\n"
                                "<options> - optional options for the filter. May be set of these letters:\n"
                                "    D - specifies to disable filter. All filters by default are disabled.\n"
                                "    M - specifies mask mode, if not set list mode is used.\n"
                                "    F - if specified instructs to use FIFO 1, otherwise FIFO 0 is used.\n"
                                "    W - specifies 32 bit word scaling, if not set 16 bit scaling is used.\n"
                                "        16 bit scaling specifies to split id and mask on two parts so we have two filters.\n"
                                "    E - Instructs to use extended frame.\n"
                                "<id> - mandatory message identifier.\n"
                                "<mask> - mandatory mask field.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanFilterHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count_min(args, 3);
    std::string opts = arg_get(args, "options");

    bool disabled  = (opts.find('d')!=std::string::npos) || (opts.find('D')!=std::string::npos);
    bool fifo1     = (opts.find('f')!=std::string::npos) || (opts.find('F')!=std::string::npos);
    bool word_scaling = (opts.find('w')!=std::string::npos) || (opts.find('W')!=std::string::npos);
    bool mask_mode = (opts.find('m')!=std::string::npos) || (opts.find('M')!=std::string::npos);
    bool extended  = (opts.find('e')!=std::string::npos) || (opts.find('E')!=std::string::npos);

    if (!(disabled || fifo1 || word_scaling || mask_mode || extended)) {
        arg_index--; // first parameter must be skipped.
    }

    // Get identifiers
    uint32_t id_0, id_1, id_2, id_3;
    uint8_t index = static_cast<uint8_t>(arg_unsigned_int(args, "index", 0, CAN_FLT_MAX_INDEX));
    id_0 = static_cast<uint32_t>(arg_unsigned_int(args, "id", 0, 0xFFFFFFFF));
    id_1 = static_cast<uint32_t>(arg_unsigned_int(args, "id/mask", 0, 0xFFFFFFFF));

    if (extended) {
        d->can_filter_ext(!disabled, index, id_0, id_1, fifo1, mask_mode);
    } else if (word_scaling) {
        d->can_filter_std_32(!disabled, index, id_0, id_1, fifo1, mask_mode);
    } else {
        id_2 = static_cast<uint32_t>(arg_unsigned_int(args, "id", 0, 0xFFFFFFFF));
        id_3 = static_cast<uint32_t>(arg_unsigned_int(args, "id/mask", 0, 0xFFFFFFFF));
        d->can_filter_std(!disabled,
                             index,
                             static_cast<uint16_t>(id_0),
                             static_cast<uint16_t>(id_1),
                             static_cast<uint16_t>(id_2),
                             static_cast<uint16_t>(id_3),
                             fifo1,
                             mask_mode);
    }
}

//----------------------------------------------------------------------------------------------//
//                                    CanReadHandler                                            //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(CanReadHandler,"can::", "::read")
std::string CanReadHandler::help() const {
    auto d = dynamic_cast<CanDev*>(device.get());
    return tools::format_string("# %s Reads messages received by CAN and status for %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void CanReadHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<CanDev*>(device.get());
    check_arg_count(args, 0);
    CanStatus status = {0};
    std::vector<CanRecvMessage> messages;
    d->can_read(status, messages);

    ui->log(CanDev::can_status_to_str(status));
    for (auto m : messages) {
        ui->log(CanDev::can_msg_to_str(m));
    }
}

//----------------------------------------------------------------------------------------------//
//                                    SPIProxyInfoHandler                                       //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIProxyInfoHandler,"spiproxy::", "::info")
std::string SPIProxyInfoHandler::help() const {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());
    return tools::format_string("# %s shows information for %s device. No parameters are required.\n",
                                get_command_name(), 
                                d->get_dev_name());
}

void SPIProxyInfoHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());
    ui->log(tools::str_format("Not implemented"));
}

//----------------------------------------------------------------------------------------------//
//                                    SPIProxyReadHandler                                       //
//----------------------------------------------------------------------------------------------//

DEFINE_HANDLER_DEFAULT_IMPL(SPIProxyReadHandler,"spiproxy::", "::read")
std::string SPIProxyReadHandler::help() const {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());
    return tools::format_string("# %s reads data from %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIProxyReadHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());

    std::vector<uint8_t> data;
    EKitTimeout to(EKIT_STD_TIMEOUT);
    EKIT_ERROR err = d->read_all(data, to);

    if (err!=EKIT_OK) {
        ui->log(tools::format_string("Error occured: %s", errname(err)));
    }

    std::string s = tools::format_buffer(16, data.data(), data.size(), " ", " | ");
    ui->log(s);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIProxyWriteHandler                                       //
//----------------------------------------------------------------------------------------------//

DEFINE_HANDLER_DEFAULT_IMPL(SPIProxyWriteHandler,"spiproxy::", "::write")
std::string SPIProxyWriteHandler::help() const {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());
    return tools::format_string("# %s writes data into %s device. Pass array of bytes separated by space.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIProxyWriteHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIProxyDev*>(device.get());

    check_arg_count_min(args, 1);
    std::vector<uint8_t> data = arg_hex_buffer(args, "buffer", 0, std::numeric_limits<uint16_t>::max());

    EKitTimeout to(EKIT_STD_TIMEOUT);
    EKIT_ERROR err = d->write(data.data(), data.size(), to);
    if (err != EKIT_OK) {
        ui->log(tools::str_format("Failed with: %d", err));
    } else {
        ui->log(tools::str_format("OK"));
    }
}

//----------------------------------------------------------------------------------------------//
//                                    AD9850DevResetHandler                                     //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(AD9850DevResetHandler,"ad9850dev::", "::reset")
std::string AD9850DevResetHandler::help() const {
    auto d = dynamic_cast<AD9850Dev*>(device.get());
    return tools::format_string("# %s resets %s device. No parameters are required.\n",
                                get_command_name(), 
                                d->get_dev_name());
}

void AD9850DevResetHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<AD9850Dev*>(device.get());
    d->reset();
}

//----------------------------------------------------------------------------------------------//
//                                    AD9850DevUpdateHandler                                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(AD9850DevUpdateHandler,"ad9850dev::", "::update")
std::string AD9850DevUpdateHandler::help() const {
    auto d = dynamic_cast<AD9850Dev*>(device.get());
    return tools::format_string("# %s set frequency and phase for %s device. Requires two parameters:\n"
                                "<frequency> - frequency in Hz\n"
                                "<phase> - phase in radians\n",
                                get_command_name(),
                                d->get_dev_name());
}

void AD9850DevUpdateHandler::handle(const std::vector<std::string>& args) {
    std::string unit;
    auto d = dynamic_cast<AD9850Dev*>(device.get());
    size_t argc = check_arg_count_min(args, 1);

    double frequency = arg_double(args, "frequency", 0.0, DBL_MAX, {"khz", "mhz", "hz"}, unit, "hz");
    frequency = arg_frequency_to_hz(frequency, unit);

    double phase = 0.0L;
    if (argc>1) {
        arg_double(args, "phase", -DBL_MAX, DBL_MAX, {"rad", "deg"}, unit, "deg");
        phase = arg_angle_to_rad(phase, unit);
    }
    d->update(frequency, phase);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACStartContinuousHandler                              //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACStartContinuousHandler,"spidac::", "::start_continuous")
std::string SPIDACStartContinuousHandler::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s starts %s device.\n"
                                "Parameters are:\n"
                                "Sampling rate\n"
                                "Phase increment\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACStartContinuousHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    check_arg_count(args, 2);
    std::string unit;

    double frequency = arg_double(args, "sampling frequency", 0.0, DBL_MAX, {"khz", "mhz", "hz"}, unit, "hz");
    frequency = arg_frequency_to_hz(frequency, unit);

    unsigned int phase_inc = arg_unsigned_int(args, "phase increment", 1, UINT16_MAX); // <!CHECKIT!> check if the same macroes are used in whole project

    d->start_signal(frequency, phase_inc, true);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACStartPeriodHandler                                  //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACStartPeriodHandler,"spidac::", "::start_period")
std::string SPIDACStartPeriodHandler::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s starts %s device.\n"
                                "Parameters are:\n"
                                "Sampling rate\n"
                                "Phase increment\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACStartPeriodHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    check_arg_count(args, 2);
    std::string unit;

    double frequency = arg_double(args, "sampling frequency", 0.0, DBL_MAX, {"khz", "mhz", "hz"}, unit, "hz");
    frequency = arg_frequency_to_hz(frequency, unit);

    unsigned int phase_inc = arg_unsigned_int(args, "phase increment", 1, UINT16_MAX); // <!CHECKIT!> check if the same macroes are used in whole project

    d->start_signal(frequency, phase_inc, false);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACSetDefaultHandler                                   //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACSetDefaultHandler,"spidac::", "::set_default")
std::string SPIDACSetDefaultHandler::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());

    size_t channel_count = d->get_channels_count();

    return tools::format_string("# %s starts %s device.\n"
                                "Parameters are:\n"
                                "%d comma separated values per each channel\n",
                                get_command_name(),
                                d->get_dev_name(),
                                channel_count);
}

void SPIDACSetDefaultHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    size_t channel_count = d->get_channels_count();
    check_arg_count(args, channel_count);
    std::string unit;
    SPIDAC_CHANNELS_VALUE_RANGE vr = d->get_value_range();
    SPIDAC_SAMPLE_VECT data;

    for (size_t i=0; i<channel_count; i++) {
        std::string descr = tools::format_string("Default value for channel %d", i);
        double value = arg_double(args, descr.c_str(), vr.at(i).first, vr.at(i).second);
        data.push_back(value);
    }

    d->set_default_values(data);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACStopHandler                                         //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACStopHandler,"spidac::", "::stop")
std::string SPIDACStopHandler::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s stops %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACStopHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    d->stop();
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACIsRunningHandler                                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACIsRunningHandler,"spidac::", "::is_running")
std::string SPIDACIsRunningHandler::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s checks if %s device is running. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACIsRunningHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    bool res = d->is_running();
    std::string tres = res ? "Device is running." : "Device is stopped.";
    ui->log(tres);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACUploadSinWaveform                                   //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACUploadSinWaveform,"spidac::", "::upload_sin")
std::string SPIDACUploadSinWaveform::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s uploads sin waveform to %s device.\n"
                                "Parameters are:\n"
                                "Number of samples to use (optional, by default all possible samples are used)\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACUploadSinWaveform::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    size_t argc = check_arg_count_min(args, 0);

    size_t sample_count = d->get_max_samples_per_channel();
    size_t channel_count = d->get_channels_count();
    SPIDAC_CHANNELS_VALUE_RANGE ranges = d->get_value_range();
    if (argc!=0) {
        sample_count = arg_unsigned_int(args, "samples count", 0, UINT_MAX);
    }

    SPIDAC_CHANNELS channels;
    for (size_t ch = 0; ch<channel_count; ch++) {
        SPIDAC_SAMPLE_VECT samples;
        double min_val = ranges.at(ch).first;
        double max_val = ranges.at(ch).second;

        for (size_t s = 0; s<sample_count; s++) {
            double x = 2.0L * M_PI * (double)s/(double)sample_count;
            double val = ((sin(x) + 1.0L) * (max_val - min_val) / 2.0L) + min_val;
            samples.push_back(val);
        }
        channels.push_back(samples);
    }

    d->upload(channels, false);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACUploadSawWaveform                                   //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACUploadSawWaveform,"spidac::", "::upload_saw")
std::string SPIDACUploadSawWaveform::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s uploads saw waveform to %s device.\n"
                                "Parameters are:\n"
                                "Number of samples to use (optional, by default all possible samples are used)\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACUploadSawWaveform::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    size_t argc = check_arg_count_min(args, 0);

    size_t sample_count = d->get_max_samples_per_channel();
    size_t channel_count = d->get_channels_count();
    SPIDAC_CHANNELS_VALUE_RANGE ranges = d->get_value_range();
    if (argc!=0) {
        sample_count = arg_unsigned_int(args, "samples count", 0, UINT_MAX);
    }

    SPIDAC_CHANNELS channels;
    for (size_t ch = 0; ch<channel_count; ch++) {
        SPIDAC_SAMPLE_VECT samples;
        double min_val = ranges.at(ch).first;
        double max_val = ranges.at(ch).second;

        for (size_t s = 0; s<sample_count; s++) {
            double x = (double)s/(double)sample_count;
            double val = (x * (max_val - min_val)) + min_val;
            samples.push_back(val);
        }
        channels.push_back(samples);
    }

    d->upload(channels, false);
}

//----------------------------------------------------------------------------------------------//
//                                    SPIDACUploadTriangleWaveform                              //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(SPIDACUploadTriangleWaveform,"spidac::", "::upload_triangle")
std::string SPIDACUploadTriangleWaveform::help() const {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    return tools::format_string("# %s uploads triangle waveform to %s device.\n"
                                "Parameters are:\n"
                                "Number of samples to use (optional, by default all possible samples are used)\n",
                                get_command_name(),
                                d->get_dev_name());
}

void SPIDACUploadTriangleWaveform::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<SPIDACDev*>(device.get());
    size_t argc = check_arg_count_min(args, 0);

    size_t sample_count = d->get_max_samples_per_channel();
    size_t channel_count = d->get_channels_count();
    SPIDAC_CHANNELS_VALUE_RANGE ranges = d->get_value_range();
    if (argc!=0) {
        sample_count = arg_unsigned_int(args, "samples count", 0, UINT_MAX);
    }

    SPIDAC_CHANNELS channels;
    for (size_t ch = 0; ch<channel_count; ch++) {
        SPIDAC_SAMPLE_VECT samples;
        double min_val = ranges.at(ch).first;
        double max_val = ranges.at(ch).second;

        size_t middle = sample_count / 2;
        for (size_t s = 0; s<middle; s++) {
            double x = (double)s/(double)middle;
            double val = (x * (max_val - min_val)) + min_val;
            samples.push_back(val);
        }

        for (size_t s = middle; s<sample_count; s++) {
            double x = (double)s/(double)middle;
            double val = ((2.0L - x) * (max_val - min_val)) + min_val;
            samples.push_back(val);
        }

        channels.push_back(samples);
    }

    d->upload(channels, false);
}

// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
//----------------------------------------------------------------------------------------------//
//                                    PaceMakerDevInfoHandler                                   //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(PaceMakerDevInfoHandler,"pacemakerdev::", "::info")
std::string PaceMakerDevInfoHandler::help() const {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    return tools::format_string("# %s shows information for %s device. No parameters are required.\n",
                                get_command_name(), 
                                d->get_dev_name());
}

void PaceMakerDevInfoHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    PaceMakerStatus status;
    d->status(status);

    ui->log(tools::format_string("%s status:", d->get_dev_name()));
    ui->log(tools::format_string("started: %i", status.started));
    ui->log(tools::format_string("last_error: %i  (%s)", status.last_error, errname(status.last_error)));
    ui->log(tools::format_string("main_counter: %i:", status.main_counter));
    ui->log(tools::format_string("internal_index: %d", status.internal_index));
}

//----------------------------------------------------------------------------------------------//
//                                    PaceMakerDevStartHandler                                  //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(PaceMakerDevStartHandler,"pacemakerdev::", "::start")
std::string PaceMakerDevStartHandler::help() const {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    return tools::format_string("# %s starts signals generation for %s device. No parameters are required.\n"
                                "#        <count> - Number of main cycles, 0 to run indefinitely, until stopped. \n"
                                "#        <frequency> - frequency of the main cycle. \n",
                                get_command_name(),
                                d->get_dev_name());
}

void PaceMakerDevStartHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());


    std::string unit;
    check_arg_count(args, 2);
    size_t main_cycles_count = arg_int(args, "count", 0, USHRT_MAX, {""}, unit, "");
    double frequency = arg_double(args, "sampling frequency", 0.0, DBL_MAX, {"khz", "mhz", "hz"}, unit, "hz");
    frequency = arg_frequency_to_hz(frequency, unit);

    d->start(frequency, main_cycles_count);
}

//----------------------------------------------------------------------------------------------//
//                                    PaceMakerDevStopHandler                                   //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(PaceMakerDevStopHandler,"pacemakerdev::", "::stop")
std::string PaceMakerDevStopHandler::help() const {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    return tools::format_string("# %s stops signals generation for %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void PaceMakerDevStopHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    d->stop();
}

//----------------------------------------------------------------------------------------------//
//                                    PaceMakerDevResetHandler                                  //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(PaceMakerDevResetHandler,"pacemakerdev::", "::reset")
std::string PaceMakerDevResetHandler::help() const {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    return tools::format_string("# %s reset signals generation for %s device. No parameters are required.\n",
                                get_command_name(),
                                d->get_dev_name());
}

void PaceMakerDevResetHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    d->reset();
}

//----------------------------------------------------------------------------------------------//
//                                    PaceMakerDevSetDataHandler                    //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(PaceMakerDevSetDataHandler,"pacemakerdev::", "::set_data")
std::string PaceMakerDevSetDataHandler::help() const {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());
    return tools::format_string("# %s adds signal transition for %s device.\n"
                                "Parameters are:\n"
                                "#        <period> - Some time quantum for signal structur \n",
                                get_command_name(),
                                d->get_dev_name());
}

void PaceMakerDevSetDataHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<PaceMakerDev*>(device.get());

    std::string unit;
    check_arg_count(args, 1);
    double quant = arg_double(args, "quant", 0.0, DBL_MAX, {"us", "ms", "s"}, unit, "s");
    quant = arg_time_to_sec(quant, unit);
    double inter_test_delay = quant * 10.0L;


    uint32_t affected_signals = d->all_signals_mask() ^ 1;  // Signal 1 is for main clock tracking

    d->reset_signals();
    d->add_flip(quant, 1);

    // Flip twice
    d->add_flip(quant, affected_signals);
    d->add_flip(quant, affected_signals);

    // Pwm and clock (it's the same)
    d->add_pwm(inter_test_delay, quant, 0.33, 3, affected_signals);
    d->add_default(inter_test_delay);

    d->set_data();
}
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
//----------------------------------------------------------------------------------------------//
//                                    TimeTrackerDevStartHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(TimeTrackerDevStartHandler,"timetrackerdev::", "::start")
std::string TimeTrackerDevStartHandler::help() const {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    auto cn = get_command_name();
    auto dn = d->get_dev_name();
    return tools::format_string("# %s starts %s device. No parameters are required.\n",
                                cn.c_str(),
                                dn.c_str());
}

void TimeTrackerDevStartHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    d->start();
}

//----------------------------------------------------------------------------------------------//
//                                    TimeTrackerDevStopHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(TimeTrackerDevStopHandler,"timetrackerdev::", "::stop")
std::string TimeTrackerDevStopHandler::help() const {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    auto cn = get_command_name();
    auto dn = d->get_dev_name();
    return tools::format_string("# %s stops %s device. No parameters are required.\n",
                                cn.c_str(),
                                dn.c_str());
}

void TimeTrackerDevStopHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    d->stop();
}

//----------------------------------------------------------------------------------------------//
//                                    TimeTrackerDevReadHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(TimeTrackerDevReadHandler,"timetrackerdev::", "::read")
std::string TimeTrackerDevReadHandler::help() const {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    auto cn = get_command_name();
    auto dn = d->get_dev_name();
    return tools::format_string("# %s reads all timestampts from %s device. No parameters are required.\n",
                                cn.c_str(),
                                dn.c_str());
}

void TimeTrackerDevReadHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    std::vector<double> data;
    d->read_all(data, true);
    for (auto d=data.begin(); d!=data.end(); ++d) {
        ui->log(tools::str_format("%f sec",*d));
    }
}

//----------------------------------------------------------------------------------------------//
//                                    TimeTrackerDevStatusHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(TimeTrackerDevStatusHandler,"timetrackerdev::", "::status")
std::string TimeTrackerDevStatusHandler::help() const {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    auto cn = get_command_name();
    auto dn = d->get_dev_name();
    return tools::format_string("# %s prints device status. No parameters are required.\n",
                                cn.c_str(),
                                dn.c_str());
}

void TimeTrackerDevStatusHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    bool running;
    uint64_t first_ts;
    size_t n = d->get_status(running, first_ts);
    std::string dn = d->get_dev_name();

    if (running) {
        ui->log(tools::str_format("Device %s is RUNNING.", dn.c_str()));
    } else {
        ui->log(tools::str_format("Device %s is STOPPED.", dn.c_str()));
    }

    ui->log(tools::str_format("%d timestamps are stored in the buffer.", n));
    ui->log(tools::str_format("First timestamp is %f", (double)first_ts/(double)d->config->tick_freq));
}

//----------------------------------------------------------------------------------------------//
//                                    TimeTrackerDevResetHandler                                //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL(TimeTrackerDevResetHandler,"timetrackerdev::", "::reset")
std::string TimeTrackerDevResetHandler::help() const {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    auto cn = get_command_name();
    auto dn = d->get_dev_name();
    return tools::format_string("# %s Reset %s device circular buffer. No parameters are required.\n",
                                cn.c_str(),
                                dn.c_str());
}

void TimeTrackerDevResetHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<TimeTrackerDev*>(device.get());
    d->reset();
}

// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
// ADD_DEVICE