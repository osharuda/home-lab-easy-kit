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
#include "pacemakerdev_conf.h"
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
struct PaceMakerDevInstance g_pacemakerdev_devs[] = PACEMAKERDEV_FW_DEV_DESCRIPTOR;

/// @}

#define PACEMAKER_DISABLE_IRQs                                              \
    uint32_t int_timer_state = NVIC_IRQ_STATE(dev->internal_timer_irqn);    \
    uint32_t main_timer_state = NVIC_IRQ_STATE(dev->main_timer_irqn);       \
    NVIC_DISABLE_IRQ(dev->internal_timer_irqn, int_timer_state);            \
    NVIC_DISABLE_IRQ(dev->main_timer_irqn, main_timer_state);

#define PACEMAKER_RESTORE_IRQs                                              \
    NVIC_RESTORE_IRQ(dev->main_timer_irqn, main_timer_state);               \
    NVIC_RESTORE_IRQ(dev->internal_timer_irqn, int_timer_state);


//---------------------------- FORWARD DECLARATIONS ----------------------------
/// \brief Start signal sequences generation command implementation.
/// \param dev - device instance to be started.
/// \param pdata - pointer to the virtual device private data.
/// \param data - pointer to the start data, a PaceMakerStartCommand structure.
/// \param length - length of the input buffer.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_start(    struct PaceMakerDevInstance* dev,
                            struct PaceMakerDevPrivData* priv_data,
                            struct PaceMakerStartCommand* data,
                            uint16_t length);

/// \brief Stop signal sequences generation command implementation.
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_stop(struct PaceMakerDevInstance* dev, struct PaceMakerDevPrivData* pdata);

/// \brief Reset device.
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
/// \return status of the operation (COMM_STATUS_XXX).
/// \note May be called under any circumstances.
uint8_t pacemaker_reset(struct PaceMakerDevInstance* dev, struct PaceMakerDevPrivData* pdata);

/// \brief Set data command implementation.
/// \param pdata - pointer to the virtual device private data.
/// \param data - pointer to the incoming device data.
/// \param length - length of the data.
/// \return status of the operation (COMM_STATUS_XXX).
uint8_t pacemaker_set_data(struct PaceMakerDevPrivData* pdata, struct PaceMakerDevData* data, uint16_t length);

/// \brief Stop signal generation (internal)
/// \param dev - device instance to be stopped.
/// \param pdata - pointer to the virtual device private data.
static inline void pacemaker_stop_generation(struct PaceMakerDevInstance* dev, struct PaceMakerDevPrivData* pdata, EKIT_ERROR err) {
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

static inline void pacemaker_first_transition( struct PaceMakerDevInstance* dev,
                                               struct PaceMakerDevPrivData* pdata) {
    struct PaceMakerTransition* trans;
    PACEMAKER_DISABLE_IRQs
    trans = pdata->transitions;
    dev->pfn_set_gpio(dev->default_pin_state);
    pdata->status->internal_index = 0;

    if (pdata->main_cycle_number > 0) {
        if (pdata->status->main_counter == 0) {
            // limited mode, stop signal generation and switch to default gpio
            pacemaker_stop_generation(dev, pdata, EKIT_OK);
            PACEMAKER_RESTORE_IRQs
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
    PACEMAKER_RESTORE_IRQs
}

static inline void pacemaker_next_transition( struct PaceMakerDevInstance* dev,
                                              struct PaceMakerDevPrivData* pdata) {
    struct PaceMakerTransition* trans;
    PACEMAKER_DISABLE_IRQs
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
    PACEMAKER_RESTORE_IRQs
}

/// \brief Common MAIN TIMER IRQ handler
/// \param index - index of the virtual device
void PACEMAKER_MAIN_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);

    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);
    struct PaceMakerDevInstance* dev = g_pacemakerdev_devs + index;
    struct PaceMakerDevPrivData* priv_data = (&dev->privdata);

    // <CHECKIT> Do we actually need this check?
    if (TIM_GetITStatus(dev->main_timer, TIM_IT_Update) == RESET) {
        return;
    }

    TIM_ClearITPendingBit(dev->main_timer, TIM_IT_Update);

    if (priv_data->status->internal_index < priv_data->trans_number) {
        PACEMAKER_DISABLE_IRQs
        pacemaker_stop_generation(dev, priv_data, EKIT_TOO_FAST);
        PACEMAKER_RESTORE_IRQs
    } else {
        pacemaker_first_transition(dev, priv_data);
    }
}
PACEMAKERDEV_FW_MAIN_TIMER_IRQ_HANDLERS

/// \brief Common INTERNAL TIMER IRQ handler
/// \param index - index of the virtual device
void PACEMAKER_INTERNAL_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<PACEMAKERDEV_DEVICE_COUNT);
    struct PaceMakerDevInstance* device = g_pacemakerdev_devs+index;
    struct PaceMakerDevPrivData* priv_data = (&device->privdata);

    if (TIM_GetITStatus(device->internal_timer, TIM_IT_Update) == RESET) {
        return;
    }

    TIM_ClearITPendingBit(device->internal_timer, TIM_IT_Update);

    pacemaker_next_transition(device, priv_data);
}
PACEMAKERDEV_FW_INTERNAL_TIMER_IRQ_HANDLERS

