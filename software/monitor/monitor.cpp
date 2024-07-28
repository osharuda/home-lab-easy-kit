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
#include <monitor.hpp>
#include <libhlek/ekit_i2c_bus.hpp>
#include <libhlek/ekit_firmware.hpp>
#include <libconfig.hpp>
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
#include <libhlek/i2c_proto.h>
#include "handlers.hpp"
#include <libhlek/texttools.hpp>
#include <libhlek/tools.hpp>
#include <libhlek/ekit_error.hpp>
#include <termui.hpp>
#define I2C_BUS_NAME "/dev/i2c-0"

using namespace LIBCONFIG_NAMESPACE;


/*
    +---------+------------------------------------------------+
    | 0,0     |                                                |
    |         |                        A                       |
    |         |                        | AUTO                  |
    |         |          LOG WINDOW    | SCROLL                |
    |         |                        |                       |
    |         |                        |                       |
    |         |                                LINES-?, COLS   |
    +         +------------------------------------------------+
    +         |                                                +
    +---------+------------------------------------------------+    
*/

#define  CMD_WINDOW_HEIGHT  5
#define  CMD_WINDOW_WIDTH  50  

#define WND_LOG 0
#define WND_CMD 1
#define WND_ARG 2

HLEKMON::HLEKMON() : super() {

}

void HLEKMON::init() {
    super::init();

    log_window = std::make_shared<TUTextWindow>([]()->int{return CMD_WINDOW_WIDTH;},
                                                []()->int{return 0;},
                                                []()->int{return COLS-CMD_WINDOW_WIDTH;},
                                                []()->int{return LINES - CMD_WINDOW_HEIGHT;});

    cmd_window = std::make_shared<TUListWindow<std::shared_ptr<CommandHandler>>>([]()->int{return 0;},
                                                                                 []()->int{return 0;},
                                                                                 []()->int{return CMD_WINDOW_WIDTH;},
                                                                                 []()->int{return LINES;});
    cmd_window->set_item_search(false);

    arg_window = std::make_shared<TUInputWindow>([]()->int{return CMD_WINDOW_WIDTH;},
                                                 []()->int{return LINES-CMD_WINDOW_HEIGHT;},
                                                 []()->int{return COLS-CMD_WINDOW_WIDTH;},
                                                 []()->int{return CMD_WINDOW_HEIGHT;});

    log_window->set_box(true, "[LOG]", TITLE_OFFSET_MIDDLE);
    cmd_window->set_box(true, "[COMMAND]", TITLE_OFFSET_MIDDLE);
    arg_window->set_box(true, "[ARGUMENTS]", TITLE_OFFSET_MIDDLE);

    add_window(WND_LOG, log_window);
    add_window(WND_ARG, arg_window);
    add_window(WND_CMD, cmd_window);

    log_window->set_scroll_bar(SCROLLBAR_AUTO,SCROLLBAR_AUTO);
    arg_window->set_scroll_bar(SCROLLBAR_AUTO,SCROLLBAR_AUTO);
    cmd_window->set_scroll_bar(SCROLLBAR_AUTO,SCROLLBAR_AUTO);
    welcome();
}

void HLEKMON::welcome() {
    TERMUI::TUTextLines welcome_text = {
            "---====   Welcome to Home Lab Easy Kit monitor utility    ====----",
            "",
            "This program will help you to test many things like conection between computer and MCU, devices being used, etc.",
            "It consist of three windows, you can switch between them by pressing TAB key.",
            "",
            "COMMAND is a window with list of commands available for your configuration.",
            "    Use up and down arrow keys, page up/down, home and end keys to select required command.",
            "    Press enter to execute command. Press F1 if you need help.",
            "",
            "ARGUMENTS is a window for command arguments.",
            "    Enter required argument(s) and press enter to execute command. Some commands don't require arguments. Press F1 if you need help.",
            "",
            "LOG is a window for output.",
            "    Use arrows, page up/down, home and end keys for scrolling. New messages are appended to the bottom.",
            "------------------------------------------------------------------",
            "",
            ""
    };

    log(welcome_text);
    redraw(true);
}

