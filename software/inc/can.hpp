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
#include "sw.h"

#ifdef CAN_DEVICE_ENABLED

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

	/// \brief Pointer to the #tag_CanInstance structure that describes CanDev virtual device represented by this class.
	const CanInstance* descr = nullptr;

	public:

    /// \brief No default constructor
    CanDev() = delete;

    /// \brief Copy construction is forbidden
    CanDev(const CanDev&) = delete;

    /// \brief Assignment is forbidden
    CanDev& operator=(const CanDev&) = delete;

    /// \brief Constructor to be used
    /// \param ebus - reference to shared pointer with EKitBus. This bus must support FIRMWARE_OPT_FLAGS
    /// \param addr - device id of the virtual device to be attached to the instance of this class. Device id must belong
    ///        to the CanDev device.
	CanDev(std::shared_ptr<EKitBus>& ebus, int addr);

    /// \brief Destructor (virtual)
	~CanDev() override;

	static const CanInstance* get_descriptor(size_t index);
	std::string get_dev_name() const;

	/// \brief Starts can communication.
	void can_start();

	/// \brief Stops can communication.
	void can_stop();

	/// \brief Sends a message to CAN.
	void can_send(uint32_t id, std::vector<uint8_t>& data, bool remote_frame, bool extended);

    /// \brief Returns CAN device status
	void can_status(CanStatus& status);

	static std::string can_status_to_str(CanStatus& status);
	static std::string can_msg_to_str(CanRecvMessage& msg);
	static std::string can_last_err_to_str(uint8_t lec);

	void can_read(CanStatus& status, std::vector<CanRecvMessage>& messages);

	/// \brief Put standard frame (4 half word ids) filter for CAN receiver.
	void can_filter_std(    bool enabled,
                            uint8_t index,
                            uint16_t msb_id,
                            uint16_t lsb_id,
                            uint16_t msb_id_mask,
                            uint16_t lsb_id_mask,
                            bool fifo1 = false,
                            bool mask_mode = false);

	/// \brief Put standard frame (2 word ids) filter for CAN receiver.
	void can_filter_std_32(    bool enabled,
                               uint8_t index,
                               uint32_t id,
                               uint32_t id_mask,
                               bool fifo1 = false,
                               bool mask_mode = false);

	/// \brief Put extended frame filter for CAN receiver.
	void can_filter_ext(    bool enabled,
                            uint8_t index,
                            uint32_t id,
                            uint32_t id_mask,
                            bool fifo1 = false,
                            bool mask_mode = false);


private:
    static std::map<uint16_t, std::pair<std::string, std::string>> state_flag_map;
    void can_filter_priv(CanFilterCommand filter);

    void can_status_priv(CanStatus& status);

};

/// @}

#endif
