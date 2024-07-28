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

 /* --------------------> END OF THE TEMPLATE HEADER <-------------------- */

/// \addtogroup group_timetrackerdev
/// @{{

/// \def TIMETRACKERDEV_START
/// \brief Defines TimeTrackDev "Start" command. This command is sent via device specific
///        part of the command byte
#define TIMETRACKERDEV_START                  (1 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def TIMETRACKERDEV_STOP
/// \brief Defines TimeTrackDev "Stop" command. This command is sent via device specific
///        part of the command byte
#define TIMETRACKERDEV_STOP                   (2 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def TIMETRACKERDEV_RESET
/// \brief Defines TimeTrackDev "Reset" command. This command is sent via device specific
///        part of the command byte
/// \note Device must be in TIMETRACKERDEV_STATUS_STOPPED state in order to execute this command.
#define TIMETRACKERDEV_RESET                  (3 << COMM_CMDBYTE_SPECIFIC_OFFSET)

/// \def TIMETRACKERDEV_STATUS_STOPPED
/// \brief Defines TimeTrackDev device "STOPPED" status.
#define TIMETRACKERDEV_STATUS_STOPPED        0

/// \def TIMETRACKERDEV_STATUS_STARTED
/// \brief Defines TimeTrackDev device "STARTED" (running) status.
#define TIMETRACKERDEV_STATUS_STARTED        1


/// \struct tag_TimeTrackerStatus
/// \brief Structure that describes status of the TimeTrackerDev device
#pragma pack(push, 1)
struct TimeTrackerStatus {{
    uint64_t first_event_ts; ///< Timestamp of the first event after reset operation. If UINT64_MAX no events since reset.
    uint16_t event_number;   ///< Number of events accumulated.
    uint8_t status;          ///< Current status.
}};
#pragma pack(pop)
/// @}}
