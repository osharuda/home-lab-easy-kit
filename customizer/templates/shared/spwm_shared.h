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

#pragma pack(push, 1)

/// \struct PWM_ENTRY
/// \brief Defines single PWM entry
struct PWM_ENTRY {{
	uint16_t n_periods; ///< Amount of timer periods until the next PWM entry. This value is put in ARR timer register.
    uint16_t data[];    ///< PORT values to be set.
}};
#pragma pack(pop)

#define PWM_ENTRY_SIZE(n_ports) (sizeof(struct PWM_ENTRY) + (n_ports)*sizeof(uint16_t))
#define GET_PWM_ENTRY_BY_INDEX(entry_array, index, entry_size) ((struct PWM_ENTRY*)( (uint8_t*)(entry_array) + (entry_size)*(index) ))

