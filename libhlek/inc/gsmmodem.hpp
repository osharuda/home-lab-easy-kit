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
 *   \brief GSMModem software implementation header
 *   \author Oleh Sharuda
 */

#include "ekit_bus.hpp"
#include "ekit_device.hpp"
#include <memory>
#include <unicode/regex.h>
#include "tools.hpp"

/// \defgroup group_gsm_modem GSMModem
/// \brief GSM modem support
/// @{
/// \page page_gsm_modem
/// \tableofcontents
///
/// \section sect_gsm_modem_01 Work with GSMModem
///
/// GSMModem is not a typical virtual device for Home Lab Easy Kit project. The big difference between GSMModem and other
/// devices is that it is not utilize any of STM32F103x peripherals directly. GSMModem is a separate class that allows
/// to use GSM modem through the #UARTDev virtual device configured with #INFO_DEV_HINT_GSM_MODEM specified in JSON
/// configuration file. There are a lot of GSM modems on the market. This implementation was designed and somewhat tested
/// with SIM800L module. Possibly it will work with other modems.
///
/// GSMModem features several features:
/// - Execute an AT command.
/// - Execute USSD request.
/// - Send sms (UCS2 is supported).
/// - Read incoming sms.
/// - Delete an sms.
/// - Issue a call
/// - List active calls,
/// - Answer/Hold an incoming call.
///
/// The following information will help to use #GSMModem:
/// 0. #GSMModem is not connected directly to firmware. Instead, it is designed to be used without firmware and STM32F103x
///    at all. GSMModem is derived from #EKitDeviceBase which is base of the #EKitVirtualDevice class (used as parent by
///    other virtual devices). #EKitDeviceBase doesn't require derived class to use communication protocol or anything
///    special that makes difference with any other serial port. Theoretically any class derived from #EKitBus may be used.
///    You just need to pass reference to std::shared_ptr<EKitBus> and enjoy. On practice it will work with two implementations
///    of EKitBus: EKitFirmware and hypothetical implementation of serial port, which currently is not available, but easily
///    may be implemented. The second case may work without firmware at all.
/// 1. Create GSMModem object and specify device id of the corresponding #UARTDev virtual device which is connected to GSM
///    modem.
/// 2. Constructor configures GSM modem, note, some time may be required for modem to initialize.
/// 3. GSMModem#at() executes some AT command.
/// 4. GSMModem#ussd() executes some ussd request.
/// 5. In order to send, read and delete sms use GSMModem#sms(), GSMModem#read_sms(), GSMModem#delete_sms() respectively.
/// 6. To control voice calls use: GSMModem#active_calls(), GSMModem#dial(), GSMModem#answer() methods.
/// 7. Error reporting may be changed with GSMModem#set_error_mode().
///
/// AT commands return statuses like "OK", "ERROR", "RING", etc. Some of these statuses are
/// errors, some informational, some may be treated as warnings. This makes impact on the model how this class treats errors:
/// - All errors related to GSM modem communication issues are reported as exceptions (#EKitException).
/// - "ERROR" status causes #EKitException to be thrown,
/// - Other statuses are returned with status_mask output parameter available in corresponding public methods.
/// This error handling approach gives opportunity to handle modem statuses that appears unexpectedly, like "RING".
///
/// A few words regarding STM32F103x to GSM modem connection. User should read documentation on GSM modem carefully. SIM800L
/// GSM modems may not simply be connected to the STM32F103 for multiple reasons:
/// - SIM800 GSM modems require non-standard \f$V_{cc}\f$ voltage. It means DC-DC conversion is required. Beware, GSM modems
///   may require significant currents (more than 2A) for some periods of time. Insufficient output current from DC-DC
///   convertor may cause GSM modem to work unstable. During the testing two serially connected shotky diodes were used
///   to decrease voltage from 5V to ~4.2V. These diodes must be capable to handle direct current consumed by GSM modem.
///   Treat this method as dirty hack, which is not recommended for many reasons. Use reliable DC-DC convertor.
/// - Non standard \f$V_{cc}\f$ also causes need for logical level shifter. Ignoring this, may lead to broken hardware.
/// - Noises causes unpleasant effects and GSM modem may become unresponsive (#EKitException with EKIT_CRC_ERROR may
///   be thrown). Tested GSM modem was connected with shielded wires during testing to suppress these effects (RX and TX
///   lines).
///
/// This functionality was not, and can not be tested on all GSM modems possible. Moreover, GSM modems are very complex
/// devices, so it is impossible to test all the features and situations on single modem. SIM800L is the only GSM modem
/// that was used to test this functionality. Therefore it is wise to double test required features comprehensively,
/// especially if other type of the GSM modem is used. Treat this functionality as very raw, and do not use it for
/// applications that require reliability.
///

