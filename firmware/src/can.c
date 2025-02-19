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
 *   \brief Can device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef CAN_DEVICE_ENABLED

#include <string.h>
#include <stm32f10x.h>
#include "utools.h"
#include "i2c_bus.h"
#include "can_conf.h"
#include "can.h"


/// \addtogroup group_can
/// @{

CAN_FW_BUFFERS

/// \brief Global array that stores all virtual Can devices configurations.
struct CanInstance g_can_devs[] = CAN_FW_DEV_DESCRIPTOR;

#define CAN_DISABLE_IRQs                                                    \
{                                                                           \
    uint32_t irqn_tx_state = NVIC_IRQ_STATE(dev->irqn_tx);                  \
    uint32_t irqn_rx0_state = NVIC_IRQ_STATE(dev->irqn_rx0);                \
    uint32_t irqn_rx1_state = NVIC_IRQ_STATE(dev->irqn_rx1);                \
    uint32_t irqn_sce_state = NVIC_IRQ_STATE(dev->irqn_sce);                \
    NVIC_DISABLE_IRQ(dev->irqn_tx,  irqn_tx_state);                         \
    NVIC_DISABLE_IRQ(dev->irqn_rx0, irqn_rx0_state);                        \
    NVIC_DISABLE_IRQ(dev->irqn_rx1, irqn_rx1_state);                        \
    NVIC_DISABLE_IRQ(dev->irqn_sce, irqn_sce_state);

#define CAN_RESTORE_IRQs                                                    \
    NVIC_RESTORE_IRQ(dev->irqn_sce, irqn_sce_state);                        \
    NVIC_RESTORE_IRQ(dev->irqn_rx1, irqn_rx1_state);                        \
    NVIC_RESTORE_IRQ(dev->irqn_rx0, irqn_rx0_state);                        \
    NVIC_RESTORE_IRQ(dev->irqn_tx,  irqn_tx_state);                         \
}

//---------------------------- FORWARD DECLARATIONS ----------------------------
/// \brief Starts CAN device (switches CAN to running mode).
/// \param devctx - device context structure represented by #DeviceContext
/// \param dev - device instance structure represented by #CanInstance
/// \param reset - non-zero to reset accumulated data.
/// \return non-zero if recovering after bus-off state
/// \note state state is tracked by #can_execute()
uint8_t can_start(struct DeviceContext* devctx, struct CanInstance* dev, uint8_t recovery);

/// \brief Stops CAN device (switches CAN to stop mode).
/// \param devctx - device context structure represented by #DeviceContext
/// \param dev - device instance structure represented by #CanInstance
/// \return non-zero in the case of success, otherwise 0
/// \note state state is tracked by #can_execute()
uint8_t can_stop(struct DeviceContext* devctx, struct CanInstance* dev);

/// \brief Applies a filter for CAN device.
/// \param devctx - device context structure represented by #DeviceContext
/// \param dev - device instance structure represented by #CanInstance
/// \param filter - filter represented by #CanFilterCommand
/// \return non-zero in the case of success, otherwise 0
/// \note state state is tracked by #can_execute(); Returns error (0) if device is in started state.
uint8_t can_filter(struct DeviceContext* devctx, struct CanInstance* dev, struct CanFilterCommand* filter);


/// \brief Sends a message to CAN bus.
/// \param devctx - device context structure represented by #DeviceContext
/// \param dev - device instance structure represented by #CanInstance
/// \param message - pointer to a message represented by variable length #CanSendCommand structure.
/// \param length - length of the message structure passed by software.
/// \return non-zero in the case of success, otherwise 0
/// \note state state is tracked by #can_execute(); Returns error (0) if device is in stopped state.
uint8_t can_send(struct DeviceContext* devctx, struct CanInstance* dev, struct CanSendCommand* message, uint8_t length);

/// \brief Put received CAN message to the internal circular buffer
/// \param dev - device instance structure represented by #CanInstance
/// \param circ_buffer - internal circular buffer represented by a pointer to #CircBuffer structure.
/// \param message - message represented by a pointer to #CanRxMsg structure.
/// \param status - status represented by a pointer to #CanStatus structure.
void can_put_message_on_buffer( struct CanInstance* dev, 
                                struct CircBuffer* circ_buffer, 
                                CanRxMsg* message,
                                struct CanStatus* status);

