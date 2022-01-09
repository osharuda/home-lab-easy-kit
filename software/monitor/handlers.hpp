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
 *   \brief Firmware to software communication test utility header
 *   \author Oleh Sharuda
 */

#pragma once
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "info_dev.hpp"
#include "gsmmodem.hpp"
#include "lcd1602a.hpp"
#include "deskdev.hpp"
#include "irrc.hpp"
#include "rtc.hpp"
#include "gpio_dev.hpp"
#include "spwm.hpp"
#include "adcdev.hpp"
#include "step_motor.hpp"
#include "uartdev.hpp"
#include "can.hpp"
// INCLUDE_HEADER

/// \addtogroup group_monitor
/// @{

class CommandHandlerException : public std::runtime_error {
public:

	explicit CommandHandlerException(const std::string& description);
	~CommandHandlerException() override;
};

class HLEKMON;

class CommandHandler {
	protected:
	size_t arg_index;    
    std::shared_ptr<EKitDeviceBase> device;
    std::shared_ptr<HLEKMON> ui;

    public:
    CommandHandler() = delete;
    CommandHandler(std::shared_ptr<EKitDeviceBase> dev, std::shared_ptr<HLEKMON> _ui);
    virtual ~CommandHandler();
    virtual void handle(const std::vector<std::string>& args) = 0;
    virtual std::string get_command_name() const = 0;
    virtual std::string help() const;
    void        arg_reset();
    // param processors:
	protected:

    size_t      check_arg_count(const std::vector<std::string>& args, size_t expected) const;
    size_t      check_arg_count_min(const std::vector<std::string>& args, size_t min) const;
    std::string arg_get(const std::vector<std::string>& args, const char* name);
    static void arg_options_check(const std::set<std::string> &opts, const std::set<std::string>& allowed_opts);
    bool        arg_boolean(const std::vector<std::string>& args, 
    						const char* name, 
    						const std::list<std::string>& truevals, 
    						const std::list<std::string>& falsevals);

    std::string arg_unit(	const std::vector<std::string>& args, 
    						const char* name, 
    						const std::list<std::string>& allowed_units, 
    						std::string& unit);

    static double arg_time_to_sec(	double val,
    								const std::string& unit);

    double      arg_double(	const std::vector<std::string>& args, 
    						const char* name, 
    						double min_val, 
    						double max_val, 
    						const std::list<std::string>& allowed_units, 
    						std::string& unit, 
    						const char* default_unit);

    int         arg_int(	const std::vector<std::string>& args, 
    						const char* name, 
    						int min_val, 
    						int max_val, 
    						const std::list<std::string>& allowed_units, 
    						std::string& unit, 
    						const char* default_unit);

    unsigned int 	arg_unsigned_int(	const std::vector<std::string>& args,
		    							const char* name, 
		    							unsigned int min_val, 
		    							unsigned int max_val, 
		    							const std::list<std::string>& allowed_units, 
		    							std::string& unit, 
		    							const char* default_unit);

    unsigned int arg_unsigned_int(  const std::vector<std::string>& args,
                                    const char* name,
                                    unsigned int min_val,
                                    unsigned int max_val);

    long long   arg_long_long(	const std::vector<std::string>& args, 
    							const char* name, 
    							long long min_val, 
    							long long max_val, 
    							const std::list<std::string>& allowed_units, 
    							std::string& unit, 
    							const char* default_unit);

    unsigned long long   arg_unsigned_long_long(	const std::vector<std::string>& args, 
    												const char* name, 
    												unsigned long long min_val, 
    												unsigned long long max_val, 
    												const std::list<std::string>& allowed_units, 
    												std::string& unit, 
    												const char* default_unit);
};

#define DEFINE_HANDLER_CLASS(classname)                                           \
	class classname : public CommandHandler {                                     \
		typedef CommandHandler super;                                             \
	public:                                                                       \
		classname() = delete;                                                     \
		classname(std::shared_ptr<EKitDeviceBase> dev, std::shared_ptr<HLEKMON> _ui);    \
		virtual ~classname();                                                     \
		virtual void handle(const std::vector<std::string>& args);                \
		virtual std::string help() const;                                         \
		virtual std::string get_command_name() const;                             \
	}