using namespace icu;
#include "uart_proxy_common.hpp"

/// \struct GSMSmsData
/// \brief This structure describes SMS with GSMModem#read_sms() call.
struct GSMSmsData {
    size_t id;                  ///< Id of the message.
    std::string message;        ///< Message text.
    std::string phone_number;   ///< Sender phone number.
    std::string status;         ///< Status of the message.
    std::string timestamp;      ///< Timestamp of the message.
};

/// \enum GSM_CALL_DIRECTION
/// \brief Describes call direction
/// \note Read modem documentation for more details
enum GSM_CALL_DIRECTION {
    GSM_CALL_DIRECTION_OUTGOING = 0,    ///< You are calling to someone.
    GSM_CALL_DIRECTION_INCOMING = 1     ///< Someone is calling you.
};

/// \brief Converts #GSM_CALL_DIRECTION into constant string
/// \param v - call direction value
/// \return Constant null terminated string with direction.
const char* gsm_call_direction_name(GSM_CALL_DIRECTION v);

/// \enum GSM_CALL_STATE
/// \brief Describes call state
/// \note Read modem documentation for more details
enum GSM_CALL_STATE {
    GSM_CALL_STATE_ACTIVE     = 0,  ///< Active
    GSM_CALL_STATE_HELD       = 1,  ///< Held
    GSM_CALL_STATE_DIALING    = 2,  ///< Dialing (MO call)
    GSM_CALL_STATE_ALERTING   = 3,  ///< Alerting (MO call)
    GSM_CALL_STATE_INCOMING   = 4,  ///< Incoming (MT call)
    GSM_CALL_STATE_WAITING    = 5,  ///< Waiting (MT call)
    GSM_CALL_STATE_DISCONNECT = 6   ///< Disconnect
};
/// \brief Converts #GSM_CALL_STATE into constant string
/// \param v - call state value
/// \return Constant null terminated string with call state
const char* gsm_call_state_name(GSM_CALL_STATE v);

/// \enum GSM_CALL_MODE
/// \brief Describes call mode.
/// \note Read modem documentation for more details.
enum GSM_CALL_MODE {
    GSM_CALL_MODE_VOICE = 0,    ///< Voice
    GSM_CALL_MODE_DATA  = 1,    ///< Data
    GSM_CALL_MODE_FAX   = 2     ///< Fax
};

/// \brief Converts #GSM_CALL_MODE into constant string
/// \param v - call mode value
/// \return Constant null terminated string with call mode
const char* gsm_call_mode_name(size_t v);

/// \enum GSM_CALL_MPTY
/// \brief Describes call multiparty mode.
enum GSM_CALL_MPTY { // (muliparty)
    GSM_CALL_MPTY_SINGLE  = 0,  ///< Single.
    GSM_CALL_MPTY_MULTI   = 1   ///< Multiparty.
};

/// \brief Converts #GSM_CALL_MPTY into constant string
/// \param v - call multiparty mode value.
/// \return Constant null terminated string with call multiparty mode.
const char* gsm_call_mpty_name(size_t v);

/// \struct GSMCallData
/// \brief Describes a call with GSMModem#read_sms() call.
struct GSMCallData {
    size_t idx;                     ///< call index
    GSM_CALL_DIRECTION direction;   ///< call direction
    GSM_CALL_STATE state;           ///< call state
    GSM_CALL_MODE mode;             ///< call mode
    GSM_CALL_MPTY mpty;             ///< call muliparty mode
    std::string number;             ///< number

    /// \brief Checks if structure is correct.
    /// \return true if structure is correct, otherwise false.
    bool is_valid() const;

    /// \brief Convert structure to std::string.
    /// \return Formated std::string with call information.
    std::string to_string() const;
};

/// \enum GSM_CALL_ACTION
/// \brief Describes action for call.
/// \note Read modem documentation for more details.
enum GSM_CALL_ACTION {
    GSM_CALL_ACTION_ANSWER = 1, ///< Answer
    GSM_CALL_ACTION_HANG = 2,   ///< Hang
    GSM_CALL_ACTION_HOLD = 3,   ///< Hold
    GSM_CALL_ACTION_RELEASE = 4 ///< Release
};

/// \enum GSM_CMEE_MODE
/// \brief Describes CMEE error mode used.
enum GSM_CMEE_MODE {
    GSM_CMEE_DISABLE = 0,   ///< Disabled
    GSM_CMEE_NUMERIC = 1,   ///< Number CMEE mode.
    GSM_CMEE_TEXT = 2       ///< Text CMEE mode.
};

/// \class GSMModem
/// \brief GSMModem implementation. Use this class in order to control #UARTDev virtual devices which is connected to GSM
///        modem.
class GSMModem final : public EKitDeviceBase {

