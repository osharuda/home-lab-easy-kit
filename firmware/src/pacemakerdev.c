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
 *   \brief PaceMakerDev device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef PACEMAKERDEV_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "pacemakerdev.h"
#include <stm32f10x.h>




/// \addtogroup group_pacemakerdev
/// @{

/// \brief Definition for initialization gpio functions
PACEMAKERDEV_FW_INIT_GPIO_FUNCTIONS

/// \brief Definition for set gpio functions
PACEMAKERDEV_FW_SET_GPIO_FUNCTIONS

// \brief Definitions of buffers
PACEMAKERDEV_FW_BUFFERS

/// \brief Global array that stores all virtual PaceMakerDev devices configurations.
volatile PaceMakerDevInstance g_pacemakerdev_devs[] = PACEMAKERDEV_FW_DEV_DESCRIPTOR;

/// @}

//---------------------------- FORWARD DECLARATIONS ----------------------------
/// \brief Start signal sequences generation command implementation.
/// \param dev - device instance to be started.
/// \param pdata - pointer to the virtual device private data.
/// \param data - pointer to the start data, a PaceMakerStartCommand structure.
/// \param length - length of the input buffer.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_start(    volatile PaceMakerDevInstance* dev,
                            volatile PPaceMakerDevPrivData priv_data,
                            PaceMakerStartCommand* data,
                            uint16_t length);

/// \brief Stop signal sequences generation command implementation.
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_stop(volatile PaceMakerDevInstance* dev, volatile PPaceMakerDevPrivData pdata);

/// \brief Reset device.
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
/// \return status of the operation (COMM_STATUS_XXX).
/// \note May be called under any circumstances.
uint8_t pacemaker_reset(volatile PaceMakerDevInstance* dev, volatile PPaceMakerDevPrivData pdata);

/// \brief Set data command implementation.
/// \param pdata - pointer to the virtual device private data.
/// \param data - pointer to the incoming device data.
/// \param length - length of the data.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_set_data(volatile PPaceMakerDevPrivData pdata, PaceMakerDevData* data, uint16_t length);

/// \brief Stop signal generation (internal)
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
static inline void pacemaker_stop_generation(volatile PaceMakerDevInstance* dev, volatile PPaceMakerDevPrivData pdata, EKIT_ERROR err) {
    ASSERT_IRQ_DISABLED;
    assert_param(pdata->status->started); // Must be started

    // Disable all interrupts
    timer_disable_no_irq(dev->internal_timer, dev->internal_timer_irqn);
    timer_disable_ex(dev->internal_timer);

    timer_disable_no_irq(dev->main_timer, dev->main_timer_irqn);
    timer_disable_ex(dev->main_timer);

    // Clear status
    pdata->status->started = 0;
    pdata->status->internal_index = 0;
    pdata->status->main_counter = 0;
    pdata->status->last_error = err;

    // Switch to default pin state
    dev->pfn_set_gpio(dev->default_pin_state);
}

static inline void pacemaker_first_transition( volatile PPaceMakerDevInstance dev,
                                            volatile PPaceMakerDevPrivData pdata) {
    volatile PaceMakerTransition* trans;
    DISABLE_IRQ
    trans = pdata->transitions;
    dev->pfn_set_gpio(dev->default_pin_state);
    pdata->status->internal_index = 0;

    if (pdata->main_cycle_number > 0) {
        if (pdata->status->main_counter == 0) {
            // limited mode, stop signal generation and switch to default gpio
            pacemaker_stop_generation(dev, pdata, EKIT_OK);
            ENABLE_IRQ
            return;
        }
        pdata->status->main_counter--;
    }

    timer_start_ex(dev->internal_timer,
                   trans->prescaller,
                   trans->counter,
                   dev->internal_timer_irqn,
                   IRQ_PRIORITY_PACEMAKER_INTERNAL,
                   0);
    ENABLE_IRQ
}

static inline void pacemaker_next_transition( volatile PPaceMakerDevInstance dev,
                                               volatile PPaceMakerDevPrivData pdata) {
    volatile PaceMakerTransition* trans;
    DISABLE_IRQ
    trans = pdata->transitions + pdata->status->internal_index;
    dev->pfn_set_gpio(trans->signal_mask);
    pdata->status->internal_index++;

    // setup next transition
    if (pdata->status->internal_index >= pdata->trans_number) {
        timer_disable_no_irq(dev->internal_timer, dev->internal_timer_irqn);
        timer_disable_ex(dev->internal_timer);
    } else {
        trans++;
        timer_reschedule(dev->internal_timer, trans->prescaller, trans->counter);
    }
    ENABLE_IRQ
}