void pacemakerdev_init_vdev(struct PaceMakerDevInstance* dev, uint16_t index) {
    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(struct DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->on_command   = pacemakerdev_execute;
    devctx->on_read_done = pacemakerdev_read_done;
    devctx->buffer       = dev->buffer;
    devctx->bytes_available = dev->buffer_size;

    pacemaker_reset(dev, &(dev->privdata));
    comm_register_device(devctx);
}

void pacemakerdev_init() {
    for (uint16_t i=0; i<PACEMAKERDEV_DEVICE_COUNT; i++) {
        struct PaceMakerDevInstance* dev = (struct PaceMakerDevInstance*)g_pacemakerdev_devs+i;
        pacemakerdev_init_vdev(dev, i);
    }
}

uint8_t pacemakerdev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct PaceMakerDevInstance* dev = (struct PaceMakerDevInstance*)g_pacemakerdev_devs + devctx->dev_index;
    struct PaceMakerDevPrivData* pdata = &(dev->privdata);
    uint8_t result = COMM_STATUS_FAIL;

    switch (cmd_byte & (~COMM_MAX_DEV_ADDR)) {
        case PACEMAKERDEV_START:
        {
            PACEMAKER_DISABLE_IRQs;
            result = pacemaker_start(dev, pdata, (struct PaceMakerStartCommand*)data, length);
            PACEMAKER_RESTORE_IRQs;
        }
        break;

        case PACEMAKERDEV_STOP:
        {
            PACEMAKER_DISABLE_IRQs
            result = pacemaker_stop(dev, pdata);
            PACEMAKER_RESTORE_IRQs
        }
        break;

        case PACEMAKERDEV_DATA:
        {
            PACEMAKER_DISABLE_IRQs;
            result = pacemaker_set_data(pdata, (struct PaceMakerDevData*)data, length);
            PACEMAKER_RESTORE_IRQs;
        }
        break;

        case PACEMAKERDEV_RESET:
        {
            PACEMAKER_DISABLE_IRQs;
            result = pacemaker_reset(dev, pdata);
            PACEMAKER_RESTORE_IRQs;
        }
        break;
    }

    return result;
}

uint8_t pacemakerdev_read_done(uint8_t device_id, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct PaceMakerDevInstance* dev = g_pacemakerdev_devs + devctx->dev_index;

    UNUSED(dev);
    UNUSED(length);

    return COMM_STATUS_OK;
}

uint8_t pacemaker_reset(struct PaceMakerDevInstance* dev, struct PaceMakerDevPrivData* pdata) {
    uint8_t result = COMM_STATUS_FAIL;

    // Stop all timers
    timer_disable_no_irq(dev->main_timer, dev->main_timer_irqn);
    timer_disable_ex(dev->main_timer);
    timer_disable_no_irq(dev->internal_timer, dev->internal_timer_irqn);
    timer_disable_ex(dev->internal_timer);

    // Clean status and private data
    memset((void*)pdata, 0, sizeof(struct PaceMakerDevPrivData));
    pdata->status = (struct PaceMakerStatus*)dev->buffer;
    pdata->transitions = (struct PaceMakerTransition*)(dev->buffer + sizeof(struct PaceMakerStatus));
    memset((void*)pdata->status, 0, sizeof(struct PaceMakerStatus));

    uint32_t trans_buffer_size = dev->buffer_size - sizeof(struct PaceMakerStatus);
    if ((trans_buffer_size % sizeof(struct PaceMakerTransition)) != 0) {
        pdata->status->last_error = EKIT_UNALIGNED;
        goto done;
    }
    pdata->max_trans_number = trans_buffer_size / sizeof(struct PaceMakerTransition);
    memset((void*)pdata->transitions, 0, sizeof(struct PaceMakerTransition)*pdata->max_trans_number);

    // Configure and reset gpio to defaults.
    dev->pfn_init_gpio();
    result = COMM_STATUS_OK;
done:
    return result;
}

uint8_t pacemaker_start(    struct PaceMakerDevInstance* dev,
                            struct PaceMakerDevPrivData* priv_data,
                            struct PaceMakerStartCommand* data,
                            uint16_t length) {
    uint8_t result = COMM_STATUS_FAIL;
    struct PaceMakerStatus* status = priv_data->status;

    if (status->started) {
        status->last_error = EKIT_NOT_STOPPED;
        goto done; // Device must be stopped.
    }

    if (priv_data->trans_number == 0) {
        status->last_error = EKIT_NO_DATA;
        goto done; // Data must be passed first.
    }

    if (length != sizeof(struct PaceMakerStartCommand)) {
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

uint8_t pacemaker_stop(struct PaceMakerDevInstance* dev, struct PaceMakerDevPrivData* pdata) {
    uint8_t result = COMM_STATUS_FAIL;
    struct PaceMakerStatus* status = pdata->status;

    if (status->started == 0) {
        status->last_error = EKIT_NOT_STARTED;
        goto done; // Device must be stopped.
    }

    pacemaker_stop_generation(dev, pdata, EKIT_OK);
    result = COMM_STATUS_OK;

done:
    return result;
}

uint8_t pacemaker_set_data(struct PaceMakerDevPrivData* pdata, struct PaceMakerDevData* data, uint16_t length) {
    uint8_t result = COMM_STATUS_FAIL;
    struct PaceMakerStatus* status = pdata->status;

    if (status->started) {
        status->last_error = EKIT_NOT_SUSPENDED;
        goto done; // Device must be stopped.
    }
    if (length < sizeof(struct PaceMakerDevData) + sizeof(struct PaceMakerTransition)) {
        status->last_error = EKIT_NO_DATA;
        goto done; // Minimal data length
    }
    uint32_t trans_data_len = length - sizeof(struct PaceMakerDevData);
    if ((trans_data_len % sizeof(struct PaceMakerTransition)) != 0) {
        status->last_error = EKIT_UNALIGNED;
        goto done; // Unaligned data
    }

    uint32_t trans_num = trans_data_len / sizeof(struct PaceMakerTransition);
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