void HLEKMON::on_command() {
    if (cmd_window->empty()) {
        log("*** no commands available");
    } else {
        std::shared_ptr<CommandHandler> handler = cmd_window->sel_item();
        std::string argtext = arg_window->get_text();
        std::vector<std::string> s_args;
        int arg_c = tools::parse_args(argtext, s_args);

        log(handler->get_command_name()+"> "+argtext);

        try {
            if (arg_c<0) {
                throw CommandHandlerException("*** Invalid arguments");
            }

            handler->arg_reset();
            handler->handle(s_args);
            arg_window->set_text("");
            set_active_window(WND_CMD);
        } catch(CommandHandlerException& che) {
            log(tools::str_format("*** ERROR: %s", che.what()));
        }
        catch(EKitException& ee) {
            log(ee.what());
        }
        catch (std::length_error& le) {
            log(le.what());
        }
        catch(...) {
            exit(1);
        }
    }
    redraw(true);
}

void HLEKMON::on_help() {
    std::shared_ptr<CommandHandler> handler = cmd_window->sel_item();
    log(handler->help());
    redraw(true);
}

void HLEKMON::on_log_event(wint_t ch, int err) {
}

void HLEKMON::on_arg_event(wint_t ch, int err) {
    if (err==KEY_CODE_YES && ch==KEY_F(1)) {
        on_help();
    } else
    if (err==OK || ch==L'\n') {
        on_command();
    }
}

void HLEKMON::on_cmd_event(wint_t ch, int err) {
    if (err==KEY_CODE_YES && ch==KEY_F(1)) {
        on_help();
    } else
    if (err==OK && ch==L'\n') {
        on_command();
    } else if (err==OK) {
        set_active_window(WND_ARG);
        arg_window->handler(ch, err);
        redraw(true);
    }
}

void HLEKMON::on_event(wint_t ch, int err) {

}

void HLEKMON::log(const TERMUI::TUTextLines& t) {
    log_window->append(t);
}

void HLEKMON::log(const std::string& t) {
    log_window->append(t);
}

void HLEKMON::add_command(int index, std::shared_ptr<CommandHandler>& handler) {
    cmd_window->push_back(TUListItem<std::shared_ptr<CommandHandler>>(index, handler->get_command_name(), handler));
}

void HLEKMON::message_handler(int index, wint_t ch, int err) {
    switch (index) {
        case WND_LOG:
            on_log_event(ch, err);
        break;
        case WND_CMD:
            on_cmd_event(ch, err);
        break;

        case WND_ARG:
            on_arg_event(ch, err);
        break;

        default:
            on_event(ch, err);
    }
}
std::vector<struct sigaction> new_sig_actions;
std::vector<struct sigaction> prev_sig_actions;

void test_signal(int signum) {
    std::cout << "Test signal" << std::endl;
}


