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

/// \def PACEMAKERDEV_DEVICE_COUNT
/// \brief Number of PaceMakerDev devices being used
#define PACEMAKERDEV_DEVICE_COUNT {__PACEMAKERDEV_DEVICE_COUNT__}

#define DEV_NO_BUFFER           0
#define DEV_LINIAR_BUFFER       1
#define DEV_CIRCULAR_BUFFER     2
#define PACEMAKERDEV_DEVICE_BUFFER_TYPE DEV_LINIAR_BUFFER

/// \addtogroup group_pacemakerdev
/// @{{

/// \def PACEMAKERDEV_RESERVED_2
/// \brief Defines PaceMakerDev command specific flag 2
#define PACEMAKERDEV_RESERVED_2             128

/// \def PACEMAKERDEV_RESERVED_1
/// \brief Defines PaceMakerDev command specific flag 1
#define PACEMAKERDEV_RESERVED_1             64

/// \def PACEMAKERDEV_RESERVED_0
/// \brief Defines PaceMakerDev command specific flag 0
#define PACEMAKERDEV_RESERVED_0             32

/// \struct tag_PaceMakerDevCommand
/// \brief This structure describes PaceMakerDev command
#pragma pack(push, 1)

/// \struct tag_PaceMakerTransition
/// \brief Structure that describes signal transition
typedef struct tag_PaceMakerTransition {{
    uint32_t signal_mask;    /// Pins state
    uint16_t prescaller;     /// Period for the wait timer
    uint16_t counter;        /// Prescaller for the wait timer
}} PaceMakerTransition;

/// \struct tag_PaceMakerDevData
/// \brief Structure that describes command for PaceMakerDev
typedef struct tag_PaceMakerDevData {{
    uint32_t default_mask;      /// Default state of the pins
    uint16_t cycle_prescaller;  /// Cycle period for the wait timer
    uint16_t cycle_counter;     /// Cycle counter for the wait timer
    uint16_t transition_number; /// Amount of described transitions
    PaceMakerTransition transitions[];
}} PaceMakerDevData;

#pragma pack(pop)
/// @}}
