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

/// \addtogroup group_{devname}
/// @{{

/// \def {DEVNAME}_RESERVED_3
/// \brief Defines {DevName} command specific flag 3
#define {DEVNAME}_RESERVED_3             128

/// \def {DEVNAME}_RESERVED_2
/// \brief Defines {DevName} command specific flag 2
#define {DEVNAME}_RESERVED_2             64

/// \def {DEVNAME}_RESERVED_1
/// \brief Defines {DevName} command specific flag 1
#define {DEVNAME}_RESERVED_1             32

/// \def {DEVNAME}_RESERVED_0
/// \brief Defines {DevName} command specific flag 0
#define {DEVNAME}_RESERVED_0             16

/// \struct tag_{DevName}Command
/// \brief This structure describes {DevName} command
#pragma pack(push, 1)
typedef struct tag_{DevName}Command {{
    uint16_t data;  ///< Some data. Customize it for particular device.
}} {DevName}Command;
#pragma pack(pop)
/// @}}
