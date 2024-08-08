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
 *   \brief TimeTrackerDev device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"

#ifdef TIMETRACKERDEV_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "timetrackerdev_conf.h"
#include "timetrackerdev.h"
#include <stm32f10x.h>
#include "extihub.h"

/// \addtogroup group_timetrackerdev
/// @{

#if TIMETRACKERDEV_DEVICE_BUFFER_TYPE != DEV_NO_BUFFER
TIMETRACKERDEV_FW_BUFFERS
#endif

/// \brief Global array that stores all virtual TimeTrackerDev devices configurations.
struct TimeTrackerDevInstance g_timetrackerdev_devs[] = TIMETRACKERDEV_FW_DEV_DESCRIPTOR;

/// @}

#define TIMERTACKER_DISABLE_IRQ                                                          \
    exti_mask_callback(dev->interrup_line.port, dev->interrup_line.pin_number);

#define TIMERTACKER_ENABLE_IRQ \
    if (dev->privdata.status.status==TIMETRACKERDEV_STATUS_STARTED) {                   \
        exti_unmask_callback(dev->interrup_line.port, dev->interrup_line.pin_number);   \
    }

//---------------------------- FORWARD DECLARATIONS ----------------------------
static uint8_t timetrackerdev_stop(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv);
static uint8_t timetrackerdev_start(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv);
static uint8_t timetrackerdev_reset(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv);
static uint8_t timetrackerdev_sync(uint8_t cmd_byte, uint16_t length);

#if SYSTICK_VERIFICATION!=0
static volatile uint64_t g_systick_verify __attribute__ ((aligned)) = 0;
#endif

void timetrackerdev_exti_handler(uint64_t clock, volatile void* ctx) {
    uint8_t dev_index = (uint32_t)(ctx);
    struct TimeTrackerDevInstance* dev = g_timetrackerdev_devs + dev_index;
    struct CircBuffer* circbuf = (struct CircBuffer*)&dev->circ_buffer;

#if SYSTICK_VERIFICATION!=0
    if (g_systick_verify >= clock) {
        assert_param(0);
    }
    g_systick_verify = clock;
#endif

    volatile uint64_t* data = circbuf_reserve_block(circbuf);
    if (data) {
        *data = clock;
        circbuf_commit_block(circbuf);

        dev->privdata.status.event_number++;
        GPIO_WriteBit(dev->near_full_line.port,
                      dev->near_full_line.pin_mask,
                      circbuf_get_wrn(circbuf));

        if (dev->privdata.status.first_event_ts == UINT64_MAX) {
            dev->privdata.status.first_event_ts = clock;
        }
    } else {
        timetrackerdev_stop(dev, &dev->privdata);
    }
}

