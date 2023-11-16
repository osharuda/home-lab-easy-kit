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

/// \addtogroup group_pacemakerdev
/// @{{

/// \def PACEMAKERDEV_START
/// \brief Instructs to start signal sequence generation
#define PACEMAKERDEV_START                  128

/// \def PACEMAKERDEV_STOP
/// \brief Instructs to stop signal sequence generation
#define PACEMAKERDEV_STOP                   64

/// \def PACEMAKERDEV_RESET
/// \brief Instructs to reset device (sequence generation will be stopped if started)
#define PACEMAKERDEV_RESET                  (PACEMAKERDEV_STOP | PACEMAKERDEV_START)

/// \def PACEMAKERDEV_DATA
/// \brief Data transfer to the internal buffer
#define PACEMAKERDEV_DATA                   0

/// \struct tag_PaceMakerDevCommand
/// \brief This structure describes PaceMakerDev command
#pragma pack(push, 1)

/// \struct tag_PaceMakerStatus
/// \brief Structure that describes status of the PaceMaker device
typedef struct tag_PaceMakerStatus {{
    uint32_t main_counter;                        /// Number of main cycles remains to the finish. If zero and started
                                                  /// is set, then infinite cycling is used.
    EKIT_ERROR last_error;                        /// Last error code
    struct {{
        uint16_t started        : 1;              /// If set, signal generation is running, otherwise cleared.
        uint16_t internal_index : 15;             /// Index of the current internal transition
    }};
}} PaceMakerStatus;
typedef volatile PaceMakerStatus* PPaceMakerStatus;

/// \struct tag_PaceMakerTransition
/// \brief Structure that describes signal transition
typedef struct tag_PaceMakerTransition {{
    uint32_t signal_mask;    /// Pins state
    uint16_t prescaller;     /// Period for the wait timer
    uint16_t counter;        /// Prescaller for the wait timer
}} PaceMakerTransition;

/// \struct tag_PaceMakerStartCommand
/// \brief Structure that describes signal transition
typedef struct tag_PaceMakerStartCommand {{
    uint32_t main_cycles_number;  /// Amount of main cycles until generation is stopped. If zero infinite cycling is used.
    uint16_t main_prescaller;     /// Main cycle prescaller.
    uint16_t main_counter;        /// Main cycle counter.
}} PaceMakerStartCommand;

/// \struct tag_PaceMakerDevData
/// \brief Structure that describes command for PaceMakerDev
typedef struct tag_PaceMakerDevData {{
    uint32_t transition_number; /// Amount of described transitions, may not be a zero.
    PaceMakerTransition transitions[];
}} PaceMakerDevData;

#pragma pack(pop)
/// @}}