/// \brief Reset state of the CAN virtual device
/// \param dev - device instance structure represented by #CanInstance
void can_reset_status(struct CanInstance* dev);
/// @}

//---------------------------- INTERRUPTS ----------------------------

void CAN_COMMON_TX_IRQ_HANDLER(uint16_t index) {
    struct CanInstance* dev = (struct CanInstance*)g_can_devs+index;
    const uint16_t mask = CAN_STATE_MB_0_BUSY | CAN_STATE_MB_1_BUSY | CAN_STATE_MB_2_BUSY;

    // Transmit mailbox empty Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_TME)==SET) {
        uint16_t mb_empty = ((uint16_t)((dev->can->TSR >> (26 - CAN_STATE_MB_0_BUSY_BIT_OFFSET)))) & mask;
        SET_BIT_FIELD(dev->privdata.status.state, mask, (uint16_t)(~mb_empty));
        CAN_ClearITPendingBit(dev->can, CAN_IT_TME);
    }
}

void CAN_COMMON_RX0_IRQ_HANDLER(uint16_t index) {
    struct CanInstance* dev = (struct CanInstance*)g_can_devs+index;

    CanRxMsg message;

    // FIFO 0 message pending Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FMP0) == SET) {
        memset(&message, 0, sizeof(CanRxMsg));
        CAN_Receive(dev->can, CAN_FIFO0, &message);
        can_put_message_on_buffer(dev,
                                  (struct CircBuffer*)&(dev->circ_buffer),
                                  &message,
                                  (struct CanStatus*) &(dev->privdata.status));
    }

    // FIFO 0 full Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FF0)==SET) {
        CAN_ClearFlag(dev->can, CAN_FLAG_FF0);
        SET_BIT(dev->privdata.status.state, CAN_ERROR_FIFO_0_FULL);
        CAN_ClearITPendingBit(dev->can, CAN_IT_FF0);
    }

    // FIFO 0 overrun Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FOV0)==SET) {
        CAN_ClearFlag(dev->can, CAN_FLAG_FOV0);
        SET_BIT(dev->privdata.status.state, CAN_ERROR_FIFO_0_OVERFLOW);
        CAN_ClearITPendingBit(dev->can, CAN_IT_FOV0);
    }
}

void CAN_COMMON_RX1_IRQ_HANDLER(uint16_t index) {
    struct CanInstance* dev = (struct CanInstance*)g_can_devs+index;

    CanRxMsg message;

    // FIFO 1 message pending Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FMP1) == SET) {
        memset(&message, 0, sizeof(CanRxMsg));
        CAN_Receive(dev->can, CAN_FIFO1, &message);
        can_put_message_on_buffer(dev,
                                  (struct CircBuffer*)&(dev->circ_buffer),
                                  &message,
                                  (struct CanStatus*) &(dev->privdata.status));
    }

    // FIFO 1 full Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FF1)==SET) {
        CAN_ClearFlag(dev->can, CAN_FLAG_FF1);
        SET_BIT(dev->privdata.status.state, CAN_ERROR_FIFO_1_FULL);
        CAN_ClearITPendingBit(dev->can, CAN_IT_FF1);
    }

    // FIFO 1 overrun Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_FOV1)==SET) {
        CAN_ClearFlag(dev->can, CAN_FLAG_FF1);
        SET_BIT(dev->privdata.status.state, CAN_ERROR_FIFO_1_OVERFLOW);
        CAN_ClearITPendingBit(dev->can, CAN_IT_FOV1);
    }
}

