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

/// \def RTC_ADDR
/// \brief RTCDev device id
#define RTC_ADDR                    {__DEVICE_ID__}

#pragma pack(push, 1)
/// \struct tag_RtcData
/// \brief RTC data
typedef struct tag_RtcData {{
	uint32_t rtcval;    ///< RTC value in seconds since epoch.
}} RtcData;
#pragma pack(pop)

typedef volatile RtcData* PRtcData;