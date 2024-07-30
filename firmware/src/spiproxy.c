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
 *   \brief SPIProxy device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef SPIPROXY_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "spiproxy.h"
#include "spiproxy_conf.h"
#include <stm32f10x.h>


/// \addtogroup group_spiproxy
/// @{

SPIPROXY_FW_IN_BUFFERS
SPIPROXY_FW_OUT_BUFFERS
SPI_FW_DMA_TX_PREINIT
SPI_FW_DMA_RX_PREINIT

/// \brief Global array that stores all virtual SPIProxy devices configurations (instances).
struct SPIProxyInstance g_spiproxy_devs[] = SPIPROXY_FW_DEV_DESCRIPTOR;

/// @}

#define SPI_PROXY_DISABLE_IRQ \
{                                                                               \
    uint32_t tx_dma_complete_state = NVIC_IRQ_STATE(dev->tx_dma_complete_irqn); \
    uint32_t rx_dma_complete_state = NVIC_IRQ_STATE(dev->rx_dma_complete_irqn); \
    uint32_t spi_state = NVIC_IRQ_STATE(dev->spi_interrupt_irqn);               \
    NVIC_DISABLE_IRQ(dev->tx_dma_complete_irqn, tx_dma_complete_state);         \
    NVIC_DISABLE_IRQ(dev->rx_dma_complete_irqn, rx_dma_complete_state);         \
    NVIC_DISABLE_IRQ(dev->spi_interrupt_irqn, spi_state);


#define SPI_PROXY_RESTORE_IRQ                                                   \
    NVIC_RESTORE_IRQ(dev->spi_interrupt_irqn, spi_state);                       \
    NVIC_RESTORE_IRQ(dev->rx_dma_complete_irqn, rx_dma_complete_state);         \
    NVIC_RESTORE_IRQ(dev->tx_dma_complete_irqn, tx_dma_complete_state);         \
}

//---------------------------- FORWARD DECLARATIONS ----------------------------
void spiproxy_initialize(struct SPIProxyInstance* dev, uint16_t index);
void spiproxy_init_vdev(struct SPIProxyInstance* dev, uint16_t index);
void spiproxy_init_gpio(struct SPIProxyInstance* dev);
void spiproxy_init_spi(struct SPIProxyInstance* dev);
void spiproxy_init_interrupt_mode(struct SPIProxyInstance* dev);
void spiproxy_init_dma_mode(struct SPIProxyInstance* dev);
void spiproxy_send(struct SPIProxyInstance* dev);
void spiproxy_receive(struct SPIProxyInstance* dev);
void spiproxy_start(struct SPIProxyInstance* dev);
void spiproxy_stop(struct SPIProxyInstance* dev);

/// \def SPI_IS_BIDIR
/// \brief Returns non-zero if SPIProxy works in bidirectional mode
#define SPI_IS_BIDIR(dev)               ((dev)->is_bidirectional)

/// \def SPI_IS_UNIDIR
/// \brief Returns non-zero if SPIProxy works in unidirectional mode
#define SPI_IS_UNIDIR(dev)              (!(dev)->is_bidirectional)

/// \def SPI_FRAME_SIZE
/// \brief Returns frame size in bytes
#define SPI_FRAME_SIZE(dev)             (((dev)->frame_size)+1)

/// \def SPI_FRAME_COUNT
/// \brief Calculates number of frames from buffer length
#define SPI_FRAME_COUNT(dev, length)    ((length) >> ((dev)->frame_size))

/// \def SPI_DMA_MODE
/// \brief Returns non-zero if DMA mode is used
#define SPI_DMA_MODE(dev)               ((dev)->use_dma != 0)