void CAN_COMMON_SCE_IRQ_HANDLER(uint16_t index) {
    struct CanInstance* dev = (struct CanInstance*)g_can_devs+index;
    struct CanStatus* pstatus = (struct CanStatus*)&(dev->privdata.status);

    pstatus->recv_error_count = CAN_GetReceiveErrorCounter(dev->can);
    pstatus->lsb_trans_count = CAN_GetLSBTransmitErrorCounter(dev->can);

    // Error Interrupt
    if (CAN_GetITStatus(dev->can, CAN_IT_ERR)==SET) {
        // Error warning Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_EWG)==SET) {
            SET_BIT(pstatus->state, CAN_ERROR_WARNING);
            CAN_ITConfig(dev->can, CAN_IT_EWG, DISABLE);
            CAN_ClearITPendingBit(dev->can, CAN_IT_EWG);
        }

        // Error passive Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_EPV)==SET) {
            SET_BIT(pstatus->state, CAN_ERROR_PASSIVE);
            CAN_ITConfig(dev->can, CAN_IT_EPV, DISABLE);
            CAN_ClearITPendingBit(dev->can, CAN_IT_EPV);
        }

        // Bus-off Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_BOF)==SET) {
            SET_BIT(pstatus->state, CAN_ERROR_BUS_OFF);
            CAN_ITConfig(dev->can, CAN_IT_BOF, DISABLE);
            CAN_ITConfig(dev->can, CAN_IT_LEC, DISABLE);
            CAN_ITConfig(dev->can, CAN_IT_ERR, DISABLE);
            CAN_ClearITPendingBit(dev->can, CAN_IT_BOF);
        }

        // Last error code Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_LEC)==SET) {
            pstatus->last_error = CAN_GetLastErrorCode(dev->can);
            CAN_ClearFlag(dev->can, CAN_FLAG_LEC);
            CAN_ITConfig(dev->can, CAN_IT_LEC, DISABLE);
            CAN_ClearITPendingBit(dev->can, CAN_IT_LEC);
        }

        CAN_ClearITPendingBit(dev->can, CAN_IT_ERR);
    } else {
        // Wake-up Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_WKU)==SET) {
            CAN_ClearFlag(dev->can, CAN_FLAG_WKU);
            CLEAR_BIT(pstatus->state, CAN_STATE_SLEEP);
            CAN_ClearITPendingBit(dev->can, CAN_IT_WKU);
        }

        // Sleep acknowledge Interrupt
        if (CAN_GetITStatus(dev->can, CAN_IT_SLK)==SET)	{
            CAN_ClearFlag(dev->can, CAN_FLAG_SLAK);
            SET_BIT(pstatus->state, CAN_STATE_SLEEP);
            CAN_ClearITPendingBit(dev->can, CAN_IT_SLK);
        }
    }
}

CAN_FW_IRQ_HANDLERS

void can_init_vdev(struct CanInstance* dev, uint16_t index) {
    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(struct DeviceContext));
    devctx->device_id      = dev->dev_id;
    devctx->dev_index      = index;
    devctx->on_command     = can_execute;
    devctx->on_read_done   = can_read_done;
    devctx->on_polling     = can_polling;
    devctx->on_sync        = can_sync;
    devctx->polling_period = CAN_POLLING_EVERY_US;

    // Init circular buffer
    struct CircBuffer* circbuf = (struct CircBuffer*) &(dev->circ_buffer);
    circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
    circbuf_init_block_mode(circbuf, sizeof(struct CanRecvMessage));
    circbuf_init_status(circbuf, (uint8_t*)&(dev->privdata.comm_status), sizeof(struct CanStatus));
    devctx->circ_buffer = circbuf;

    // Initialize GPIO and remap if required
    START_PIN_DECLARATION
    DECLARE_PIN(dev->canrx_port, 1 << dev->canrx_pin, GPIO_Mode_IPU);
    DECLARE_PIN(dev->cantx_port, 1 << dev->cantx_pin, GPIO_Mode_AF_PP);

    if (dev->can_remap) {
        GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
    }

    // Initialize state
    can_reset_status(dev);

    // Set can filter to default state.
    for (uint8_t i=0; i<CAN_MAX_FILTER_COUNT; i++) {
        CAN_FilterInitTypeDef* filter = (CAN_FilterInitTypeDef*)dev->privdata.can_filters + i;

        filter->CAN_FilterNumber = i;
        filter->CAN_FilterMode = CAN_FilterMode_IdMask;
        filter->CAN_FilterScale = CAN_FilterScale_32bit;
        filter->CAN_FilterFIFOAssignment = CAN_FIFO0;
        filter->CAN_FilterActivation = DISABLE;

        filter->CAN_FilterIdHigh = 0;
        filter->CAN_FilterIdLow = 0;
        filter->CAN_FilterMaskIdHigh = 0;
        filter->CAN_FilterMaskIdLow = 0;
    }

    comm_register_device(devctx);
}

void can_init() {
    for (uint16_t i=0; i<CAN_DEVICE_COUNT; i++) {
        struct CanInstance* dev = (struct CanInstance*)g_can_devs+i;
        can_init_vdev(dev, i);
    }
}