void timetrackerdev_init_vdev(struct TimeTrackerDevInstance* dev, uint16_t index) {
    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(struct DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->on_command   = timetrackerdev_execute;
    devctx->on_read_done = timetrackerdev_read_done;
    devctx->on_sync      = timetrackerdev_sync;

    IS_SIZE_ALIGNED(&dev->privdata.status.first_event_ts);
    IS_SIZE_ALIGNED(&dev->privdata.status.event_number);

    dev->privdata.status.status = 0;

    // Init circular buffer
    struct CircBuffer* circbuf = &(dev->circ_buffer);
    circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
    devctx->circ_buffer  = circbuf;
    circbuf_init_block_mode(circbuf, sizeof(uint64_t));
    circbuf_init_status(circbuf,
                        (uint8_t*)&(dev->privdata.status),
                        sizeof(struct TimeTrackerStatus));

    comm_register_device(devctx);
    uint32_t di = devctx->dev_index;
    START_PIN_DECLARATION;
    DECLARE_PIN(dev->near_full_line.port, dev->near_full_line.pin_mask, dev->near_full_line.type);


    dev->privdata.status.status = 0;
    timetrackerdev_reset(dev, &dev->privdata);

    exti_register_callback(dev->interrup_line.port,
                           dev->interrup_line.pin_number,
                           dev->interrup_line.type,
                           dev->intrrupt_exci_cr,
                           dev->trig_on_rise,
                           dev->trig_on_fall,
                           timetrackerdev_exti_handler,
                           (volatile void*)di,
                           1);
}

void timetrackerdev_init() {
    for (uint16_t i=0; i<TIMETRACKERDEV_DEVICE_COUNT; i++) {
        struct TimeTrackerDevInstance* dev = (struct TimeTrackerDevInstance*)g_timetrackerdev_devs+i;
        timetrackerdev_init_vdev(dev, i);
    }
}

uint8_t timetrackerdev_start(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv) {
    /// Writes to bytes are always atomic
    TIMERTACKER_DISABLE_IRQ
    priv->status.status = TIMETRACKERDEV_STATUS_STARTED;
    TIMERTACKER_ENABLE_IRQ
    return COMM_STATUS_OK;
}

uint8_t timetrackerdev_stop(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv) {
    TIMERTACKER_DISABLE_IRQ
    priv->status.status = TIMETRACKERDEV_STATUS_STOPPED;
    TIMERTACKER_ENABLE_IRQ; /// Note: this call is added for code clarity, it will not enable IRQ, since device is stopped.
    return COMM_STATUS_OK;
}

uint8_t timetrackerdev_reset(struct TimeTrackerDevInstance* dev, struct TimeTrackerDevPrivData* priv) {
    UNUSED(priv);
    if (dev->privdata.status.status != TIMETRACKERDEV_STATUS_STOPPED) {
        return COMM_STATUS_FAIL;
    }

    /// Note: timetrackerdev_reset() should always run when timetracker device is stopped, so this is the only context
    ///       accessing cicula buffer. I2C bus may not access it because device is busy during this call.
    circbuf_reset((volatile struct CircBuffer *) &dev->circ_buffer);
    dev->privdata.status.event_number = 0;
    dev->privdata.status.first_event_ts = UINT64_MAX;
    GPIO_WriteBit(dev->near_full_line.port, dev->near_full_line.pin_mask, dev->near_full_line.default_val);
    return COMM_STATUS_OK;
}

uint8_t timetrackerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    UNUSED(data);
    UNUSED(length);

    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct TimeTrackerDevInstance* dev = (struct TimeTrackerDevInstance*)g_timetrackerdev_devs + devctx->dev_index;
    struct TimeTrackerDevPrivData* priv = &(dev->privdata);
    uint8_t command = cmd_byte & COMM_CMDBYTE_SPECIFIC_MASK;
    uint8_t res = COMM_STATUS_FAIL;

    switch (command) {
        case TIMETRACKERDEV_START:
            res = timetrackerdev_start(dev, priv);
        break;

        case TIMETRACKERDEV_STOP:
            res = timetrackerdev_stop(dev, priv);
        break;

        case TIMETRACKERDEV_RESET:
            res = timetrackerdev_reset(dev, priv);
        break;
    }

    return res;
}

uint8_t timetrackerdev_read_done(uint8_t device_id, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct TimeTrackerDevInstance* dev = g_timetrackerdev_devs + devctx->dev_index;

    volatile struct CircBuffer* circbuf = (volatile struct CircBuffer*)&(dev->circ_buffer);

    circbuf_stop_read(circbuf, length);

    uint16_t curlen = circbuf_len(circbuf);
    dev->privdata.status.event_number =  curlen / sizeof(uint64_t);
    // Either length is less than size of the status or number of bytes read from buffer consist of complete 8 byte blocks
    assert_param( (length < sizeof(struct TimeTrackerStatus)) ||
                 ((length - sizeof(struct TimeTrackerStatus)) % sizeof(uint64_t)) == 0);


    GPIO_WriteBit(dev->near_full_line.port,
                  dev->near_full_line.pin_mask,
                  circbuf_get_wrn(circbuf));

    return COMM_STATUS_OK;
}

uint8_t timetrackerdev_sync(uint8_t cmd_byte, uint16_t length) {
    UNUSED(length);
    struct DeviceContext* dev_ctx = comm_dev_context(cmd_byte);
    struct TimeTrackerDevInstance* dev = (struct TimeTrackerDevInstance*)(g_timetrackerdev_devs + dev_ctx->dev_index);
    struct TimeTrackerStatus* status = &dev->privdata.status;
    struct TimeTrackerStatus* comm_status = &dev->privdata.comm_status;

    TIMERTACKER_DISABLE_IRQ
    /// It is safe to copy status information because device have COMM_STATUS_BUSY status at the moment. All status
    /// reads should fail because of this reason.
    memcpy(comm_status, status, sizeof(struct TimeTrackerStatus));
    TIMERTACKER_ENABLE_IRQ

    return COMM_STATUS_OK;
}

#endif
