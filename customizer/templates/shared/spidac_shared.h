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

/// \addtogroup group_spidac
/// @{{

typedef enum {{
    START = 0x10,
    START_PERIOD = 0x20,
    STOP = 0x30,
    DATA = 0x40,
    SETDEFAULT = 0x50
}} SPIDAC_COMMAND;

typedef enum {{
    STARTED = 0,
    STARTING = 1,
    STOPPING = 2,
    RESETTING = 3,
    SHUTDOWN = 4
}} SPIDAC_STATUS;

typedef enum {{
    LSB = 0,
    MSB = 1
}} SPIDAC_FRAME_FORMAT;

#pragma pack(push, 1)
/// \struct tag_SPIDACSampling
/// \brief Structure that describes sampling of the SPIDAC device
typedef struct tag_SPIDACSampling {{
    uint16_t prescaler;
    uint16_t period;
    uint16_t phase_increment;   // Sample increment in bytes (number of frames per sample * frame size)
}} SPIDACSampling;
typedef volatile SPIDACSampling* PSPIDACSampling;

/// \struct tag_SPIDACStatus
/// \brief Structure that describes status of the SPIDAC device
typedef struct tag_SPIDACStatus {{
    uint8_t status;             /// Describes status of the device.
    uint8_t repeat_count;
    SPIDACSampling sampling;
}} SPIDACStatus;
typedef volatile SPIDACStatus* PSPIDACStatus;
#pragma pack(pop)

/// @}}