void can_reset_status(struct CanInstance* dev) {
    struct CircBuffer* circbuf = (struct CircBuffer*) &(dev->circ_buffer);
    dev->privdata.status.data_len = circbuf_total_len(circbuf);
    dev->privdata.status.state = 0;
    dev->privdata.status.lsb_trans_count = 0;
    dev->privdata.status.recv_error_count = 0;
    dev->privdata.status.last_error = 0;
}

uint8_t can_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct CanInstance* dev = (struct CanInstance*)g_can_devs + devctx->dev_index;
    struct CanPrivData* priv = &(dev->privdata);

    // Check if there is no error
    uint8_t cmd = cmd_byte & COMM_CMDBYTE_DEV_SPECIFIC_MASK;
    uint8_t no_error = IS_SINGLE_BIT(cmd);
    if (no_error==0) goto done;

    switch (cmd) {
        case CAN_START:
            no_error = (length==0) && IS_CLEARED(priv->status.state, CAN_STATE_STARTED);
            break;
        case CAN_STOP:
            no_error = (length==0) && IS_SET(priv->status.state, CAN_STATE_STARTED);
            break;

        case CAN_FILTER:
            no_error = (length>=sizeof(struct CanFilterCommand)) && IS_CLEARED(priv->status.state, CAN_STATE_STARTED);
            break;

        case CAN_SEND:
            no_error = (length>=sizeof(struct CanSendCommand)) && IS_SET(priv->status.state, CAN_STATE_STARTED);
            break;
        default:
            no_error = 0;
    }
    if (no_error==0) goto done;

    // Allowed processing actual command
    switch (cmd) {
        case CAN_START:
            no_error = can_start(devctx, dev, 0);
        break;

        case CAN_STOP:
            no_error = can_stop(devctx, dev);
        break;

        case CAN_FILTER:
            no_error = can_filter(devctx, dev, (struct CanFilterCommand*)data);
        break;

        case CAN_SEND:
            no_error = can_send(devctx, dev, (struct CanSendCommand*)data, length);
        break;
    }

done:

    return no_error!=0 ? 0 : COMM_STATUS_FAIL;
}

void can_polling(uint8_t device_id) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct CanInstance* dev = g_can_devs + devctx->dev_index;

    if (IS_SET(dev->privdata.status.state, CAN_STATE_STARTED | CAN_ERROR_BUS_OFF)) {
        can_start(devctx, dev, 1);
    }
}

uint8_t can_read_done(uint8_t device_id, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct CanInstance* dev = g_can_devs + devctx->dev_index;

    volatile struct CircBuffer* circbuf = (volatile struct CircBuffer*)&(dev->circ_buffer);

    circbuf_stop_read(circbuf, length);
    circbuf_clear_ovf(circbuf);

    dev->privdata.status.data_len = circbuf_total_len(circbuf);

    return COMM_STATUS_OK;
}

