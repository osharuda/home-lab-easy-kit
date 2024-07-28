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
 *   \brief INFODev device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef INFO_DEVICE_ENABLED

#include <assert.h>
#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "info_dev.h"
#include "info_conf.h"



struct DeviceContext g_info_devctx __attribute__ ((aligned));
uint8_t g_info_uuid[] = INFO_UUID;

uint8_t info_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length);
uint8_t info_read_done(uint8_t device_id, uint16_t length);

void info_dev_init() {
    assert(INFO_UUID_LEN==sizeof(g_info_uuid));
    memset((void*)&g_info_devctx, 0, sizeof(g_info_devctx));

    g_info_devctx.device_id = INFO_ADDR;
    g_info_devctx.buffer = g_info_uuid;
    g_info_devctx.bytes_available = sizeof(g_info_uuid);
    g_info_devctx.on_command = info_dev_execute;
    g_info_devctx.on_read_done = info_read_done;

    comm_register_device(&g_info_devctx);
}

uint8_t info_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    UNUSED(cmd_byte);
    UNUSED(data);
    UNUSED(length);
    return COMM_STATUS_FAIL;
}

uint8_t info_read_done(uint8_t device_id, uint16_t length) {
	UNUSED(device_id);
	UNUSED(length);
    return COMM_STATUS_OK;
}


#endif