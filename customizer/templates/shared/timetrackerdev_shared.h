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

#define TIMETRACKERDEV_STATUS_STOPPED        0
#define TIMETRACKERDEV_STATUS_STARTED        1


/// \def TIMETRACKERDEV_RESERVED_3
/// \brief Defines TimeTrackerDev command specific flag 3
#define TIMETRACKERDEV_RESERVED_3             128

/// \def TIMETRACKERDEV_RESERVED_2
/// \brief Defines TimeTrackerDev command specific flag 2
#define TIMETRACKERDEV_RESERVED_2             64

/// \def TIMETRACKERDEV_RESET
/// \brief Defines reset option for TimeTrackerDev command. If flag is specified, device must be reset.
#define TIMETRACKERDEV_RESET                  32

/// \def TIMETRACKERDEV_START
/// \brief Defines start TimeTrackerDev command. If flag is specified, the device in instructed
///        to switch into started state. If flag is not set, the device should be switched to stopped state.
#define TIMETRACKERDEV_START                  16

/// \struct tag_TimeTrackerStatus
/// \brief Structure that describes status of the TimeTrackerDev device
#pragma pack(push, 1)
typedef struct tag_TimeTrackerStatus {{
    uint64_t last_reset;  ///< Timestamp of the last reset operation.
    uint16_t event_number;///< Number of events accumulated.
    uint8_t status;       ///< Current status.
}} TimeTrackerStatus;
#pragma pack(pop)
typedef volatile TimeTrackerStatus* PTimeTrackerStatus;
/// @}}