/// \brief Common MAIN TIMER IRQ handler
/// \param index - index of the virtual device
void PACEMAKER_MAIN_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);

    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);
    volatile PaceMakerDevInstance* device = g_pacemakerdev_devs+index;
    volatile PaceMakerDevPrivData* priv_data = (&device->privdata);

    // <CHECKIT> Do we actually need this check?
    if (TIM_GetITStatus(device->main_timer, TIM_IT_Update) == RESET) {
        return;
    }

    TIM_ClearITPendingBit(device->main_timer, TIM_IT_Update);

    if (priv_data->status->internal_index < priv_data->trans_number) {
        DISABLE_IRQ
        pacemaker_stop_generation(device, priv_data, EKIT_TOO_FAST);
        ENABLE_IRQ
    } else {
        pacemaker_first_transition(device, priv_data);
    }
}
PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS

/// \brief Common INTERNAL TIMER IRQ handler
/// \param index - index of the virtual device
void PACEMAKER_INTERNAL_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);
    volatile PaceMakerDevInstance* device = g_pacemakerdev_devs+index;
    volatile PaceMakerDevPrivData* priv_data = (&device->privdata);

    if (TIM_GetITStatus(device->internal_timer, TIM_IT_Update) == RESET) {
        return;
    }

    TIM_ClearITPendingBit(device->internal_timer, TIM_IT_Update);

    pacemaker_next_transition(device, priv_data);
}
PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS

void pacemakerdev_init_vdev(volatile PaceMakerDevInstance* dev, uint16_t index) {
    volatile PDeviceContext devctx = (volatile PDeviceContext)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->on_command   = pacemakerdev_execute;
    devctx->on_read_done = pacemakerdev_read_done;
    devctx->buffer       = dev->buffer;
    devctx->bytes_available = dev->buffer_size;

    DISABLE_IRQ;
    pacemaker_reset(dev, &(dev->privdata));
    ENABLE_IRQ;

    comm_register_device(devctx);
}

void pacemakerdev_init() {
    for (uint16_t i=0; i<PACEMAKERDEV_DEVICE_COUNT; i++) {
        volatile PaceMakerDevInstance* dev = (volatile PaceMakerDevInstance*)g_pacemakerdev_devs+i;
        pacemakerdev_init_vdev(dev, i);
    }
}

void pacemakerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(cmd_byte);
    volatile PaceMakerDevInstance* dev = (volatile PaceMakerDevInstance*)g_pacemakerdev_devs + devctx->dev_index;
    volatile PaceMakerDevPrivData* pdata = &(dev->privdata);
    uint8_t result = COMM_STATUS_FAIL;

    switch (cmd_byte & (~COMM_MAX_DEV_ADDR)) {
        case PACEMAKERDEV_START:
            DISABLE_IRQ;
            result = pacemaker_start(dev, pdata, (PaceMakerStartCommand*)data, length);
            ENABLE_IRQ;
        break;

        case PACEMAKERDEV_STOP:
            DISABLE_IRQ
            result = pacemaker_stop(dev, pdata);
            ENABLE_IRQ
        break;

        case PACEMAKERDEV_DATA:
            DISABLE_IRQ;
            result = pacemaker_set_data(pdata, (PaceMakerDevData*)data, length);
            ENABLE_IRQ;
        break;

        case PACEMAKERDEV_RESET:
            DISABLE_IRQ;
            result = pacemaker_reset(dev, pdata);
            ENABLE_IRQ;
        break;
    }

    comm_done(result);
}

void pacemakerdev_read_done(uint8_t device_id, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(device_id);
    volatile PaceMakerDevInstance* dev = g_pacemakerdev_devs + devctx->dev_index;

    UNUSED(dev);
    UNUSED(length);

    comm_done(COMM_STATUS_OK);
}