/// \brief Common TX DMA IRQ handler (DMA mode only)
/// \param index - index of the virtual device
void SPI_COMMON_TX_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIPROXY_DEVICE_COUNT);
    struct SPIProxyInstance* dev = g_spiproxy_devs+index;
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&(dev->privdata);
    assert_param(priv_data->send_frame_counter == 0);

    DMA_ClearITPendingBit(dev->dma_tx_it);
    DMA_Cmd(dev->tx_dma_channel, DISABLE);

    SPI_PROXY_DISABLE_IRQ
    assert_param(priv_data->status->running);
    priv_data->send_frame_counter = priv_data->frame_number;
    if (priv_data->recv_frame_counter==(priv_data->frame_number & priv_data->recv_frames_mask)) {
        spiproxy_stop(dev);
    }
    SPI_PROXY_RESTORE_IRQ
}
SPI_FW_TX_DMA_IRQ_HANDLERS

/// \brief Common TX DMA IRQ handler (DMA mode only)
/// \param index - index of the virtual device
void SPI_COMMON_RX_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIPROXY_DEVICE_COUNT);
    struct SPIProxyInstance* dev = g_spiproxy_devs+index;
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&(dev->privdata);
    assert_param(priv_data->recv_frame_counter == 0);
    assert_param(SPI_IS_BIDIR(dev));

    DMA_ClearITPendingBit(dev->dma_rx_it);
    DMA_Cmd(dev->rx_dma_channel, DISABLE);

    SPI_PROXY_DISABLE_IRQ
    assert_param(priv_data->status->running);
    priv_data->recv_frame_counter = priv_data->frame_number & priv_data->recv_frames_mask;
    if (priv_data->send_frame_counter==priv_data->frame_number) {
        spiproxy_stop(dev);
    }
    SPI_PROXY_RESTORE_IRQ
}
SPI_FW_RX_DMA_IRQ_HANDLERS

/// \brief Common SPIProxy IRQ handler (interrupt mode only)
/// \param index - index of the virtual device
void SPI_COMMON_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIPROXY_DEVICE_COUNT);
    struct SPIProxyInstance* dev = g_spiproxy_devs+index;
    struct DeviceContext* dev_ctx = &(dev->dev_ctx);
    struct SPIProxyPrivData* priv_data = &(dev->privdata);

    if (SPI_I2S_GetITStatus(dev->spi, SPI_I2S_IT_RXNE)!=RESET &&
        priv_data->recv_frame_counter < priv_data->frame_number) {
        // Receive buffer not empty interrupt.
        spiproxy_receive(dev);
    }

    if (SPI_I2S_GetITStatus(dev->spi, SPI_I2S_IT_TXE)!=RESET &&
        priv_data->send_frame_counter < priv_data->frame_number) {
        // Transmit buffer empty interrupt.
        spiproxy_send(dev);
    }

    if (SPI_I2S_GetITStatus(dev->spi, SPI_I2S_IT_OVR)!=RESET) {
        // Overrun interrupt.
        assert_param(0);
    }

    if (SPI_I2S_GetITStatus(dev->spi, SPI_IT_MODF)!=RESET) {
        // Mode Fault interrupt.
        assert_param(0);
    }

    if (SPI_I2S_GetITStatus(dev->spi, SPI_IT_CRCERR)!=RESET) {
        // CRC Error interrupt.
        assert_param(0);
    }

    SPI_PROXY_DISABLE_IRQ
    if (    (priv_data->send_frame_counter==priv_data->frame_number) &&
            (priv_data->recv_frame_counter==(priv_data->frame_number & priv_data->recv_frames_mask)) &&
             priv_data->status->running) {
        spiproxy_stop(dev);
        dev_ctx->bytes_available =  sizeof(struct SPIProxyStatus) +
                                    (priv_data->transmit_len & priv_data->recv_frames_mask);
    }
    SPI_PROXY_RESTORE_IRQ
}
SPI_FW_IRQ_HANDLERS

/// \brief Initializes SPIproxy
/// \param dev - SPIProxy instance definition structure.
/// \param index- index of the virtual device.
void spiproxy_initialize(struct SPIProxyInstance* dev, uint16_t index) {
        spiproxy_init_vdev(dev, index);
        spiproxy_init_spi(dev);
        spiproxy_init_gpio(dev);
}

