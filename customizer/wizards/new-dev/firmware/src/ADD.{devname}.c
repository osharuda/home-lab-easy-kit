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
volatile {DevName}Instance g_{devname}_devs[] = {DEVNAME}_FW_DEV_DESCRIPTOR;

/// @}

//---------------------------- FORWARD DECLARATIONS ----------------------------

void {devname}_init_vdev(volatile {DevName}Instance* dev, uint16_t index) {
    volatile PDeviceContext devctx = (volatile PDeviceContext)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->on_command   = {devname}_execute;
    devctx->on_read_done = {devname}_read_done;

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_LINIAR_BUFFER
    devctx->buffer       = dev->buffer;
    devctx->bytes_available = dev->buffer_size;
#endif

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
    // Init circular buffer
    volatile struct CircBuffer* circbuf = (volatile struct CircBuffer*) &(dev->circ_buffer);
    circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
    devctx->circ_buffer  = circbuf;
#endif

    comm_register_device(devctx);
}

void {devname}_init() {
    for (uint16_t i=0; i<{DEVNAME}_DEVICE_COUNT; i++) {
        volatile {DevName}Instance* dev = (volatile {DevName}Instance*)g_{devname}_devs+i;
        {devname}_init_vdev(dev, i);
    }
}

void {devname}_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(cmd_byte);
    volatile {DevName}Instance* dev = (volatile {DevName}Instance*)g_{devname}_devs + devctx->dev_index;
    volatile {DevName}PrivData* priv = &(dev->privdata);

    // Add command processing code here ...
    UNUSED(data);
    UNUSED(length);
    UNUSED(priv);

    comm_done(0);
}

void {devname}_read_done(uint8_t device_id, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(device_id);
    volatile {DevName}Instance* dev = g_{devname}_devs + devctx->dev_index;

#if {DEVNAME}_DEVICE_BUFFER_TYPE == DEV_CIRCULAR_BUFFER
    volatile struct CircBuffer* circbuf = (volatile struct CircBuffer*)&(dev->circ_buffer);
    circbuf_stop_read(circbuf, length);
    circbuf_clear_ovf(circbuf);
#endif

    UNUSED(dev);
    UNUSED(length);

    comm_done(0);
}

#endif