uint8_t pacemaker_reset(volatile PaceMakerDevInstance* dev, volatile PPaceMakerDevPrivData pdata) {
    uint8_t result = COMM_STATUS_FAIL;

    ASSERT_IRQ_DISABLED;    // Must execute with IRQ disabled.

    // Stop all timers
    timer_disable_no_irq(dev->main_timer, dev->main_timer_irqn);
    timer_disable_ex(dev->main_timer);
    timer_disable_no_irq(dev->internal_timer, dev->internal_timer_irqn);
    timer_disable_ex(dev->internal_timer);

    // Clean status and private data
    memset((void*)pdata, 0, sizeof(PaceMakerDevPrivData));
    pdata->status = (volatile PaceMakerStatus*)dev->buffer;
    pdata->transitions = (volatile PaceMakerTransition*)(dev->buffer + sizeof(PaceMakerStatus));
    memset((void*)pdata->status, 0, sizeof(PaceMakerStatus));

    uint32_t trans_buffer_size = dev->buffer_size - sizeof(PaceMakerStatus);
    if ((trans_buffer_size % sizeof(PaceMakerTransition)) != 0) {
        pdata->status->last_error = EKIT_UNALIGNED;
        goto done;
    }
    pdata->max_trans_number = trans_buffer_size / sizeof(PaceMakerTransition);
    memset((void*)pdata->transitions, 0, sizeof(PaceMakerTransition)*pdata->max_trans_number);

    // Configure and reset gpio to defaults.
    dev->pfn_init_gpio();
    result = COMM_STATUS_OK;
done:
    return result;
}

uint8_t pacemaker_start(    volatile PaceMakerDevInstance* dev,
                            volatile PPaceMakerDevPrivData priv_data,
                            PaceMakerStartCommand* data,
                            uint16_t length) {
    uint8_t result = COMM_STATUS_FAIL;
    volatile PPaceMakerStatus status = priv_data->status;
    ASSERT_IRQ_DISABLED;
    if (status->started) {
        status->last_error = EKIT_NOT_STOPPED;
        goto done; // Device must be stopped.
    }

    if (priv_data->trans_number == 0) {
        status->last_error = EKIT_NO_DATA;
        goto done; // Data must be passed first.
    }

    if (length != sizeof(PaceMakerStartCommand)) {
        status->last_error = EKIT_BAD_PARAM;
        goto done; // Bad buffer.
    }

    priv_data->main_cycle_number = data->main_cycles_number;
    priv_data->main_cycle_prescaller = data->main_prescaller;
    priv_data->main_cycle_counter = data->main_counter;

    status->started = 1;
    status->last_error = EKIT_OK;
    status->main_counter = priv_data->main_cycle_number;
    status->internal_index = priv_data->trans_number;
    result = COMM_STATUS_OK;

    // Start main timer
    timer_start_ex(dev->main_timer,
                   priv_data->main_cycle_prescaller,
                   priv_data->main_cycle_counter,
                   dev->main_timer_irqn,
                   IRQ_PRIORITY_PACEMAKER_MAIN,
                   1);
done:
    return result;
}

uint8_t pacemaker_stop(volatile PaceMakerDevInstance* dev, volatile PPaceMakerDevPrivData pdata) {
    uint8_t result = COMM_STATUS_FAIL;
    volatile PPaceMakerStatus status = pdata->status;
    ASSERT_IRQ_DISABLED;
    if (status->started == 0) {
        status->last_error = EKIT_NOT_STARTED;
        goto done; // Device must be stopped.
    }

    pacemaker_stop_generation(dev, pdata, EKIT_OK);
    result = COMM_STATUS_OK;

done:
    return result;
}

uint8_t pacemaker_set_data(volatile PPaceMakerDevPrivData pdata, PaceMakerDevData* data, uint16_t length) {
    uint8_t result = COMM_STATUS_FAIL;
    volatile PPaceMakerStatus status = pdata->status;

    ASSERT_IRQ_DISABLED;

    if (status->started) {
        status->last_error = EKIT_NOT_SUSPENDED;
        goto done; // Device must be stopped.
    }
    if (length < sizeof(PaceMakerDevData) + sizeof(PaceMakerTransition)) {
        status->last_error = EKIT_NO_DATA;
        goto done; // Minimal data length
    }
    uint32_t trans_data_len = length - sizeof(PaceMakerDevData);
    if ((trans_data_len % sizeof(PaceMakerTransition)) != 0) {
        status->last_error = EKIT_UNALIGNED;
        goto done; // Unaligned data
    }

    uint32_t trans_num = trans_data_len / sizeof(PaceMakerTransition);
    if (trans_num > pdata->max_trans_number) {
        status->last_error = EKIT_OVERFLOW;
        goto done;
    }

    // All is good, copy data
    pdata->trans_number = trans_num;
    pdata->status->internal_index = 0;
    memcpy((void*)pdata->transitions, (void*)data->transitions, trans_data_len);

    status->last_error = EKIT_OK;
    result = COMM_STATUS_OK;
done:
    return result;
}

#endif
