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
 *   \brief Can device software implementation header
 *   \author Oleh Sharuda
 */

#pragma once

#include <map>
#include "ekit_device.hpp"
#include "can_common.hpp"

/// \defgroup group_can CanDev
/// \brief Can support
/// @{
/// \page page_can
/// \tableofcontents
///
/// \section sect_can_01 Work with CanDev
///
/// CanDev functionality provides the following features:
/// - features list ...
///
/// Basic logic of CanDev functionality work is shown on the following schema:
/// \image html CanDev_schema.png
/// \image latex CanDev_schema.eps
///
/// CanDev can be used as:
/// 1. Create CanDev object
/// 2. Call CanDev#do_something() method to do something.
///

/// \class CanDev
/// \brief CanDev implementation. Use this class in order to control CanDev virtual devices.
class CanDev final : public EKitVirtualDevice {

    /// \typedef super
    /// \brief Defines parent class
	typedef EKitVirtualDevice super;

	public:

    /// \brief Pointer to the #tag_CANConfig structure that describes CanDev virtual device represented by this class.
    const CANConfig* config;

    /// \brief No default constructor
    CanDev() = delete;

    /// \brief Copy construction is forbidden
    CanDev(const CanDev&) = delete;

    /// \brief Assignment is forbidden
    CanDev& operator=(const CanDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param config - actual configuration of the device taken from generated configuration library.
	CanDev(std::shared_ptr<EKitBus>& ebus, const CANConfig* config);

    /// \brief Destructor (virtual)
	~CanDev() override;

	/// \brief Starts can communication.
	void can_start();

	/// \brief Stops can communication.
	void can_stop();

	/// \brief Sends a message to CAN.
	/// \param id - message id.
	/// \param data - std::vector of up to 8 bytes.
	/// \param remote_frame - true to send remote frame, otherwise false.
	/// \param extended - true to send extended frame, otherwise (standard frame) false.
	void can_send(uint32_t id, std::vector<uint8_t>& data, bool remote_frame, bool extended);

    /// \brief Returns CAN device status
    /// \param status - device status represented by #CanStatus structure.
	void can_status(CanStatus& status);

	/// \brief Converts #CanStatus to std::string
	/// \param status - device status represented by #CanStatus
	/// \return std::string
	static std::string can_status_to_str(CanStatus& status);

	/// \brief Converts #CanRecvMessage to std::string
	/// \param msg - message represented by #CanRecvMessage
	/// \return std::string
	static std::string can_msg_to_str(CanRecvMessage& msg);

	/// \brief Converts last error code to std::string
	/// \param lec - last error code
	/// \return std::string
	static std::string can_last_err_to_str(uint8_t lec);

	/// \brief Reads messages and status from CAN
	/// \param status - status of devide represented by #CanStatus
	/// \return messages - reference to output std::vector of messages.
	void can_read(CanStatus& status, std::vector<CanRecvMessage>& messages);

	/// \brief Put standard frame (4 half word ids) filter for CAN receiver.
	/// \param enabled - true if filter should be enabled, otherwise false.
	/// \param index - filter index [0 .. CAN_MAX_FILTER_COUNT-1 ].
	/// \param msb_id - id 0.
	/// \param lsb_id - id 1.
	/// \param msb_id_mask - id 2 or mask 0 (for mask mode),
	/// \param lsb_id_mask - id 3 or mask 1  (for mask mode),
	/// \param fifo1 - true to use FIFO 1, otherwise FIFO 0 is used.
	/// \param mask_mode - true to set mask mode, otherwise id list mode is used.
	void can_filter_std(    bool enabled,
                            uint8_t index,
                            uint16_t msb_id,
                            uint16_t lsb_id,
                            uint16_t msb_id_mask,
                            uint16_t lsb_id_mask,
                            bool fifo1 = false,
                            bool mask_mode = false);

	/// \brief Put standard frame (2 word ids) filter for CAN receiver.
	/// \param enabled - true if filter should be enabled, otherwise false.
	/// \param index - filter index [0 .. CAN_MAX_FILTER_COUNT-1 ].
	/// \param id - id 0.
	/// \param id_mask - id 1 or mask 0 (for mask mode),
	/// \param fifo1 - true to use FIFO 1, otherwise FIFO 0 is used.
	/// \param mask_mode - true to set mask mode, otherwise id list mode is used.
	void can_filter_std_32(    bool enabled,
                               uint8_t index,
                               uint32_t id,
                               uint32_t id_mask,
                               bool fifo1 = false,
                               bool mask_mode = false);

	/// \brief Put extended frame filter for CAN receiver.
	/// \param enabled - true if filter should be enabled, otherwise false.
	/// \param index - filter index [0 .. CAN_MAX_FILTER_COUNT-1 ].
	/// \param id - id 0.
	/// \param id_mask - id 1 or mask 0 (for mask mode),
	/// \param fifo1 - true to use FIFO 1, otherwise FIFO 0 is used.
	/// \param mask_mode - true to set mask mode, otherwise id list mode is used.
	void can_filter_ext(    bool enabled,
                            uint8_t index,
                            uint32_t id,
                            uint32_t id_mask,
                            bool fifo1 = false,
                            bool mask_mode = false);


private:
    static std::map<uint16_t, std::pair<std::string, std::string>> state_flag_map;

    /// \brief Sends filter structure to CAN device
    /// \param filter - filter represented by #CanFilterCommand structure
    /// \note Throws an exception if device is started.
    void can_filter_priv(CanFilterCommand filter);

    /// \brief Returns CAN device status
    /// \param status - device status represented by #CanStatus structure.
    /// \note Doesn't lock a bus; It is callers responsibility to lock a bus.
    void can_status_priv(CanStatus& status, EKitTimeout& to);
};

/// @}