uint8_t can_start(struct DeviceContext* devctx, struct CanInstance* dev, uint8_t recovery) {
    assert_param(recovery || IS_CLEARED(dev->privdata.status.state, CAN_STATE_STARTED));

    UNUSED(devctx);

    // Initialize CAN
    CAN_DeInit(dev->can);
    CAN_InitTypeDef can_init;

    can_init.CAN_Prescaler = dev->can_prescaller;
    can_init.CAN_BS1       = dev->can_bs1;
    can_init.CAN_SJW       = dev->can_sample_point;
    can_init.CAN_BS2       = dev->can_bs2;

    can_init.CAN_Mode = CAN_Mode_Normal; // CAN_Mode_Silent_LoopBack;
    can_init.CAN_TTCM = DISABLE;
    can_init.CAN_ABOM = DISABLE;
    can_init.CAN_AWUM = ENABLE;
    can_init.CAN_NART = DISABLE;
    can_init.CAN_RFLM = DISABLE;
    can_init.CAN_TXFP = ENABLE;
    CAN_Init(dev->can, &can_init);

    // Reset data if required
    if (recovery==0) {
        circbuf_reset(devctx->circ_buffer);
    }

    can_reset_status(dev);

    // CAN Initialize filters. By default, filters are disabled, message reception is not possible.
    for (uint8_t i=0; i<CAN_MAX_FILTER_COUNT; i++) {
        CAN_FilterInitTypeDef* filter = (CAN_FilterInitTypeDef*)dev->privdata.can_filters + i;
        CAN_FilterInit(filter);
    }

    // Initialize interrupts
    CAN_ITConfig(dev->can, CAN_IT_TME, ENABLE);         // Transmit mailbox empty Interrupt

    CAN_ITConfig(dev->can, CAN_IT_FMP0, ENABLE);        // FIFO 0 message pending Interrupt
    CAN_ITConfig(dev->can, CAN_IT_FF0, ENABLE);         // FIFO 0 full Interrupt
    CAN_ITConfig(dev->can, CAN_IT_FOV0, ENABLE);        // FIFO 0 overrun Interrupt

    CAN_ITConfig(dev->can, CAN_IT_FMP1, ENABLE);        // FIFO 1 message pending Interrupt
    CAN_ITConfig(dev->can, CAN_IT_FF1, ENABLE);         // FIFO 1 full Interrupt
    CAN_ITConfig(dev->can, CAN_IT_FOV1, ENABLE);        // FIFO 1 overrun Interrupt

    CAN_ITConfig(dev->can, CAN_IT_WKU, ENABLE);         // Wake-up Interrupt
    CAN_ITConfig(dev->can, CAN_IT_SLK, ENABLE);         // Sleep acknowledge Interrupt

    CAN_ITConfig(dev->can, CAN_IT_EWG, ENABLE);         // Error warning Interrupt
    CAN_ITConfig(dev->can, CAN_IT_EPV, ENABLE);         // Error passive Interrupt
    CAN_ITConfig(dev->can, CAN_IT_BOF, ENABLE);         // Bus-off Interrupt
    CAN_ITConfig(dev->can, CAN_IT_LEC, ENABLE);         // Last error code Interrupt
    CAN_ITConfig(dev->can, CAN_IT_ERR, ENABLE);         // Error Interrupt

    NVIC_SetPriority(dev->irqn_tx, IRQ_PRIORITY_CAN);
    NVIC_EnableIRQ(dev->irqn_tx);

    NVIC_SetPriority(dev->irqn_rx0, IRQ_PRIORITY_CAN);
    NVIC_EnableIRQ(dev->irqn_rx0);

    NVIC_SetPriority(dev->irqn_rx1, IRQ_PRIORITY_CAN);
    NVIC_EnableIRQ(dev->irqn_rx1);

    NVIC_SetPriority(dev->irqn_sce, IRQ_PRIORITY_CAN);
    NVIC_EnableIRQ(dev->irqn_sce);

    SET_BIT(dev->privdata.status.state, CAN_STATE_STARTED);
    return 1;
}

uint8_t can_stop(struct DeviceContext* devctx, struct CanInstance* dev) {
    UNUSED(devctx);
    assert_param(IS_SET(dev->privdata.status.state, CAN_STATE_STARTED));
    CAN_DeInit(dev->can);
    CLEAR_BIT(dev->privdata.status.state, CAN_STATE_STARTED);
    return 1;
}

uint8_t can_filter( struct DeviceContext* devctx,
                    struct CanInstance* dev,
                    struct CanFilterCommand* flt) {
    UNUSED(devctx);
    assert_param(IS_CLEARED(dev->privdata.status.state, CAN_STATE_STARTED));

    uint8_t result = 0;
    uint8_t index = flt->flags & CAN_FLT_INDEX_MASK;
    CAN_FilterInitTypeDef* filter = (CAN_FilterInitTypeDef*)dev->privdata.can_filters + index;
    if (index>CAN_MAX_FILTER_COUNT) {
        assert_param(0);
        goto done;
    }

    filter->CAN_FilterNumber         = index;
    filter->CAN_FilterMode           = (flt->flags & CAN_FLT_LIST_MODE) ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask;
    filter->CAN_FilterScale          = (flt->flags & CAN_FLT_SCALE) ? CAN_FilterScale_32bit : CAN_FilterScale_16bit;
    filter->CAN_FilterFIFOAssignment = (flt->flags & CAN_FLT_FIFO) ? CAN_FIFO1 : CAN_FIFO0;
    filter->CAN_FilterActivation     = (flt->flags & CAN_FLT_ENABLE) ? ENABLE : DISABLE;

    filter->CAN_FilterIdHigh         = flt->id_msb;
    filter->CAN_FilterIdLow          = flt->id_lsb;
    filter->CAN_FilterMaskIdHigh     = flt->mask_msb;
    filter->CAN_FilterMaskIdLow      = flt->mask_lsb;

    CAN_FilterInit(filter);
    result = 1;
done:
    return result;
}

