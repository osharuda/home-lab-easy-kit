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
 *   \brief {DevName} device C source file.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "fw.h"
#include "{devname}.h"
#include <stm32f10x.h>


#ifdef {DEVNAME}_DEVICE_ENABLED

/// \addtogroup group_{devname}
/// @{

#if {DEVNAME}_DEVICE_BUFFER_TYPE != DEV_NO_BUFFER
{DEVNAME}_FW_BUFFERS
#endif

/// \brief Global array that stores all virtual {DevName} devices configurations.
struct {DevName}Instance g_{devname}_devs[] = {DEVNAME}_FW_DEV_DESCRIPTOR;

/// @}

#define {DEVNAME}_DISABLE_IRQs
//    uint32_t state = NVIC_IRQ_STATE(dev->scan_complete_irqn);
//    NVIC_DISABLE_IRQ(dev->irqn, state);

#define {DEVNAME}_RESTORE_IRQs
//    NVIC_RESTORE_IRQ(dev->irqn, state);

//---------------------------- FORWARD DECLARATIONS ----------------------------

void {devname}_init_vdev(volatile struct {DevName}Instance* dev, uint16_t index) {
    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(struct DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->on_command   = {devname}_execute;
    devctx->on_read_done = {devname}_read_done;
    devctx->on_sync      = {devname}_sync;

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_LINIAR_BUFFER
    devctx->buffer       = dev->buffer;
    devctx->bytes_available = dev->buffer_size;
#endif

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
    struct CircBuffer* circbuf = (struct CircBuffer*) &(dev->circ_buffer);
    circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
    devctx->circ_buffer  = circbuf;
#endif

    comm_register_device(devctx);
}

void {devname}_init() {
    for (uint16_t i=0; i<{DEVNAME}_DEVICE_COUNT; i++) {
        struct {DevName}Instance* dev = (struct {DevName}Instance*)g_{devname}_devs+i;
        {devname}_init_vdev(dev, i);
    }
}

uint8_t {devname}_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct {DevName}Instance* dev = (struct {DevName}Instance*)g_{devname}_devs + devctx->dev_index;
    struct {DevName}PrivData* priv = &(dev->privdata);

    // Add command processing code here ...
    UNUSED(data);
    UNUSED(length);
    UNUSED(priv);

    return COMM_STATUS_OK;
}

uint8_t {devname}_read_done(uint8_t device_id, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct {DevName}Instance* dev = g_{devname}_devs + devctx->dev_index;

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
    struct CircBuffer* circbuf = (struct CircBuffer*)&(dev->circ_buffer);
    circbuf_stop_read(circbuf, length);
#endif

    UNUSED(dev);
    UNUSED(length);

    return COMM_STATUS_OK;
}

uint8_t {devname}_sync(uint8_t cmd_byte, uint16_t length) {
    UNUSED(length);
    struct DeviceContext* dev_ctx = comm_dev_context(cmd_byte);
    struct {DevName}Instance* dev = (struct {DevName}Instance*)(g_{devname}_devs + dev_ctx->dev_index);
    struct {DevName}Status* status = &dev->privdata.status;

    /// Disable device interrupts and update status visible to software
    {DEVNAME}_DISABLE_IRQs;

    /// It is safe to copy status information because device have COMM_STATUS_BUSY status at the moment. All status
    /// reads should fail because of this reason.
    dev->status.status = status->status;

    {DEVNAME}_RESTORE_IRQs;

    return COMM_STATUS_OK;
}

#endif