void spiproxy_init() {
    for (uint16_t i=0; i<SPIPROXY_DEVICE_COUNT; i++) {
        struct SPIProxyInstance* dev = (struct SPIProxyInstance*)g_spiproxy_devs+i;
        spiproxy_initialize(dev, i/*, SPIPROXY_INIT_ALL*/);
    }
}

uint8_t spiproxy_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct SPIProxyInstance* dev = (struct SPIProxyInstance*)g_spiproxy_devs + devctx->dev_index;
    struct SPIProxyPrivData* priv_data = &(dev->privdata);
    uint8_t res = COMM_STATUS_FAIL;

    // Check if data is aligned by frame_size
    if ((length & dev->frame_size) != 0) {
        assert_param(0);
        res = COMM_STATUS_FAIL;
        goto done;
    }

    if (length > dev->buffer_size) {
        assert_param(0);
        res = COMM_STATUS_OVF;
        goto done;
    }

    // Copy data to the buffer
    memcpy((void*)dev->out_buffer, data, length);
    priv_data->frame_number = SPI_FRAME_COUNT(dev, length);
    priv_data->recv_frame_counter = 0;
    priv_data->send_frame_counter = 0;
    priv_data->transmit_len = length;
    devctx->bytes_available = sizeof(struct SPIProxyStatus);   // Allow read status until data is fully received

    SPI_PROXY_DISABLE_IRQ
    spiproxy_start(dev);
    SPI_PROXY_RESTORE_IRQ

    res = COMM_STATUS_OK;

done:
    return res;
}

uint8_t spiproxy_read_done(uint8_t device_id, uint16_t length) {
    UNUSED(device_id);
    UNUSED(length);
    return COMM_STATUS_OK;
}
/// \brief Preinitializes SPIProxyPrivData::dma_rx_preinit structure (used in DMA mode only)
/// \param dev - SPIProxy instance definition structure.
void spi_preinit_dma_rx(struct SPIProxyInstance* dev) {
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&(dev->privdata);
    assert_param(priv_data->dma_rx_preinit != NULL);

    DMA_DeInit(dev->rx_dma_channel);
    priv_data->dma_rx_preinit->DMA_PeripheralBaseAddr = (uint32_t)&(dev->spi->DR);
    priv_data->dma_rx_preinit->DMA_MemoryBaseAddr = (uint32_t)priv_data->in_data_buffer;
    priv_data->dma_rx_preinit->DMA_DIR = DMA_DIR_PeripheralSRC;
    priv_data->dma_rx_preinit->DMA_BufferSize = 0;
    priv_data->dma_rx_preinit->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    priv_data->dma_rx_preinit->DMA_MemoryInc = DMA_MemoryInc_Enable;
    priv_data->dma_rx_preinit->DMA_PeripheralDataSize = (SPI_FRAME_SIZE(dev) == 1) ? DMA_PeripheralDataSize_Byte : DMA_PeripheralDataSize_HalfWord;
    priv_data->dma_rx_preinit->DMA_MemoryDataSize = (SPI_FRAME_SIZE(dev) == 1) ? DMA_MemoryDataSize_Byte : DMA_MemoryDataSize_HalfWord;
    priv_data->dma_rx_preinit->DMA_Mode = DMA_Mode_Normal;
    priv_data->dma_rx_preinit->DMA_Priority = DMA_Priority_VeryHigh;
    priv_data->dma_rx_preinit->DMA_M2M = DMA_M2M_Disable;
}

