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
 *   \brief Generated include header of software part for Can device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once

#include <stdint.h>



/*
#define DEV_NO_BUFFER           0
#define DEV_LINIAR_BUFFER       1
#define DEV_CIRCULAR_BUFFER     2
#define CAN_DEVICE_BUFFER_TYPE DEV_CIRCULAR_BUFFER
*/

/// \addtogroup group_can
/// @{

/// \def CAN_SEND
/// \brief Instructs to send message over the CAN bus. CAN device must be started (with CAN_START).
#define CAN_SEND              128

/// \def CAN_FILTER
/// \brief Applies filter. CAN device must be stopped in order to apply filter (with CAN_STOP).
#define CAN_FILTER            64

/// \def CAN_STOP
/// \brief Stops CAN device.
#define CAN_STOP              32

/// \def CAN_START
/// \brief Starts CAN device.
#define CAN_START             16

/// \def CAN_MAX_FILTER_COUNT
/// \brief Defines maximum number of filters.
#define CAN_MAX_FILTER_COUNT  14

/// \def CAN_MSG_EXTENDED_ID
/// \brief Specifies extended id to be used.
#define CAN_MSG_EXTENDED_ID     (1 << 7)

/// \def CAN_MSG_REMOTE_FRAME
/// \brief Specifies remote frame to be used.
#define CAN_MSG_REMOTE_FRAME    (1 << 6)

/// \def CAN_MSG_MAX_DATA_LEN
/// \brief Specifies data length.
#define CAN_MSG_MAX_DATA_LEN    (8)

/// \def CAN_MSG_MAX_DATA_LEN_MASK
/// \brief Specifies bit mask in order to get data length.
#define CAN_MSG_MAX_DATA_LEN_MASK   (0x0F)

#pragma pack(push, 1)
/// \struct tag_CanSendCommand
/// \brief This structure describes Send command. This structure is passed with CAN_SEND.
typedef struct tag_CanSendCommand {
    uint32_t id;        ///< Standard identifier. Values: [0 ... 0x7FF].

    uint32_t ext_id;     ///< Extended identifier. Values: [0 ... 0x1FFFFFFF].

    uint8_t extra;      ///< Extra information for the message. For details take a look on set of CAN_MSG_XXX macro.

    uint8_t data[];     ///< Data to be transmitted. Variable size array - must not exceed CAN_MSG_MAX_DATA_LEN.
} CanSendCommand;
#pragma pack(pop)

typedef volatile CanSendCommand* PCanSendCommand;


/// \def CAN_STATE_STARTED
/// \brief If set device is started, otherwise device is stopped
#define CAN_STATE_STARTED           (1 << 0)

/// \def CAN_STATE_SLEEP
/// \brief If set device is in sleep mode, otherwise device has woken up.
#define CAN_STATE_SLEEP             (1 << 1)

/// \def CAN_STATE_MB_0_BUSY_BIT_OFFSET
/// \brief Specifies bit position of the CAN_STATE_MB_0_BUSY flag.
#define CAN_STATE_MB_0_BUSY_BIT_OFFSET (2)

/// \def CAN_STATE_MB_0_BUSY
/// \brief If set mailbox 0 is busy
#define CAN_STATE_MB_0_BUSY         (1 << CAN_STATE_MB_0_BUSY_BIT_OFFSET)

/// \def CAN_STATE_MB_1_BUSY
/// \brief If set mailbox 1 is busy
#define CAN_STATE_MB_1_BUSY         (1 << (CAN_STATE_MB_0_BUSY_BIT_OFFSET+1))

/// \def CAN_STATE_MB_2_BUSY
/// \brief If set mailbox 2 is busy
#define CAN_STATE_MB_2_BUSY         (1 << (CAN_STATE_MB_0_BUSY_BIT_OFFSET+2))

/// \def CAN_ERROR_OVERFLOW
/// \brief Internal buffer overflow
#define CAN_ERROR_OVERFLOW          (1 << 5)

/// \def CAN_ERROR_FIFO_0_FULL
/// \brief FIFO 0 full
#define CAN_ERROR_FIFO_0_FULL       (1 << 6)

/// \def CAN_ERROR_FIFO_0_OVERFLOW
/// \brief FIFO 0 overflow
#define CAN_ERROR_FIFO_0_OVERFLOW   (1 << 7)

/// \def CAN_ERROR_FIFO_1_FULL
/// \brief FIFO 1 overflow
#define CAN_ERROR_FIFO_1_FULL       (1 << 8)

/// \def CAN_ERROR_FIFO_1_OVERFLOW
/// \brief FIFO 1 overflow
#define CAN_ERROR_FIFO_1_OVERFLOW   (1 << 9)

/// \def CAN_ERROR_WARNING
/// \brief Error warning
#define CAN_ERROR_WARNING           (1 << 10)

/// \def CAN_ERROR_PASSIVE
/// \brief Error passive
#define CAN_ERROR_PASSIVE           (1 << 11)

/// \def CAN_ERROR_BUS_OFF
/// \brief Bus off
#define CAN_ERROR_BUS_OFF           (1 << 12)

/// \def CAN_ERROR_NO_MAILBOX
/// \brief No mail box was found during transmission
#define CAN_ERROR_NO_MAILBOX        (1 << 13)

/// \def CAN_ESR_LEC_MASK
/// \brief Last error code mask.
#define CAN_ESR_LEC_MASK            (0b01110000)

/// \def CAN_ESR_LEC_OK
/// \brief Last error code: ok.
#define CAN_ESR_LEC_OK              (0b00000000)

/// \def CAN_ESR_LEC_STUFF_ERR
/// \brief Last error code: stuff error.
#define CAN_ESR_LEC_STUFF_ERR       (0b00010000)

