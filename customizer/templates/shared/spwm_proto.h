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

/// \def SPWM_ADDR
/// \brief SPWMDev virtual device id
#define SPWM_ADDR               {__DEVICE_ID__}

/// \def SPWM_CHANNEL_COUNT
/// \brief Numbed of the channels configured.
#define SPWM_CHANNEL_COUNT      {__SPWM_CHANNEL_COUNT__}

/// \def SPWM_MAX_PWM_ENTRIES_COUNT
/// \brief Maximum number of PWM entries
#define SPWM_MAX_PWM_ENTRIES_COUNT  {__SPWM_MAX_PWM_ENTRIES_COUNT__}

/// \def SPWM_PORT_COUNT
/// \brief Number of ports used
#define SPWM_PORT_COUNT         {__SPWM_PORT_COUNT__}

/// \def SPWM_PRESCALE_VALUE
/// \brief Prescaller value
#define SPWM_PRESCALE_VALUE     {__SPWM_PRESCALE_VALUE__}

#pragma pack(push, 1)

/// \struct tag_PWM_ENTRY
/// \brief Defines single PWM entry
typedef struct tag_PWM_ENTRY {{

	uint16_t n_periods;             ///< Amount of timer periods until the next PWM entry. This value is put in ARR timer
	                                ///  register.

	uint16_t data[SPWM_PORT_COUNT]; ///< PORT values to be set.

}} PWM_ENTRY;

#pragma pack(pop)

typedef volatile PWM_ENTRY* PPWM_ENTRY;