#define DEFINE_HANDLER_DEFAULT_IMPL(classname, cmd_prefix, cmd_suffix)                                         \
    classname::classname(std::shared_ptr<EKitDeviceBase> dev, std::shared_ptr<HLEKMON> _ui) : super (std::move(dev), std::move(_ui)) {}     \
    classname::~classname(){}                                                                                  \
    std::string classname::get_command_name() const {                                                          \
        return cmd_prefix + device->get_dev_name() + cmd_suffix;                                               \
    }

#ifdef INFO_DEVICE_ENABLED
    DEFINE_HANDLER_CLASS(InfoDevHandler);
#endif

#ifdef UART_PROXY_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(ATHandler);
	DEFINE_HANDLER_CLASS(SMSHandler);
	DEFINE_HANDLER_CLASS(ReadSMSHandler);
	DEFINE_HANDLER_CLASS(DeleteSMSHandler);
	DEFINE_HANDLER_CLASS(USSDHandler);
	DEFINE_HANDLER_CLASS(DialHandler);
	DEFINE_HANDLER_CLASS(ActiveCallsHandler);
	DEFINE_HANDLER_CLASS(AnswerCallHandler);

	DEFINE_HANDLER_CLASS(UartDevInfo);
    DEFINE_HANDLER_CLASS(UartDevRead);
    DEFINE_HANDLER_CLASS(UartDevWrite);
#endif 

#ifdef LCD1602a_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(LCDPrintHandler);
	DEFINE_HANDLER_CLASS(LCDLightHandler);
#endif


#ifdef DESKDEV_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(DeskDevStatusHandler);
#endif


#ifdef IRRC_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(IRRCHandler);
#endif

#ifdef RTC_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(RTCGetHandler);
	DEFINE_HANDLER_CLASS(RTCSyncRtcHandler);
	DEFINE_HANDLER_CLASS(RTCSyncHostHandler);
#endif 

#ifdef GPIODEV_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(GPIOHandler);
#endif


#ifdef SPWM_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(SPWMListHandler);
	DEFINE_HANDLER_CLASS(SPWMSetHandler);
	DEFINE_HANDLER_CLASS(SPWMSetFreqHandler);
	DEFINE_HANDLER_CLASS(SPWMResetHandler);
#endif


#ifdef ADCDEV_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(ADCDevStartHandler);
	DEFINE_HANDLER_CLASS(ADCDevStopHandler);
	DEFINE_HANDLER_CLASS(ADCDevReadHandler);
	DEFINE_HANDLER_CLASS(ADCDevReadMeanHandler);
#endif


#ifdef STEP_MOTOR_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(StepMotorInfoHandler);
	DEFINE_HANDLER_CLASS(StepMotorEnableHandler);
	DEFINE_HANDLER_CLASS(StepMotorSleepHandler);
	DEFINE_HANDLER_CLASS(StepMotorWaitHandler);
	DEFINE_HANDLER_CLASS(StepMotorDirHandler);
	DEFINE_HANDLER_CLASS(StepMotorSpeedHandler);
	DEFINE_HANDLER_CLASS(StepMotorMicroStepHandler);
	DEFINE_HANDLER_CLASS(StepMotorConfigHandler);
	DEFINE_HANDLER_CLASS(StepMotorStatusHandler);
	DEFINE_HANDLER_CLASS(StepMotorStartHandler);
	DEFINE_HANDLER_CLASS(StepMotorStopHandler);
	DEFINE_HANDLER_CLASS(StepMotorResetHandler);
	DEFINE_HANDLER_CLASS(StepMotorMoveHandler);
	DEFINE_HANDLER_CLASS(StepMotorMoveNonstopHandler);
    DEFINE_HANDLER_CLASS(StepMotorFeedHandler);
    DEFINE_HANDLER_CLASS(StepMotorSoftwareEndstopHandler);
#endif

#ifdef CAN_DEVICE_ENABLED
	DEFINE_HANDLER_CLASS(CanInfoHandler);
	DEFINE_HANDLER_CLASS(CanStartHandler);
	DEFINE_HANDLER_CLASS(CanStopHandler);
	DEFINE_HANDLER_CLASS(CanSendHandler);
	DEFINE_HANDLER_CLASS(CanFilterHandler);
	DEFINE_HANDLER_CLASS(CanStatusHandler);
	DEFINE_HANDLER_CLASS(CanReadHandler);
#endif
// ADD_DEVICE

/// @}