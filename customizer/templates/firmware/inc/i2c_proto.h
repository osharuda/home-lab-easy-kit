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
 *   \brief Header for communication protocol implementation over i2c bus (shared between software and firmware)
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>

/// \defgroup group_communication Communication
///  @{{

/// \defgroup group_communication_details Communication protocol
/// \brief Communication protocol between software and firmware
///  @{{
/// \page page_communication_details
/// \tableofcontents
///
/// \section sect_communication_details_01 Description
/// Software start communication by sending a command byte. Command byte consist of device ID, an identifier unique to each device represented by firmware.
/// It is possible to address up to the 16 such devices. Also command byte may keep up to the 4 additional flags which will be delivered to the device.
/// These flags are passed to the device and every device may react on these flags. It is possible to pass simple commands/states using these flags.
///
/// Communication with firmware is based on commands. Each device should define it’s own command(s) set, but there is general flow that every command should satisfy.
/// Simple devices may use device specific flags allocated in command byte, more complex devices may pass information using command data.
///
/// Software sends data and selects device. If device does not require any data then software may send just #tag_CommCommandHeader to select device. When command received, all communication status flags are cleared.
/// Software waits until device reacts on the command received by reading communication status flags tag_CommResponseHeader#comm_status from device.
/// If device does not answer or #COMM_STATUS_BUSY is set then device is busy, wait should be prolonged.
/// When software is confident that #COMM_STATUS_BUSY is cleared it may reads (optionally) data from the device.
///
/// Software sends data or commands to firmware as buffer which consist of #tag_CommCommandHeader structure and data following this header.
/// To receive data software must select device by sending a command to this device and make sure #COMM_STATUS_BUSY is cleared.
/// Software receives data back from the selected device as #tag_CommResponseHeader structure followed by the device data.
/// The length of read operation is defined by software, so it is possible software reads more data than device buffer has.
/// In this case, #COMM_BAD_BYTE is sent repeatedly and software is responsible for detection and handling of such situation.
/// To do this, software may pay attention on tag_CommResponseHeader#length field, this value is calculated by firmware when read is initiated and is equal to
/// amount of data in the buffer at that moment.
///
/// Master may commit several reads from the device. This behavior is device specific.
///
/// \section sect_communication_details_02 Control sum calculation
/// Control sum is being calculated by firmware on data reception from software. All data bytes, including #tag_CommCommandHeader are XORed (except tag_CommCommandHeader#control_crc), and checked against tag_CommCommandHeader#control_crc.
/// Note, that initial value for control sum is #COMM_CRC_INIT_VALUE.
/// In the case of mismatch firmware discards received buffer and communication status flag #COMM_STATUS_CRC is set.
/// Software reads are checked with use of tag_CommResponseHeader#last_crc . To check control sum software should issue two read operations, the first to read data, and the second to read previous operation control sum.
/// Thus, data integrity is verified on both paths.
///
/// \section sect_communication_details_03 Communication workflow
/// \image html communication.png
/// \image latex communication.eps
/// @}}

/// \defgroup group_communication_command Command
/// \brief Details for sending commands from software to firmware
/// @{{
/// \page page_communication_command
/// \tableofcontents
///
/// \section sect_communication_command_01 Command
///
/// Command are sent from software to firmware, as #tag_CommCommandHeader structure and optional data. Data is put into
/// receive buffer (common to all virtual devices) and ON_COMMAND device callback is called.
///

/// \def COMM_CMDBYTE_DEV_SPECIFIC_4
/// \brief Defines a custom device specific flag in command byte (offset 4)
#define COMM_CMDBYTE_DEV_SPECIFIC_4  (uint8_t)(1<<4)

/// \def COMM_CMDBYTE_DEV_SPECIFIC_5
/// \brief Defines a custom device specific flag in command byte (offset 5)
#define COMM_CMDBYTE_DEV_SPECIFIC_5  (uint8_t)(1<<5)

/// \def COMM_CMDBYTE_DEV_SPECIFIC_6
/// \brief Defines a custom device specific flag in command byte (offset 6)
#define COMM_CMDBYTE_DEV_SPECIFIC_6  (uint8_t)(1<<6)

/// \def COMM_CMDBYTE_DEV_SPECIFIC_7
/// \brief Defines a custom device specific flag in command byte (offset 7)
#define COMM_CMDBYTE_DEV_SPECIFIC_7  (uint8_t)(1<<7)

/// \def COMM_CMDBYTE_DEV_SPECIFIC_MASK
/// \brief Defines a mask for custom device specific flags in command byte
#define COMM_CMDBYTE_DEV_SPECIFIC_MASK  (uint8_t)(COMM_CMDBYTE_DEV_SPECIFIC_4 | \
                                                  COMM_CMDBYTE_DEV_SPECIFIC_5 | \
                                                  COMM_CMDBYTE_DEV_SPECIFIC_6 | \
                                                  COMM_CMDBYTE_DEV_SPECIFIC_7)