    /// \enum GSMStatusOffset
    /// \brief Bit offset for GSMModemStatus constants
    enum GSMStatusOffset {
        AT_OK           = 0, ///< Bit offset for AT_STATUS_OK.
        AT_CONNECT      = 1, ///< Bit offset for AT_STATUS_CONNECT.
        AT_RING         = 2, ///< Bit offset for AT_STATUS_RING.
        AT_NO_CARRIER   = 3, ///< Bit offset for AT_STATUS_NO_CARRIER.
        AT_ERROR        = 4, ///< Bit offset for AT_STATUS_ERROR.
        AT_NO_DIALTONE  = 5, ///< Bit offset for AT_STATUS_NO_DIALTONE.
        AT_BUSY         = 6, ///< Bit offset for AT_STATUS_BUSY.
        AT_NO_ANSWER    = 7, ///< Bit offset for AT_STATUS_NO_ANSWER.
        AT_PROMPT       = 8  ///< Bit offset for AT_STATUS_PROMPT.
    };

    /// \brief AT statuses
    static constexpr char* at_status_name[] = {(char*)"OK", (char*)"CONNECT", (char*)"RING", (char*)"NO CARRIER", (char*)"ERROR", (char*)"NO DIALTONE", (char*)"BUSY", (char*)"NO ANSWER", (char*)"> "};
    static constexpr char* cmee_error_header = (char*)"+CME ERROR: "; ///< CMEE error header

public:

    /// \enum GSMModemStatus
    /// \brief AT statuses.
    /// \note These AT statuses constants are represented as bitmask of flags. This is made because some operation may
    ///       return none, one or more status flags.
    enum GSMModemStatus {
        AT_STATUS_OK            = (1 << AT_OK),         ///< OK
        AT_STATUS_CONNECT       = (1 << AT_CONNECT),    ///< CONNECT
        AT_STATUS_RING          = (1 << AT_RING),       ///< RING
        AT_STATUS_NO_CARRIER    = (1 << AT_NO_CARRIER), ///< NO CARIER
        AT_STATUS_ERROR         = (1 << AT_ERROR),      ///< ERROR
        AT_STATUS_NO_DIALTONE   = (1 << AT_NO_DIALTONE),///< NO DIALTONE
        AT_STATUS_BUSY          = (1 << AT_BUSY),       ///< BUSY
        AT_STATUS_NO_ANSWER     = (1 << AT_NO_ANSWER),  ///< NO ANSWER
        AT_STATUS_PROMPT        = (1 << AT_PROMPT)      ///< PROMPT
    };

private:

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitDeviceBase super;

	std::unique_ptr<RegexPattern> re_ussd;      ///< Regular expression to parse USSD request
    std::unique_ptr<RegexPattern> re_read_sms;  ///< Regular expression to parse SMS listing
    std::unique_ptr<RegexPattern> re_list_call; ///< Regular expression to parse call listing

    bool sms_ascii_mode = false;                ///< true if sms in ASCII mode
    std::string modem_name;                     ///< UARTDev virtual device name associated with this GSMModem
    const std::string at_terminator = "\r\n";   ///< AT line terminator

    /// \brief Private delegating constructor
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///               to the UARTDev device (with #INFO_DEV_HINT_GSM_MODEM hint specified in JSON configuration file).
    /// \param name - name of the device as specified in JSON configuration file.
    GSMModem(std::shared_ptr<EKitBus>& ebus, int addr, const char* name);

    /// \brief Convert string returned by modem into modem status code
    /// \param line - Line returned by a modem
    /// \return Status code, one of #GSMModemStatus values. If 0, line doesn't contain any of AT statuses.
    unsigned int get_status(const std::string& line);

    /// \brief Throws an exception with respect to current CMEE mode.
    /// \param func_name - name of the function
    /// \param status - status code
    /// \param description - Text description
    void throw_at_error(const char* func_name, unsigned  int status, const std::string& description);

    /// \brief Check if character is a line terminator
    /// \param c - character to be checked
    /// \return true if terminating character, otherwise false.
    static bool is_terminator(char c) noexcept;

    /// \brief Converts hex encoded UCS2 string into string
    /// \param hex - Hex encoded UCS2 string into string
    /// \return Converted string
    std::string UCS2_to_string(const std::string& hex) const;

    /// \brief Converts string into hex encoded UCS2 string into string.
    /// \param s - String to be converted
    /// \return Hex encoded UCS2 string
    std::string string_to_UCS2(const std::string& s) const;

    /// \brief Internal implementation of at command
    /// \param cmd - std::string with an AT command
    /// \param response - vector of strings with response
    /// \param sw - stop watch timer
    /// \param completion_status_mask - One or several #GSMModemStatus status mask that indicates command is completed.
    /// \note  Call will not return until timeout is expired or one of completion_status_mask statuses are received from
    ///        device
    void at(const std::string& cmd, std::vector<std::string>& response, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int &completion_status_mask);