/// \brief Preinitializes SPIProxyPrivData::dma_tx_preinit structure (used in DMA mode only)
/// \param dev - SPIProxy instance definition structure.
void spi_preinit_dma_tx(struct SPIProxyInstance* dev) {
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*) &(dev->privdata);
    assert_param(priv_data->dma_tx_preinit != NULL);

    DMA_DeInit(dev->tx_dma_channel);
    priv_data->dma_tx_preinit->DMA_PeripheralBaseAddr = (uint32_t) &(dev->spi->DR);
    priv_data->dma_tx_preinit->DMA_MemoryBaseAddr = (uint32_t) dev->out_buffer;
    priv_data->dma_tx_preinit->DMA_DIR = DMA_DIR_PeripheralDST;
    priv_data->dma_tx_preinit->DMA_BufferSize = 0;
    priv_data->dma_tx_preinit->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    priv_data->dma_tx_preinit->DMA_MemoryInc = DMA_MemoryInc_Enable;
    priv_data->dma_tx_preinit->DMA_PeripheralDataSize = (SPI_FRAME_SIZE(dev) == 1) ? DMA_PeripheralDataSize_Byte
                                                                        : DMA_PeripheralDataSize_HalfWord;
    priv_data->dma_tx_preinit->DMA_MemoryDataSize = (SPI_FRAME_SIZE(dev) == 1) ? DMA_MemoryDataSize_Byte
                                                                    : DMA_MemoryDataSize_HalfWord;
    priv_data->dma_tx_preinit->DMA_Mode = DMA_Mode_Normal;
    priv_data->dma_tx_preinit->DMA_Priority = DMA_Priority_VeryHigh;
    priv_data->dma_tx_preinit->DMA_M2M = DMA_M2M_Disable;
}

/// \brief Initializes structures and registers virtual device.
/// \param dev - SPIProxy instance definition structure.
/// \param index- index of the virtual device.
void spiproxy_init_vdev(struct SPIProxyInstance* dev, uint16_t index) {
    assert_param( dev->buffer_size > 0 );

    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&(dev->privdata);
    memset((void*)devctx, 0, sizeof(struct DeviceContext));

    priv_data->recv_frames_mask = SPI_IS_BIDIR(dev) ? 0xFFFF : 0;
    priv_data->status = (struct SPIProxyStatus*)dev->in_status_and_data_buffer;
    priv_data->in_data_buffer = dev->in_status_and_data_buffer + sizeof(struct SPIProxyStatus);

    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->buffer       = dev->in_status_and_data_buffer;
    devctx->bytes_available = sizeof(struct SPIProxyStatus);
    devctx->on_command   = spiproxy_execute;
    devctx->on_read_done = spiproxy_read_done;

    comm_register_device(devctx);
}

/// \brief Initializes virtual device GPIO.
/// \param dev - SPIProxy instance definition structure.
void spiproxy_init_gpio(struct SPIProxyInstance* dev){
    START_PIN_DECLARATION;
    // Enable alternative function if required
    if (dev->remap) {
        if (dev->spi == SPI1) {
            GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
        } else if (dev->spi == SPI3) {
            GPIO_PinRemapConfig(GPIO_Remap_SPI3, ENABLE);
        } else {
            assert_param(0);
        }
    }

    // Configure MOSI (TX) pin as output
    DECLARE_PIN(dev->mosi_port, 1 << dev->mosi_pin, GPIO_Mode_AF_PP);

    if (SPI_IS_BIDIR(dev)) {
        // Configure MISO (RX) pin as input
        DECLARE_PIN(dev->miso_port, 1 << dev->miso_pin, GPIO_Mode_IPD);
    }

    // Configure SCK and NSS pins as output
    DECLARE_PIN(dev->sck_port, 1 << dev->sck_pin, GPIO_Mode_AF_PP);
    SPI_PROXY_DISABLE_IRQ
    spiproxy_stop(dev);
    SPI_PROXY_RESTORE_IRQ
}