int main(int argc, char* argv[])
{
    tools::install_signal_handler(SIGPWR, prev_sig_actions, new_sig_actions, test_signal);

    // Create a pid file
    bool singlton = tools::make_pid_file();
    if (!singlton) {
        std::cout << "This program is already running." << std::endl;
        return 1;
    }

    // Main TUI
    size_t cmd_index = 0;
    std::shared_ptr<HLEKMON> ui(new HLEKMON());
    ui->init();

    // Open I2C bus
    std::string bus_name = I2C_BUS_NAME;
    if (argc==2) {
        bus_name = argv[1];
    }
    std::shared_ptr<EKitBus> i2cbus(new EKitI2CBus(bus_name));
    EKitTimeout to(0);
    i2cbus->open(to);

    // Create firmware using a bus opened above
    std::shared_ptr<EKitBus> firmware (new EKitFirmware(i2cbus, LIBCONFIG_NAMESPACE::INFO_I2C_ADDRESS));

    std::shared_ptr<INFODev> info_dev(new INFODev(firmware, LIBCONFIG_NAMESPACE::info_config_ptr));
    std::shared_ptr<CommandHandler> info_dev_handler(dynamic_cast<CommandHandler*>(new InfoDevHandler(std::dynamic_pointer_cast<EKitDeviceBase>(info_dev), ui)));
    ui->add_command(cmd_index++, info_dev_handler);

#ifdef UART_PROXY_DEVICE_ENABLED
    for (size_t i = 0; i<uart_proxy_configs_number; i++) {
        const UARTProxyConfig* config = uart_proxy_configs + i;
        bool add_uart_dev = true;

        if (info_dev->config[config->dev_id].devices[i].hint == INFO_DEV_HINT_GSM_MODEM) {
            // attempt to add GSM modem
            try {
                std::shared_ptr<GSMModem> modem(new GSMModem(firmware, config));
                modem->set_timeout(30000);

                std::shared_ptr<CommandHandler> at_handler(dynamic_cast<CommandHandler*>(new ATHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> ussd_handler(dynamic_cast<CommandHandler*>(new USSDHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> sms_handler(dynamic_cast<CommandHandler*>(new SMSHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> read_sms_handler(dynamic_cast<CommandHandler*>(new ReadSMSHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> delete_sms_handler(dynamic_cast<CommandHandler*>(new DeleteSMSHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> dial_handler(dynamic_cast<CommandHandler*>(new DialHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> active_calls_handler(dynamic_cast<CommandHandler*>(new ActiveCallsHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));
                std::shared_ptr<CommandHandler> answer_call_handler(dynamic_cast<CommandHandler*>(new AnswerCallHandler(std::dynamic_pointer_cast<EKitDeviceBase>(modem), ui)));

                ui->add_command(cmd_index++, at_handler);
                ui->add_command(cmd_index++, ussd_handler);
                ui->add_command(cmd_index++, sms_handler);
                ui->add_command(cmd_index++, read_sms_handler);
                ui->add_command(cmd_index++, delete_sms_handler);
                ui->add_command(cmd_index++, dial_handler);
                ui->add_command(cmd_index++, active_calls_handler);
                ui->add_command(cmd_index++, answer_call_handler);
                add_uart_dev = false;
            } catch (EKitException& ee) {
                ui->log(ee.what());
            }
        }

        if (add_uart_dev) {
            std::shared_ptr<UARTProxyDev> uart_dev(new UARTProxyDev(firmware, config));
            std::shared_ptr<CommandHandler> uart_info_handler(dynamic_cast<CommandHandler*>(new UartDevInfo(std::dynamic_pointer_cast<EKitDeviceBase>(uart_dev), ui)));
            std::shared_ptr<CommandHandler> uart_read_handler(dynamic_cast<CommandHandler*>(new UartDevRead(std::dynamic_pointer_cast<EKitDeviceBase>(uart_dev), ui)));
            std::shared_ptr<CommandHandler> uart_write_handler(dynamic_cast<CommandHandler*>(new UartDevWrite(std::dynamic_pointer_cast<EKitDeviceBase>(uart_dev), ui)));

            ui->add_command(cmd_index++, uart_info_handler);
            ui->add_command(cmd_index++, uart_read_handler);
            ui->add_command(cmd_index++, uart_write_handler);
        }
    }
#endif

#ifdef LCD1602a_DEVICE_ENABLED
    std::shared_ptr<LCD1602ADev> lcd(new LCD1602ADev(firmware, lcd1602a_config_ptr));

    std::shared_ptr<CommandHandler> lcd_print_handler(dynamic_cast<CommandHandler*>(new LCDPrintHandler(std::dynamic_pointer_cast<EKitDeviceBase>(lcd), ui)));
    std::shared_ptr<CommandHandler> lcd_light_handler(dynamic_cast<CommandHandler*>(new LCDLightHandler(std::dynamic_pointer_cast<EKitDeviceBase>(lcd), ui)));

    ui->add_command(cmd_index++, lcd_light_handler);
    ui->add_command(cmd_index++, lcd_print_handler);    
#endif

#ifdef DESKDEV_DEVICE_ENABLED
    std::shared_ptr<DESKDev> deskdev(new DESKDev(firmware, desk_config_ptr));

    std::shared_ptr<CommandHandler> deskdev_status_handler(dynamic_cast<CommandHandler*>(new DeskDevStatusHandler(std::dynamic_pointer_cast<EKitDeviceBase>(deskdev), ui)));

    ui->add_command(cmd_index++, deskdev_status_handler);
#endif

#ifdef IRRC_DEVICE_ENABLED    
    std::shared_ptr<IRRCDev> irrc(new IRRCDev(firmware, irrc_config_ptr));

    std::shared_ptr<CommandHandler> irrc_handler(dynamic_cast<CommandHandler*>(new IRRCHandler(std::dynamic_pointer_cast<EKitDeviceBase>(irrc), ui)));

    ui->add_command(cmd_index++, irrc_handler);    
#endif

#ifdef RTC_DEVICE_ENABLED    
    std::shared_ptr<RTCDev> rtc(new RTCDev(firmware, rtc_config_ptr));

    std::shared_ptr<CommandHandler> rtc_get_handler(dynamic_cast<CommandHandler*>(new RTCGetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(rtc), ui)));
    std::shared_ptr<CommandHandler> rtc_sync_rtc_handler(dynamic_cast<CommandHandler*>(new RTCSyncRtcHandler(std::dynamic_pointer_cast<EKitDeviceBase>(rtc), ui)));
    std::shared_ptr<CommandHandler> rtc_sync_host_handler(dynamic_cast<CommandHandler*>(new RTCSyncHostHandler(std::dynamic_pointer_cast<EKitDeviceBase>(rtc), ui)));

    ui->add_command(cmd_index++, rtc_get_handler);
    ui->add_command(cmd_index++, rtc_sync_rtc_handler);
    ui->add_command(cmd_index++, rtc_sync_host_handler);    
#endif

#ifdef GPIODEV_DEVICE_ENABLED
    std::shared_ptr<GPIODev> gpio(new GPIODev(firmware, gpio_config_ptr));

    std::shared_ptr<CommandHandler> gpio_handler(dynamic_cast<CommandHandler*>(new GPIOHandler(std::dynamic_pointer_cast<EKitDeviceBase>(gpio), ui)));

    ui->add_command(cmd_index++, gpio_handler);    
#endif   


#ifdef SPWM_DEVICE_ENABLED
    std::shared_ptr<SPWMDev> spwm(new SPWMDev(firmware, spwm_config_ptr));

    std::shared_ptr<CommandHandler> spwm_list_handler(dynamic_cast<CommandHandler*>(new SPWMListHandler(std::dynamic_pointer_cast<EKitDeviceBase>(spwm), ui)));
    std::shared_ptr<CommandHandler> spwm_set_handler(dynamic_cast<CommandHandler*>(new SPWMSetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(spwm), ui)));
    std::shared_ptr<CommandHandler> spwm_freq_handler(dynamic_cast<CommandHandler*>(new SPWMSetFreqHandler(std::dynamic_pointer_cast<EKitDeviceBase>(spwm), ui)));
    std::shared_ptr<CommandHandler> spwm_reset_handler(dynamic_cast<CommandHandler*>(new SPWMResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(spwm), ui)));

    ui->add_command(cmd_index++, spwm_list_handler);    
    ui->add_command(cmd_index++, spwm_set_handler);
    ui->add_command(cmd_index++, spwm_freq_handler);
    ui->add_command(cmd_index++, spwm_reset_handler);
#endif

#ifdef ADCDEV_DEVICE_ENABLED
    

    struct ADCCommandHandlers {
        std::shared_ptr<ADCDev> dev;
        std::shared_ptr<CommandHandler> adc_start_handler;
        std::shared_ptr<CommandHandler> adc_status_handler;
        std::shared_ptr<CommandHandler> adc_config_handler;
        std::shared_ptr<CommandHandler> adc_stop_handler;
        std::shared_ptr<CommandHandler> adc_reset_handler;
        std::shared_ptr<CommandHandler> adc_read_handler;
    };

    std::vector<ADCCommandHandlers> adc_handlers(adc_configs_number);

    for (size_t index=0; index<adc_configs_number; index++) {
        const ADCConfig* config = adc_configs + index;
        ADCCommandHandlers& h = adc_handlers.at(index);

        h.dev.reset(new ADCDev(firmware, config));

        h.adc_start_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevStartHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.adc_status_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevStatusHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.adc_config_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevConfigHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.adc_stop_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.adc_reset_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.adc_read_handler.reset(dynamic_cast<CommandHandler*>(new ADCDevReadHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));


        ui->add_command(cmd_index++, h.adc_status_handler);
        ui->add_command(cmd_index++, h.adc_start_handler);
        ui->add_command(cmd_index++, h.adc_config_handler);
        ui->add_command(cmd_index++, h.adc_stop_handler);
        ui->add_command(cmd_index++, h.adc_reset_handler);
        ui->add_command(cmd_index++, h.adc_read_handler);  
    }
#endif    

#ifdef STEP_MOTOR_DEVICE_ENABLED
    struct StepMotorCommandHandlers {
        std::shared_ptr<StepMotorDev> dev;
        std::shared_ptr<CommandHandler> step_motor_info_handler;
        std::shared_ptr<CommandHandler> step_motor_status_handler;
        std::shared_ptr<CommandHandler> step_motor_config_handler;
        std::shared_ptr<CommandHandler> step_motor_start_handler;
        std::shared_ptr<CommandHandler> step_motor_feed_handler;
        std::shared_ptr<CommandHandler> step_motor_stop_handler;
        std::shared_ptr<CommandHandler> step_motor_reset_handler;

        // Commands
        std::shared_ptr<CommandHandler> step_motor_enable_handler;        
        std::shared_ptr<CommandHandler> step_motor_sleep_handler;
        std::shared_ptr<CommandHandler> step_motor_wait_handler;
        std::shared_ptr<CommandHandler> step_motor_dir_handler;
        std::shared_ptr<CommandHandler> step_motor_speed_handler;
        std::shared_ptr<CommandHandler> step_motor_microstep_handler;
        std::shared_ptr<CommandHandler> step_motor_move_handler;
        std::shared_ptr<CommandHandler> step_motor_move_ind_handler;
        std::shared_ptr<CommandHandler> step_motor_sft_endstop_handler;
    };

    std::vector<StepMotorCommandHandlers> step_motor_handlers(step_motor_configs_number);

    for (size_t i=0; i<step_motor_configs_number; i++) {
        const StepMotorConfig* config = step_motor_configs + i;
        uint8_t dev_id = config->dev_id;
        StepMotorCommandHandlers& h = step_motor_handlers.at(i);

        h.dev.reset(new StepMotorDev(firmware, config));

        h.step_motor_info_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorInfoHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_status_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorStatusHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_config_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorConfigHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_start_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorStartHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_feed_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorFeedHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_stop_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_reset_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_enable_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorEnableHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_sleep_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorSleepHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_wait_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorWaitHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_dir_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorDirHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_speed_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorSpeedHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_microstep_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorMicroStepHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_move_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorMoveHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_move_ind_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorMoveNonstopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.step_motor_sft_endstop_handler.reset(dynamic_cast<CommandHandler*>(new StepMotorSoftwareEndstopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));

        ui->add_command(cmd_index++, h.step_motor_info_handler);
        ui->add_command(cmd_index++, h.step_motor_status_handler);
        ui->add_command(cmd_index++, h.step_motor_start_handler);
        ui->add_command(cmd_index++, h.step_motor_feed_handler);
        ui->add_command(cmd_index++, h.step_motor_stop_handler);

        ui->add_command(cmd_index++, h.step_motor_enable_handler);
        ui->add_command(cmd_index++, h.step_motor_dir_handler);
        ui->add_command(cmd_index++, h.step_motor_speed_handler);
        ui->add_command(cmd_index++, h.step_motor_microstep_handler);
        ui->add_command(cmd_index++, h.step_motor_config_handler);
        ui->add_command(cmd_index++, h.step_motor_sft_endstop_handler);
        ui->add_command(cmd_index++, h.step_motor_sleep_handler);
        ui->add_command(cmd_index++, h.step_motor_reset_handler);
        ui->add_command(cmd_index++, h.step_motor_wait_handler);
        ui->add_command(cmd_index++, h.step_motor_move_handler);
        ui->add_command(cmd_index++, h.step_motor_move_ind_handler);

    }
#endif  

#ifdef CAN_DEVICE_ENABLED
    struct CanCommandHandlers {
        std::shared_ptr<CanDev> dev;
        std::shared_ptr<CommandHandler> can_info_handler;
        std::shared_ptr<CommandHandler> can_start_handler;
        std::shared_ptr<CommandHandler> can_stop_handler;
        std::shared_ptr<CommandHandler> can_send_handler;
        std::shared_ptr<CommandHandler> can_filter_handler;
        std::shared_ptr<CommandHandler> can_status_handler;
        std::shared_ptr<CommandHandler> can_read_handler;
    };

    std::vector<CanCommandHandlers> can_handlers(can_configs_number);

    for (size_t index=0; index<can_configs_number; index++) {
        const CANConfig* config = can_configs + index;
        CanCommandHandlers& h = can_handlers.at(index);


        h.dev.reset(new CanDev(firmware, config));

        h.can_info_handler.reset(dynamic_cast<CommandHandler*>(new CanInfoHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_info_handler);

        h.can_status_handler.reset(dynamic_cast<CommandHandler*>(new CanStatusHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_status_handler);

        h.can_read_handler.reset(dynamic_cast<CommandHandler*>(new CanReadHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_read_handler);

        h.can_start_handler.reset(dynamic_cast<CommandHandler*>(new CanStartHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_start_handler);

        h.can_stop_handler.reset(dynamic_cast<CommandHandler*>(new CanStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_stop_handler);

        h.can_send_handler.reset(dynamic_cast<CommandHandler*>(new CanSendHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_send_handler);

        h.can_filter_handler.reset(dynamic_cast<CommandHandler*>(new CanFilterHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.can_filter_handler);
    }
#endif  
#ifdef SPIPROXY_DEVICE_ENABLED
    struct SPIProxyCommandHandlers {
        std::shared_ptr<SPIProxyDev> dev;
        std::shared_ptr<CommandHandler> spiproxy_info_handler;
        std::shared_ptr<CommandHandler> spiproxy_read_handler;
        std::shared_ptr<CommandHandler> spiproxy_write_handler;
        std::shared_ptr<CommandHandler> spiproxy_reset_handler;
    };

    std::vector<SPIProxyCommandHandlers> spiproxy_handlers(spiproxy_configs_number);

    for (size_t index=0; index<spiproxy_configs_number; index++) {
        const SPIProxyConfig* config = spiproxy_configs + index;
        const InfoDeviceDescriptor* info_descr = info_dev->get_device_info(config->dev_id);
        SPIProxyCommandHandlers& h = spiproxy_handlers.at(index);
        h.dev.reset(new SPIProxyDev(firmware, config));

        // Initialize SPIProxy
        h.spiproxy_info_handler.reset(dynamic_cast<CommandHandler*>(new SPIProxyInfoHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spiproxy_read_handler.reset(dynamic_cast<CommandHandler*>(new SPIProxyReadHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spiproxy_write_handler.reset(dynamic_cast<CommandHandler*>(new SPIProxyWriteHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));

        ui->add_command(cmd_index++, h.spiproxy_info_handler);
        ui->add_command(cmd_index++, h.spiproxy_read_handler);
        ui->add_command(cmd_index++, h.spiproxy_write_handler);
    }
#endif  

#ifdef AD9850DEV_DEVICE_ENABLED
    struct AD9850DevCommandHandlers {
        std::shared_ptr<AD9850Dev> dev;
        std::shared_ptr<CommandHandler> ad9850dev_reset_handler;
        std::shared_ptr<CommandHandler> ad9850dev_update_handler;
    };

    std::vector<AD9850DevCommandHandlers> ad9850dev_handlers(ad9850_configs_number);

    for (size_t index=0; index<ad9850_configs_number; index++) {
        const AD9850Config* config = ad9850_configs + index;
        AD9850DevCommandHandlers& h = ad9850dev_handlers.at(index);


        h.dev.reset(new AD9850Dev(firmware, config));
        h.ad9850dev_reset_handler.reset(dynamic_cast<CommandHandler*>(new AD9850DevResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.ad9850dev_update_handler.reset(dynamic_cast<CommandHandler*>(new AD9850DevUpdateHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.ad9850dev_reset_handler);
        ui->add_command(cmd_index++, h.ad9850dev_update_handler);
    }
#endif  

#ifdef SPIDAC_DEVICE_ENABLED
    struct SPIDACCommandHandlers {
        std::shared_ptr<SPIDACDev> dev;
        std::shared_ptr<CommandHandler> spidac_start_cont_handler;
        std::shared_ptr<CommandHandler> spidac_start_period_handler;
        std::shared_ptr<CommandHandler> spidac_set_default_handler;
        std::shared_ptr<CommandHandler> spidac_stop_handler;
        std::shared_ptr<CommandHandler> spidac_is_running_handler;
        std::shared_ptr<CommandHandler> spidac_sin_handler;
        std::shared_ptr<CommandHandler> spidac_saw_handler;
        std::shared_ptr<CommandHandler> spidac_triangle_handler;
    };

    std::vector<SPIDACCommandHandlers> spidac_handlers(spidac_configs_number);

    for (size_t index=0; index<spidac_configs_number; index++) {
        const SPIDACConfig* config = spidac_configs + index;
        SPIDACCommandHandlers& h = spidac_handlers.at(index);

        h.dev.reset(new SPIDACDev(firmware, config));
        h.spidac_start_cont_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACStartContinuousHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_start_period_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACStartPeriodHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_set_default_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACSetDefaultHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_stop_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_is_running_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACIsRunningHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_sin_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACUploadSinWaveform(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_saw_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACUploadSawWaveform(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.spidac_triangle_handler.reset(dynamic_cast<CommandHandler*>(new SPIDACUploadTriangleWaveform(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));

        ui->add_command(cmd_index++, h.spidac_start_cont_handler);
        ui->add_command(cmd_index++, h.spidac_start_period_handler);
        ui->add_command(cmd_index++, h.spidac_set_default_handler);
        ui->add_command(cmd_index++, h.spidac_stop_handler);
        ui->add_command(cmd_index++, h.spidac_is_running_handler);
        ui->add_command(cmd_index++, h.spidac_sin_handler);
        ui->add_command(cmd_index++, h.spidac_saw_handler);
        ui->add_command(cmd_index++, h.spidac_triangle_handler);
    }
#endif  
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
#ifdef PACEMAKERDEV_DEVICE_ENABLED
    struct PaceMakerDevCommandHandlers {
        std::shared_ptr<PaceMakerDev> dev;
        std::shared_ptr<CommandHandler> pacemakerdev_info_handler;
        std::shared_ptr<CommandHandler> pacemakerdev_start_handler;
        std::shared_ptr<CommandHandler> pacemakerdev_stop_handler;
        std::shared_ptr<CommandHandler> pacemakerdev_reset_handler;
        std::shared_ptr<CommandHandler> pacemakerdev_add_transition_handler;
    };

    std::vector<PaceMakerDevCommandHandlers> pacemakerdev_handlers(LIBCONFIG_NAMESPACE::pacemakerdev_configs_number);

    for (size_t index=0; index<LIBCONFIG_NAMESPACE::pacemakerdev_configs_number; index++) {
        const PaceMakerDevConfig* descr = LIBCONFIG_NAMESPACE::pacemakerdev_configs+index;
        uint8_t dev_id = descr->dev_id;
        PaceMakerDevCommandHandlers& h = pacemakerdev_handlers.at(index);


        h.dev.reset(new PaceMakerDev(firmware, descr));
        h.pacemakerdev_info_handler.reset(dynamic_cast<CommandHandler*>(new PaceMakerDevInfoHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.pacemakerdev_start_handler.reset(dynamic_cast<CommandHandler*>(new PaceMakerDevStartHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.pacemakerdev_stop_handler.reset(dynamic_cast<CommandHandler*>(new PaceMakerDevStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.pacemakerdev_reset_handler.reset(dynamic_cast<CommandHandler*>(new PaceMakerDevResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.pacemakerdev_add_transition_handler.reset(dynamic_cast<CommandHandler*>(new PaceMakerDevSetDataHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));

        ui->add_command(cmd_index++, h.pacemakerdev_info_handler);
        ui->add_command(cmd_index++, h.pacemakerdev_start_handler);
        ui->add_command(cmd_index++, h.pacemakerdev_stop_handler);
        ui->add_command(cmd_index++, h.pacemakerdev_reset_handler);
        ui->add_command(cmd_index++, h.pacemakerdev_add_transition_handler);
    }
#endif  
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
#ifdef TIMETRACKERDEV_DEVICE_ENABLED
    struct TimeTrackerDevCommandHandlers {
        std::shared_ptr<TimeTrackerDev> dev;
        std::shared_ptr<CommandHandler> timetrackerdev_status_handler;
        std::shared_ptr<CommandHandler> timetrackerdev_start_handler;
        std::shared_ptr<CommandHandler> timetrackerdev_stop_handler;
        std::shared_ptr<CommandHandler> timetrackerdev_reset_handler;
        std::shared_ptr<CommandHandler> timetrackerdev_read_handler;
    };

    std::vector<TimeTrackerDevCommandHandlers> timetrackerdev_handlers(LIBCONFIG_NAMESPACE::timetrackerdev_configs_number);

    for (size_t index=0; index<LIBCONFIG_NAMESPACE::timetrackerdev_configs_number; index++) {
        const TimeTrackerDevConfig* descr = LIBCONFIG_NAMESPACE::timetrackerdev_configs+index;
        uint8_t dev_id = descr->dev_id;
        TimeTrackerDevCommandHandlers& h = timetrackerdev_handlers.at(index);

        h.dev.reset(new TimeTrackerDev(firmware, descr));

        h.timetrackerdev_status_handler.reset(dynamic_cast<CommandHandler*>(new TimeTrackerDevStatusHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.timetrackerdev_start_handler.reset(dynamic_cast<CommandHandler*>(new TimeTrackerDevStartHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.timetrackerdev_stop_handler.reset(dynamic_cast<CommandHandler*>(new TimeTrackerDevStopHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.timetrackerdev_reset_handler.reset(dynamic_cast<CommandHandler*>(new TimeTrackerDevResetHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        h.timetrackerdev_read_handler.reset(dynamic_cast<CommandHandler*>(new TimeTrackerDevReadHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));

        ui->add_command(cmd_index++, h.timetrackerdev_status_handler);
        ui->add_command(cmd_index++, h.timetrackerdev_start_handler);
        ui->add_command(cmd_index++, h.timetrackerdev_stop_handler);
        ui->add_command(cmd_index++, h.timetrackerdev_reset_handler);
        ui->add_command(cmd_index++, h.timetrackerdev_read_handler);
    }
#endif  
// -> ADD_DEVICE | HASH: 18812534EC04D74C570D3CB18C756C595E8A3613
// ADD_DEVICE

    ui->runloop();

    tools::delete_pid_file();
}