    /// \brief Sets CMEE error mode
    /// \param cmee - #GSM_CMEE_MODE error mode
    /// \param sw - stop watch timer
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void set_error_mode(GSM_CMEE_MODE cmee, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask);

    /// \brief Configures modem with default configuration
    /// \param timeout_ms - timeout in milliseconds.
    void configure(int timeout_ms);

    /// \brief Configures sms
    /// \param ascii - true if ascii mode is required.
    /// \param sw - stop watch timer
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void configure_sms(bool ascii, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask);

    /// \brief Reads some strings with line breaks (or lines) from modem
    /// \param result - vector of lines read during call
    /// \param sw - stop watch timer
    /// \return Corresponding #EKIT_ERROR
    EKIT_ERROR read_lines(std::vector<std::string>& result, tools::StopWatch<std::chrono::milliseconds>& sw);

    /// \brief Wait until completion AT status is returned
    /// \param result - vector of lines read during call
    /// \param sw - stop watch timer
    /// \param completion_status_mask - One or several #GSMModemStatus status mask that indicates command is completed.
    /// \return Corresponding #EKIT_ERROR
    /// \note  Call will not return until timeout is expired or one of completion_status_mask statuses are received from
    ///        device
    EKIT_ERROR wait_at_status(std::vector<std::string>& result, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& completion_status_mask);

    /// \brief Wait until command returned line with a prefix
    /// \param prefix - expected prefix
    /// \param result - vector of lines read during call
    /// \param sw - stop watch timer
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    /// \return Corresponding #EKIT_ERROR
    EKIT_ERROR wait_at_response(const std::string& prefix, std::vector<std::string>& result, tools::StopWatch<std::chrono::milliseconds>& sw, unsigned int& status_mask);

    GSM_CMEE_MODE cmee_mode = GSM_CMEE_MODE::GSM_CMEE_TEXT;
    std::string last_cmee_error;

public:

    /// \brief No default constructor
    GSMModem() = delete;

    /// \brief Copy construction is forbidden
    GSMModem(const GSMModem&) = delete;

    /// \brief Assignment is forbidden
    GSMModem& operator=(const GSMModem&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus.
    /// \param config - actual configuration of the device taken from generated configuration library.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    GSMModem(std::shared_ptr<EKitBus>& ebus,  const UARTProxyConfig* config, int timeout_ms);

    /// \brief Destructor (virtual)
	~GSMModem() override;

    /// \brief Execute an AT command
    /// \param cmd - command to be executed
    /// \param response - Response with text
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param completion_status_mask - One or several #GSMModemStatus status mask that indicates command is completed.
    /// \note  Call will not return until timeout is expired or one of completion_status_mask statuses are received from
    ///        device
    void at(const std::string& cmd, std::vector<std::string>& response, int timeout_ms, unsigned int &completion_status_mask);

    /// \brief Execute USSD request
    /// \param ussd - USSD request
    /// \param result - result
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void ussd(const std::string& ussd, std::string& result, int timeout_ms, unsigned int& status_mask);

    /// \brief Send an SMS
    /// \param number - telephone number
    /// \param text - text
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void sms(const std::string& number, const std::string& text, int timeout_ms, unsigned int& status_mask);

    /// \brief Read sms
    /// \param messages - vector of #GSMSmsData describing messages available
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void read_sms(std::vector<GSMSmsData>& messages, int timeout_ms, unsigned int& status_mask);

    /// \brief Delete sms
    /// \param id - id of the message to be deleted. Negative values will remove all messages.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout_ms.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void delete_sms(int id, int timeout_ms, unsigned int& status_mask);

    /// \brief Set CMEE error mode
    /// \param cmee - CMEE error mode
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void set_error_mode(GSM_CMEE_MODE cmee, int timeout_ms, unsigned int& status_mask);

    /// \brief Lists active calls
    /// \param active_calls - vector of #GSMCallData structures describing active calls.
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void active_calls(std::vector<GSMCallData>& active_calls, int timeout_ms, unsigned int& status_mask);

    /// \brief Dial a number
    /// \param number - telephone number
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void dial(const std::string& number, int timeout_ms, unsigned int& status_mask);

    /// \brief Answer a call
    /// \param action - action to be made
    /// \param timeout_ms - Timeout in milliseconds. Zero or negative means infinite timeout.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    void answer(GSM_CALL_ACTION action, int timeout_ms, unsigned int& status_mask);

    /// \brief Formats status_mask description from status_mask value returned by one of the calls above.
    /// \param status_mask - One or several #GSMModemStatus status mask that indicates status of operation.
    /// \return string that represents passed value.
    static std::string status_description(unsigned int status_mask);
};

/// @}