/// \brief Initializes virtual device SPI peripherals.
/// \param dev - SPIProxy instance definition structure.
void spiproxy_init_spi(struct SPIProxyInstance* dev) {
    SPI_InitTypeDef init_struct;

    init_struct.SPI_Direction =  SPI_IS_BIDIR(dev) ?
                                    SPI_Direction_2Lines_FullDuplex :
                                    SPI_Direction_1Line_Tx;

    init_struct.SPI_Mode      = SPI_Mode_Master;

    init_struct.SPI_DataSize  = dev->frame_size ?
                                    SPI_DataSize_16b :
                                    SPI_DataSize_8b;

    init_struct.SPI_CPOL      = dev->clock_polarity ?
                                    SPI_CPOL_High :
                                    SPI_CPOL_Low;

    init_struct.SPI_CPHA      = dev->clock_phase ?
                                    SPI_CPHA_2Edge :
                                    SPI_CPHA_1Edge;

    init_struct.SPI_NSS       = SPI_NSS_Hard;

    init_struct.SPI_BaudRatePrescaler = dev->baud_rate_control;

    init_struct.SPI_FirstBit =  dev->frame_format ?
                                    SPI_FirstBit_MSB :
                                    SPI_FirstBit_LSB;

    init_struct.SPI_CRCPolynomial = 7;

    SPI_Init(dev->spi, &init_struct);

    if (SPI_DMA_MODE(dev)) {
        spiproxy_init_dma_mode(dev);
    } else {
        spiproxy_init_interrupt_mode(dev);
    }
}

/// \brief Initializes SPI in DMA mode (DMA mode only)
/// \param dev - SPIProxy instance definition structure.
void spiproxy_init_dma_mode(struct SPIProxyInstance* dev) {
    spi_preinit_dma_tx(dev);
    if (SPI_IS_BIDIR(dev)) {
        spi_preinit_dma_rx(dev);
    }
}

/// \brief Initializes SPI in interrupt mode (interrupt mode only)
/// \param dev - SPIProxy instance definition structure.
void spiproxy_init_interrupt_mode(struct SPIProxyInstance* dev) {
    NVIC_SetPriority(dev->spi_interrupt_irqn, IRQ_PRIORITY_SPI);
    NVIC_EnableIRQ(dev->spi_interrupt_irqn);
    SPI_I2S_ITConfig(dev->spi, SPI_I2S_IT_TXE, ENABLE);
    SPI_I2S_ITConfig(dev->spi, SPI_I2S_IT_ERR, ENABLE);

    if (SPI_IS_BIDIR(dev)) {
        SPI_I2S_ITConfig(dev->spi, SPI_I2S_IT_RXNE, ENABLE);
    }
}

/// \brief Sends a frame to SPI (interrupt mode only)
/// \param dev - SPIProxy instance definition structure.
void spiproxy_send(struct SPIProxyInstance* dev) {
    uint16_t data = 0;
    uint8_t* pdata = (uint8_t*)&data;
    uint16_t data_offset;
    struct SPIProxyPrivData* priv_data = &(dev->privdata);
    SPI_PROXY_DISABLE_IRQ
    data_offset = priv_data->send_frame_counter << dev->frame_size;
    assert_param(data_offset < dev->buffer_size);
    SPI_PROXY_RESTORE_IRQ

    pdata[0] = dev->out_buffer[data_offset];
    pdata[dev->frame_size] = dev->out_buffer[data_offset+dev->frame_size];
    SPI_I2S_SendData(dev->spi, data);

    SPI_PROXY_DISABLE_IRQ
    priv_data->send_frame_counter++;
    assert_param(priv_data->send_frame_counter <= priv_data->frame_number);
    SPI_PROXY_RESTORE_IRQ
}

/// \brief Receives a frame from SPI (interrupt mode only)
/// \param dev - SPIProxy instance definition structure.
void spiproxy_receive(struct SPIProxyInstance* dev) {
    uint16_t data = 0;
    uint8_t* pdata = (uint8_t*)&data;
    uint16_t data_offset;
    struct SPIProxyPrivData* priv_data = &(dev->privdata);

    if (SPI_IS_UNIDIR(dev)) {
        assert_param(0);
        goto done;
    }

    data = SPI_I2S_ReceiveData(dev->spi);

    SPI_PROXY_DISABLE_IRQ
    data_offset = priv_data->recv_frame_counter << dev->frame_size;
    assert_param(data_offset < dev->buffer_size);
    SPI_PROXY_RESTORE_IRQ

    priv_data->in_data_buffer[data_offset] = pdata[0];
    priv_data->in_data_buffer[data_offset+dev->frame_size] = pdata[dev->frame_size];

    SPI_PROXY_DISABLE_IRQ
    priv_data->recv_frame_counter++;
    assert_param(priv_data->recv_frame_counter <= priv_data->frame_number);
    SPI_PROXY_RESTORE_IRQ

done:
    return;
}

