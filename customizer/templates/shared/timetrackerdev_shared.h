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

/// \def TIMETRACKERDEV_RESERVED_3
/// \brief Defines TimeTrackerDev command specific flag 3
#define TIMETRACKERDEV_RESERVED_3             128

/// \def TIMETRACKERDEV_RESERVED_2
/// \brief Defines TimeTrackerDev command specific flag 2
#define TIMETRACKERDEV_RESERVED_2             64

/// \def TIMETRACKERDEV_RESERVED_1
/// \brief Defines TimeTrackerDev command specific flag 1
#define TIMETRACKERDEV_RESERVED_1             32

/// \def TIMETRACKERDEV_RESERVED_0
/// \brief Defines TimeTrackerDev command specific flag 0
#define TIMETRACKERDEV_RESERVED_0             16

/// \struct tag_TimeTrackerDevCommand
/// \brief This structure describes TimeTrackerDev command
#pragma pack(push, 1)
typedef struct tag_TimeTrackerDevCommand {{
    uint16_t data;  ///< Some data. Customize it for particular device.
}} TimeTrackerDevCommand;
#pragma pack(pop)
/// @}}