/// \def CAN_ESR_LEC_FORM_ERR
/// \brief Last error code: form error.
#define CAN_ESR_LEC_FORM_ERR       (0b00100000)

/// \def CAN_ESR_LEC_ACK_ERR
/// \brief Last error code: acknowledgment error.
#define CAN_ESR_LEC_ACK_ERR       (0b00110000)

/// \def CAN_ESR_LEC_REC_ERR
/// \brief Last error code: bit recessive error.
#define CAN_ESR_LEC_REC_ERR       (0b01000000)

/// \def CAN_ESR_LEC_DOM_ERR
/// \brief Last error code: bit dominant error.
#define CAN_ESR_LEC_DOM_ERR       (0b01010000)

/// \def CAN_ESR_LEC_CRC_ERR
/// \brief Last error code: CRC error.
#define CAN_ESR_LEC_CRC_ERR       (0b01100000)

/// \def CAN_ESR_LEC_SFT_ERR
/// \brief Last error code: set by software.
#define CAN_ESR_LEC_SFT_ERR       (0b01110000)

/// \def CAN_ESR_FLAG_WARNING
/// \brief Error warning flag.
#define CAN_ESR_FLAG_WARNING      (1)

/// \def CAN_ESR_FLAG_PASSIVE
/// \brief Error passive flag.
#define CAN_ESR_FLAG_PASSIVE      (2)

/// \def CAN_ESR_FLAG_BUSOFF
/// \brief Bus-off flag.
#define CAN_ESR_FLAG_BUSOFF       (4)

#pragma pack(push, 1)
/// \struct tag_CanStatus
/// \brief This structure describes can device status.
typedef struct tag_CanStatus {
    uint16_t data_len;   ///< Number of bytes available in buffer.
    uint16_t state;      ///< State bitmask. Consist of CAN_STATE_XXX and CAN_ERROR_XXX bits.
    uint8_t  last_error; ///< Last error code.
    uint8_t  recv_error_count; ///< Receive error counter.
    uint8_t  lsb_trans_count; ///< LSB of the 9-bit CAN Transmit Error Counter.
} CanStatus;
#pragma pack(pop)

typedef volatile CanStatus* PCanStatus;

#pragma pack(push, 1)
/// \struct tag_CanRecvMessage
/// \brief This structure describes received message. This structure is wrote into output circular buffer to be read by
///        software.
typedef struct tag_CanRecvMessage {
    uint32_t id;        ///< Standard 11-bit identifier (values: [0 ... 0x7FF]) or extended 29-bit identifier (values [0 ... 0x1FFFFFFF])
                        ///  Test #extra for #CAN_MSG_EXTENDED_ID bit to distinguish standard and extended messages.

    uint8_t extra;      ///< Extra information for the message. For details take a look on set of CAN_MSG_XXX macro.

    uint8_t fmi;        ///< Index of the message filter.

    uint8_t data[CAN_MSG_MAX_DATA_LEN];     ///< Data to be transmitted.
} CanRecvMessage;
#pragma pack(pop)

typedef volatile CanRecvMessage* PCanRecvMessage;

/// \def CAN_FLT_MAX_INDEX
/// \brief Specifies maximum index of the filter
#define CAN_FLT_MAX_INDEX       (13)

/// \def CAN_FLT_INDEX_MASK
/// \brief Specifies mask in order to retrieve index from flags fields.
#define CAN_FLT_INDEX_MASK      (0x0F)

/// \def CAN_FLT_LIST_MODE
/// \brief Filter list mode. If bit is set (1) Id list mode is used; If bit is cleared (0) Id mask mode is used.
#define CAN_FLT_LIST_MODE       (1 << 4)

/// \def CAN_FLT_SCALE
/// \brief 32bit or 16bit scaling. If bit is set (1) 32 bit scaling is used; If bit is cleared (0) 16 bit scaling is used.
#define CAN_FLT_SCALE           (1 << 5)

/// \def CAN_FLT_FIFO
/// \brief FIFO to be used: if bit is set (1) FIFO1 is used; If bit is cleared (0) FIFO0 is used.
#define CAN_FLT_FIFO            (1 << 6)

/// \def CAN_FLT_ENABLE
/// \brief Specifies if filter is enabled: if bit is set (1) filter is enabled; If bit is cleared (0) is disabled.
#define CAN_FLT_ENABLE          (1 << 7)


#pragma pack(push, 1)
/// \struct tag_FilterCommand
/// \brief This structure describes Filter command. This structure is passed with CAN_FILTER.
typedef struct tag_CanFilterCommand {
    uint16_t id_msb;         ///< Filter identification number: MSBs for a 32-bit configuration,
                             ///  first one for a 16-bit configuration.

    uint16_t id_lsb;         ///< Filter identification number: LSBs for a 32-bit configuration,
                             ///  second one for a 16-bit configuration.

    uint16_t mask_msb;       ///< Filter mask number or identification number (depends on mode):
                             ///  MSBs for a 32-bit configuration, first one for a 16-bit configuration.

    uint16_t mask_lsb;       ///< Filter mask number or identification number (depends on mode):
                             ///  LSBs for a 32-bit configuration, second one for a 16-bit configuration.

    uint8_t flags;           ///< Flags that specify filter behaviour.
} CanFilterCommand;
#pragma pack(pop)

typedef volatile CanFilterCommand* PCanFilterCommand;
/// @}



/// \addtogroup group_can
/// @{

/// \struct tag_CANConfig
/// \brief CAN configuration structure.
typedef struct tag_CANConfig{
    uint8_t         dev_id;             ///< Device ID for Can virtual device
    const char*     dev_name;           ///< Name of the Can virtual device as given in JSON configuration file
    uint16_t        dev_buffer_len;     ///< Length of the Can internal buffer
} CANConfig;

/// @}