/// \brief Starts SPI transaction.
/// \param dev - SPIProxy instance definition structure.
void spiproxy_start(struct SPIProxyInstance* dev) {
    START_PIN_DECLARATION;
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&dev->privdata;

    // Enable DMA if needed
    if (SPI_DMA_MODE(dev)) {
        priv_data->dma_tx_preinit->DMA_BufferSize = priv_data->frame_number;
        DMA_Init(dev->tx_dma_channel, priv_data->dma_tx_preinit);

        // Enable DMA channel
        DMA_Cmd(dev->tx_dma_channel, ENABLE);

        // Enable DMA interrupt
        NVIC_SetPriority(dev->tx_dma_complete_irqn, IRQ_PRIORITY_DMA);
        NVIC_EnableIRQ(dev->tx_dma_complete_irqn);
        DMA_ITConfig(dev->tx_dma_channel, DMA_IT_TC, ENABLE);

        // Enable DMA mode in SPI
        SPI_I2S_DMACmd(dev->spi, SPI_I2S_DMAReq_Tx, ENABLE);

        if (SPI_IS_BIDIR(dev)) {
            priv_data->dma_rx_preinit->DMA_BufferSize = priv_data->frame_number;
            DMA_Init(dev->rx_dma_channel, priv_data->dma_rx_preinit);

            // Enable DMA channel
            DMA_Cmd(dev->rx_dma_channel, ENABLE);

            // Enable DMA interrupt
            NVIC_SetPriority(dev->rx_dma_complete_irqn, IRQ_PRIORITY_DMA);
            NVIC_EnableIRQ(dev->rx_dma_complete_irqn);
            DMA_ITConfig(dev->rx_dma_channel, DMA_IT_TC, ENABLE);

            // Enable DMA mode in SPI
            SPI_I2S_DMACmd(dev->spi, SPI_I2S_DMAReq_Rx, ENABLE);
        }
    }

    // Start
    SPI_SSOutputCmd(dev->spi, ENABLE);
    SPI_Cmd(dev->spi, ENABLE);

    DECLARE_PIN(dev->nss_port, 1 << dev->nss_pin, GPIO_Mode_AF_PP);

    dev->privdata.status->running = 1;
}

/// \brief Stops SPI transaction.
/// \param dev - SPIProxy instance definition structure.
void spiproxy_stop(struct SPIProxyInstance* dev) {
    struct SPIProxyPrivData* priv_data = (struct SPIProxyPrivData*)&(dev->privdata);
    START_PIN_DECLARATION;

    // Disable DMA if needed
    if (SPI_DMA_MODE(dev)) {
        DMA_DeInit(dev->tx_dma_channel);
        if (SPI_IS_BIDIR(dev)) {
            DMA_DeInit(dev->rx_dma_channel);
        }
    }

    // Disable NSS
    DECLARE_PIN(dev->nss_port, 1 << dev->nss_pin, GPIO_Mode_Out_PP);
    GPIO_SetBits(dev->nss_port, 1 << dev->nss_pin);

    // Stop
    dev->dev_ctx.bytes_available = sizeof(struct SPIProxyStatus) + (priv_data->transmit_len & priv_data->recv_frames_mask);
    assert_param(SPI_I2S_GetFlagStatus(dev->spi, SPI_I2S_FLAG_BSY)==RESET);
    SPI_Cmd(dev->spi, DISABLE);
    SPI_SSOutputCmd(dev->spi, DISABLE);


    dev->privdata.status->running = 0;
}
#endif

