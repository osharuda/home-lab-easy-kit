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

/*!  \file
 *   \brief RTC (Real Time Clock) device C source file.
 *   \author Oleh Sharuda
 */
#include "fw.h"
#ifdef RTC_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "rtc.h"
#include "rtc_conf.h"

volatile DeviceContext rtc_context __attribute__ ((aligned));
uint32_t rtc_data = 0;

void set_rtc_val(uint32_t ts) {
	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();

	// Change the current time
	RTC_SetCounter(ts);

	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();
}

void rtc_on_command(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
	UNUSED(cmd_byte);
	uint8_t status = 0;
	if (length==sizeof(rtc_data)) {
		// set
		uint32_t ts = *((uint32_t*)data);
		set_rtc_val(ts);
	} else if (length==0){
	} else {
		status = COMM_STATUS_FAIL;
	}

	rtc_data = RTC_GetCounter();

    comm_done(status);
}

void rtc_init()
{
	// Allow access to BKP Domain
	PWR_BackupAccessCmd(ENABLE);

	if (BKP_ReadBackupRegister(RTC_BACKUP_REG) != RTC_MAGIC_NUM)
	{
		// Reset Backup Domain
		BKP_DeInit();

		// Enable LSE
		RCC_LSEConfig(RCC_LSE_ON);

		// Wait till LSE is ready
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}

		// Select LSE as RTC Clock Source
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		// Enable RTC Clock
		RCC_RTCCLKCmd(ENABLE);

		// Wait for RTC registers synchronization
		RTC_WaitForSynchro();

		// Wait until last write operation on RTC registers has finished
		RTC_WaitForLastTask();

		// Disable the RTC Second interrupt
		RTC_ITConfig(RTC_IT_SEC, DISABLE);

		// Wait until last write operation on RTC registers has finished
		RTC_WaitForLastTask();

		// Set RTC prescaler: set RTC period to 1sec
		RTC_SetPrescaler(32767); // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)

		// Wait until last write operation on RTC registers has finished
		RTC_WaitForLastTask();

		// Change the current time
		RTC_SetCounter(0);

		// Wait until last write operation on RTC registers has finished
		RTC_WaitForLastTask();

	    BKP_WriteBackupRegister(RTC_BACKUP_REG, RTC_MAGIC_NUM);
    } else {
	    // Wait for RTC registers synchronization
	    RTC_WaitForSynchro();

	    // Disable the RTC Second interrupt
	    RTC_ITConfig(RTC_IT_SEC, DISABLE);

	    // Wait until last write operation on RTC registers has finished
	    RTC_WaitForLastTask();
	}

    memset((void*)&rtc_context, 0, sizeof(rtc_context));
    rtc_context.device_id = RTC_ADDR;
    rtc_context.buffer = (uint8_t*) &rtc_data;
    rtc_context.bytes_available = sizeof(rtc_data);
    rtc_context.circ_buffer = 0;
    rtc_context.on_command = rtc_on_command;
    rtc_context.on_read_done = 0;

    comm_register_device(&rtc_context);
}

#endif