uint8_t can_send(   struct DeviceContext* devctx,
                    struct CanInstance* dev,
                    struct CanSendCommand* msg,
                    uint8_t length) {
    assert_param(IS_SET(dev->privdata.status.state, CAN_STATE_STARTED));
    uint8_t result = 0;
    UNUSED(devctx);

    CanTxMsg message;

    // Check message length
    uint8_t len = (msg->extra & CAN_MSG_MAX_DATA_LEN_MASK);
    if ( (len>CAN_MSG_MAX_DATA_LEN) || (length!=(sizeof(struct CanSendCommand)+len))) goto done;

    // Fill message structure
    message.StdId = msg->id;
    message.ExtId = msg->ext_id;
    message.IDE = (msg->extra & CAN_MSG_EXTENDED_ID) ? CAN_Id_Extended : CAN_Id_Standard;
    message.RTR = (msg->extra & CAN_MSG_REMOTE_FRAME) ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    message.DLC = len;
    memcpy(message.Data, (const uint8_t*)msg->data, len);
    memset(message.Data+len, 0, CAN_MSG_MAX_DATA_LEN - len);

    // Check result
    uint8_t mb = CAN_Transmit(dev->can, &message);
    if (mb!=CAN_TxStatus_NoMailBox) {
        CAN_DISABLE_IRQs
        SET_BIT(dev->privdata.status.state, 1 << (mb + CAN_STATE_MB_0_BUSY_BIT_OFFSET));
        CAN_RESTORE_IRQs
        result = 1;
    } else {
        CAN_DISABLE_IRQs
        SET_BIT(dev->privdata.status.state, CAN_ERROR_NO_MAILBOX);
        CAN_RESTORE_IRQs
    }

done:
    return result;
}

void can_put_message_on_buffer( struct CanInstance* dev,
                                struct CircBuffer* circ_buffer,
                                CanRxMsg* message,
                                struct CanStatus* status) {
    struct CanRecvMessage* recv_msg = (struct CanRecvMessage*)circbuf_reserve_block(circ_buffer);
    if (recv_msg==0) {
        // Failed to reserve
        SET_BIT(status->state, CAN_ERROR_OVERFLOW);
        can_stop((struct DeviceContext*)&(dev->dev_ctx), dev);
        return;
    }

    // Using CAN_Receive() and further transformation to CanRecvMessage is not optimal solution! It is possible to further optimize it
    // by using peripheral registers directly.
    recv_msg->fmi = message->FMI;
    if (message->IDE == CAN_Id_Extended) {
        recv_msg->id = message->ExtId;
        recv_msg->extra = CAN_MSG_EXTENDED_ID;
    } else {
        recv_msg->id = message->StdId;
        recv_msg->extra = 0;
    }

    recv_msg->extra = (message->IDE == CAN_Id_Extended) ? CAN_MSG_EXTENDED_ID : 0;
    recv_msg->extra |= (message->RTR == CAN_RTR_Remote) ? CAN_MSG_REMOTE_FRAME : 0;
    recv_msg->extra += message->DLC;
    memcpy(recv_msg->data, message->Data, CAN_MSG_MAX_DATA_LEN);

    circbuf_commit_block(circ_buffer);
    status->data_len = circbuf_total_len(circ_buffer);
}

uint8_t can_sync(uint8_t cmd_byte, uint16_t length) {
    UNUSED(length);
    struct DeviceContext* dev_ctx = comm_dev_context(cmd_byte);
    struct CanInstance* dev = (struct CanInstance*)(g_can_devs + dev_ctx->dev_index);
    struct CanStatus* status = &dev->privdata.status;
    struct CanStatus* comm_status = &dev->privdata.comm_status;

    CAN_DISABLE_IRQs
    /// It is safe to copy status information because device have COMM_STATUS_BUSY status at the moment. All status
    /// reads should fail because of this reason.
    memcpy(comm_status, status, sizeof(struct CanStatus));
    CAN_RESTORE_IRQs

    return COMM_STATUS_OK;
}

#endif