/// \def COMM_CMDBYTE_DEV_ADDRESS_MASK
/// \brief Defines a mask for device ID part of the command byte
#define COMM_CMDBYTE_DEV_ADDRESS_MASK  (uint8_t)(~COMM_CMDBYTE_DEV_SPECIFIC_MASK)

#pragma pack(push, 1)

/// \struct tag_CommCommandHeader
/// \brief This structure represent command being sent from software to firmware
typedef struct tag_CommCommandHeader{{
	uint8_t  command_byte; ///< Command byte, contains device ID and may have several device specific flags set.
	uint16_t length;	///< Length of the data that follows this structure, may be equal to 0
	uint8_t  control_crc; ///< Is a control sum. All the bytes, including this header (but excluding this value) are XORed and must be equal to this value, otherwise command will not be accepted and #COMM_STATUS_CRC will be set.
}} CommCommandHeader, *PCommCommandHeader;

#pragma pack(pop)
/// @}}

/// \defgroup group_communication_response Response
/// \brief Details for receiving response from firmware
/// @{{
/// \page page_communication_response
/// \tableofcontents
///
/// \section sect_communication_response_01 Response
/// Response is sent by firmware per software request. Response consist of #tag_CommResponseHeader structure and optional
/// data. It is important to note that length of the data transmitted completely depend on software. Software may read
/// less or more data then selected virtual device has or even just single byte. Firmware should carefully handle all such
/// situations. When transmission is over virtual device ON_READDONE callback is called.
///

/// \def COMM_STATUS_BUSY
/// \brief This communication status indicates that communication with firmware is blocked by currently executed command.
/// Do not attempt to send any commands to the device if this bit set.
/// This flag is set by communication protocol implementation immediately after reception of command byte.
#define COMM_STATUS_BUSY              128

/// \def COMM_STATUS_FAIL
/// \brief This communication status indicates that last command was not executed or recognized by device.
/// In this case command should be either repeated or software should make corresponding actions that depend on the nature of the command.
#define COMM_STATUS_FAIL              64

/// \def COMM_STATUS_CRC
/// \brief This communication status indicates that last command was not delivered successfully to device because of data corruptions detected by control sum check.
/// Master should repeat this command. Control sum check is implemented by device separately.
#define COMM_STATUS_CRC               32

/// \def COMM_STATUS_OVF
/// \brief This communication status indicates that circular buffer of the device is overflow and some data is lost
#define COMM_STATUS_OVF               16

/// \struct tag_CommResponseHeader
/// \brief This structure represent information being received by software from firmware.
#pragma pack(push, 1)
typedef struct tag_CommResponseHeader{{
	uint8_t last_crc;   ///< Control sum of the previous operation.
	uint8_t  comm_status; ///< Represents state of the firmware communication status flags. If response header is not read completely then device is not notified when transmission is done, therefore software may read just one byte in order to get communication status.
	uint16_t length;	///< Number of bytes available in the device buffer at the moment when software started receive. If device requested more data than was available #COMM_BAD_BYTE may be sent.
}} CommResponseHeader, *PCommResponseHeader;
#pragma pack(pop)
/// @}}

/// \def COMM_BAD_BYTE
/// \brief Defines bad byte to sent if software requires more data than available in device buffer on firmware side.
#define COMM_BAD_BYTE                 0xBB

/// \def COMM_BUFFER_LENGTH
/// \brief Defines communication buffer length. Software may not send commands longer than this value.
#define COMM_BUFFER_LENGTH            {__COMM_BUFFER_LENGTH__}

/// \def COMM_MAX_DEV_ADDR
/// \brief Defines maximum device ID value.
#define COMM_MAX_DEV_ADDR             15

// CRC initial value
/// \def COMM_CRC_INIT_VALUE
/// \brief Defines initial value (salt) for control sum calculation
#define COMM_CRC_INIT_VALUE           0

/// \def COMM_CRC_OFFSET
/// \brief Defines offset (in bytes) of the tag_CommCommandHeader#control_crc
#define COMM_CRC_OFFSET               3

/// \def COMM_COMMAND_BYTE_OFFSET
/// \brief Defines offset (in bytes) of the tag_CommCommandHeader#command_byte
#define COMM_COMMAND_BYTE_OFFSET      0


/// \def I2C_FIRMWARE_ADDRESS
/// \brief Defines first I2C address for the device
#define I2C_FIRMWARE_ADDRESS          {__I2C_FIRMWARE_ADDRESS__}

/// @}}