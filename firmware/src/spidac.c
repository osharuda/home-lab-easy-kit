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
 *   \brief SPIDAC device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef SPIDAC_DEVICE_ENABLED

#include <string.h>
#include "utools.h"
#include "i2c_bus.h"
#include "spidac.h"
#include <stm32f10x.h>

/// \addtogroup group_spidac
/// @{

/// \def SPIDAC_FRAME_SIZE
/// \brief Returns frame size in bytes
#define SPIDAC_FRAME_SIZE(dev)             (((dev)->frame_size)+1)

//---------------------------- FORWARD AND INLINE DECLARATIONS ----------------------------

/// \brief Prepares channel data for sampling.
/// \param dev - Device instance to start.
/// \param priv_data - private data for the device.
/// \param start_info - sampling start information structure. It may be ether software side received structure or default
///        sample pre-initialized structure.
/// \param overflow_status - Status to be set when sample buffer is completely sampled.
/// \note If no samples are uploaded for some channel, default sample will be used.
static inline void spidac_init_channels_data(struct SPIDACInstance* dev,
                                             struct SPIDACPrivData* priv_data,
                                             struct SPIDACStartInfo* start_info,
                                             SPIDAC_STATUS overflow_status);


/// \brief Initializes generation and sends the very first sample.
/// \param dev - Device instance to start.
/// \note All other samples and status changes are made inside timer and DMA (TC) IRQs.
static inline void spidac_sample_first(struct SPIDACInstance* dev, struct SPIDACPrivData* priv_data);
static inline void spidac_sample_next(struct SPIDACInstance* dev, struct SPIDACPrivData* priv_data);


SPIDAC_FW_DEFAULT_VALUES
SPIDAC_FW_BUFFERS

/// \brief Global array that stores all virtual SPIDAC devices configurations.
struct SPIDACInstance g_spidac_devs[] = SPIDAC_FW_DEV_DESCRIPTOR;

#ifndef NDEBUG
uint8_t dac_irq_disabled = 0;
#define DAC_CHECK_IRQ_ENTER assert(dac_irq_disabled==0); \
                            dac_irq_disabled = 1;

#define DAC_CHECK_IRQ_LEAVE assert(dac_irq_disabled==1); \
                            dac_irq_disabled = 0;
#else
#define DAC_CHECK_IRQ_ENTER (void)(0);
#define DAC_CHECK_IRQ_LEAVE (void)(0);
#endif

/// \define DAC_DISABLE_IRQs
/// \brief Saves current DACDev NVIC IRQ state, and temporary disables specified IRQ
#define DAC_DISABLE_IRQs                                                    \
    uint32_t timer_state = NVIC_IRQ_STATE(dev->timer_irqn);                 \
    uint32_t tx_dma_state = NVIC_IRQ_STATE(dev->tx_dma_complete_irqn);      \
    NVIC_DISABLE_IRQ(dev->timer_irqn, timer_state);                         \
    NVIC_DISABLE_IRQ(dev->tx_dma_complete_irqn, tx_dma_state);              \
    DAC_CHECK_IRQ_ENTER


/// \define DAC_RESTORE_IRQs
/// \brief Restores DAC IRQs state
#define DAC_RESTORE_IRQs                                                    \
    DAC_CHECK_IRQ_LEAVE                                                     \
    NVIC_RESTORE_IRQ(dev->tx_dma_complete_irqn, tx_dma_state);              \
    NVIC_RESTORE_IRQ(dev->timer_irqn, timer_state);                         \


static inline void spidac_sample_first(
        struct SPIDACInstance* dev,
        struct SPIDACPrivData* priv_data) {
    assert_param(priv_data->current_channel_data == priv_data->channel_data);     // First channel must be selected
    assert_param(priv_data->current_channel_data->first_sample_ptr != NULL);
    assert_param(priv_data->current_channel_data->end_sample_ptr != NULL);
    assert_param(priv_data->current_channel_data->current_sample_ptr != NULL);

    DAC_DISABLE_IRQs
    assert_param(priv_data->status->status == STOPPED ||
                 priv_data->status->status == STOPPED_ABNORMAL ||
                 priv_data->status->status == WAITING);
    priv_data->status->status = SAMPLING;
    DAC_RESTORE_IRQs

    priv_data->dma_tx_preinit.DMA_MemoryBaseAddr = (uint32_t)priv_data->current_channel_data->current_sample_ptr;

    // Enable DMA channel
    DMA_Init(dev->tx_dma_channel, (DMA_InitTypeDef*)&(priv_data->dma_tx_preinit));

    // Enable DMA interrupt
    DMA_ITConfig(dev->tx_dma_channel, DMA_IT_TC, ENABLE);

    // Enable DMA mode in SPI
    SPI_I2S_DMACmd(dev->spi, SPI_I2S_DMAReq_Tx, ENABLE);

    // Optimization: cache spi->CR1 register, we will use it in STARTED state to optimize timer interrupt
    priv_data->spi_cr1_disabled = dev->spi->CR1;

    // Start
    SPI_Cmd(dev->spi, ENABLE);

    // Optimization: cache spi->CR1 register, we will use it in STARTED state to optimize timer interrupt
    priv_data->spi_cr1_enabled = dev->spi->CR1;
    priv_data->dma_ccr_disabled = dev->tx_dma_channel->CCR;

    DMA_Cmd(dev->tx_dma_channel, ENABLE);
    priv_data->dma_ccr_enabled = dev->tx_dma_channel->CCR;
}

static inline void spidac_sample_next(
        struct SPIDACInstance* dev,
        struct SPIDACPrivData* priv_data) {
    // Enable SPI
    dev->spi->CR1 = priv_data->spi_cr1_enabled;

    // Restart DMA
    dev->tx_dma_channel->CCR   = priv_data->dma_ccr_disabled;
    dev->tx_dma_channel->CMAR  = (uint32_t)priv_data->current_channel_data->current_sample_ptr;
    dev->tx_dma_channel->CNDTR = (uint32_t)dev->transaction_size;
    dev->tx_dma_channel->CCR   = priv_data->dma_ccr_enabled;
}


/// \brief Common TX DMA IRQ handler
/// \param index - index of the virtual device
/// \note  We don't disable irq here because priority of DMA request is higher than priority of the DAC timer.
///        Moreover DMA TX handler doesn't access to the DACDev status value. So it is safe to access variables
///        from the private device data from here.
void SPIDAC_COMMON_TX_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIDAC_DEVICE_COUNT);
    uint8_t new_status = WAITING;

    struct SPIDACInstance* dev = (struct SPIDACInstance*)g_spidac_devs+index;

    // Clear TX DMA interrupt pending bit (otherwise it will be called again once handler is returned)
    dev->dma->IFCR = dev->dma_tx_it;

    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&(dev->priv_data);
    SPI_TypeDef* spi = dev->spi;

    // Increment sample pointer
    priv_data->current_channel_data->current_sample_ptr += priv_data->current_channel_data->phase_increment;

    // Check for sample pointer overflow
    if (priv_data->current_channel_data->current_sample_ptr >= priv_data->current_channel_data->end_sample_ptr) {
        // It must be equal, otherwise either buffer or phase_increment are not aligned.
        assert_param(priv_data->current_channel_data->current_sample_ptr < (priv_data->current_channel_data->end_sample_ptr+priv_data->current_channel_data->samples_len));
        priv_data->current_channel_data->current_sample_ptr -= priv_data->current_channel_data->samples_len;
        new_status = priv_data->current_channel_data->phase_overflow_status;
    }

    // Wait SPI to complete transaction.
    // Note, you may try to comment line below (with while waiting loop) to achieve some performance improvement in some situations;
    // However this will introduce a bug (for example: LD signal will appear BEFORE SPI transaction ends). Do this very
    // carefully, check results with oscilloscope to make sure code above this comment is enough to 'wait' for SPI
    // transaction end. If not, commenting out this line will cause issues. Do not comment it out when using slow SPI speeds.
    while ((spi->SR & SPI_I2S_FLAG_BSY) != 0) {}

    // Disable SPI
    spi->CR1 = priv_data->spi_cr1_disabled;

#if SPIDAC_MULTI_CHANNEL
    // Switch to the next channel and start new transaction immediately.
    priv_data->current_channel_data++;
    if (priv_data->current_channel_data < priv_data->end_channel_data) {
        spidac_sample_next(dev, priv_data); // Sample next channel
        return;
    } else {
        priv_data->current_channel_data = priv_data->channel_data; // Switch to the first channel
    }
#endif

#if SPIDAC_NEED_LD
    // LD pulse
    *(priv_data->ld_port_BSRR) = dev->ld_bit_mask;
    *(priv_data->ld_port_BRR) = dev->ld_bit_mask;
#endif

    priv_data->status->status = new_status;

}
SPIDAC_FW_TX_DMA_IRQ_HANDLERS


static inline void spidac_wait(struct SPIDACInstance* dev, struct SPIDACPrivData* priv_data) {
    assert_param(IN_INTERRUPT == 0);// Must not be called from ISR
    uint8_t last_status;
    do {
        DAC_DISABLE_IRQs
        last_status = priv_data->status->status;
        DAC_RESTORE_IRQs
    } while (last_status != STOPPED);
}

/// \brief Common TIMER IRQ handler
/// \param index - index of the virtual device
void SPIDAC_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIDAC_DEVICE_COUNT);
    struct SPIDACInstance* dev       = (struct SPIDACInstance*)g_spidac_devs+index;
    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&(dev->priv_data);

    if (dev->timer->SR & TIM_IT_Update) {
        TIM_ClearITPendingBit(dev->timer, TIM_IT_Update);

        if (priv_data->status->status == WAITING) {
            priv_data->status->status = SAMPLING;

            /// Note, it is legal to start sampling with spidac_sample_next() because spidac_sample_first() was already called
            /// during virtual device initialization!
            spidac_sample_next(dev, priv_data);

            return;
        }

        uint8_t final_status = priv_data->status->status == STOPPED ? STOPPED : STOPPED_ABNORMAL;
        spidac_shutdown(dev, STOPPED);

        // set default value
        spidac_init_channels_data(
                dev,
                &dev->priv_data,
                dev->default_start_info,
                STOPPED);

        spidac_sample_first(dev, priv_data);
        spidac_wait(dev, priv_data);

        spidac_shutdown(dev, final_status);
    }
}
SPIDAC_FW_TIMER_IRQ_HANDLERS

/// @}

void spidac_init_vdev(struct SPIDACInstance* dev, uint16_t index) {
    assert_param( dev->buffer_size > 0 );

    struct DeviceContext* devctx = (struct DeviceContext*)&(dev->dev_ctx);
    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&(dev->priv_data);


    // There should be at least one sample
    assert_param(dev->buffer_size >= sizeof(struct SPIDACStatus) + dev->frames_per_sample * SPIDAC_FRAME_SIZE(dev));
    priv_data->sample_buffer_size = 0;
    priv_data->status             = (struct SPIDACStatus*)dev->buffer;
    priv_data->status->status     = STOPPED;
    priv_data->status->repeat_count = 0;

    memcpy((void*)dev->default_sample_base, (void*)dev->default_values, dev->sample_size);
    memset(priv_data->channel_data, 0, sizeof(struct SPIDACChannelData) * dev->channel_count);
    dev->default_start_info->period = 0;
    dev->default_start_info->prescaler = 0;


    if (dev->ld_port != NULL) {
        if (dev->ld_rise) {
            priv_data->ld_port_BSRR = &(dev->ld_port->BSRR);
            priv_data->ld_port_BRR = &(dev->ld_port->BRR);
        } else {
            // Swap registers to invert polarity
            priv_data->ld_port_BSRR = &(dev->ld_port->BRR);
            priv_data->ld_port_BRR = &(dev->ld_port->BSRR);
        }
    } else {
        priv_data->ld_port_BSRR = &g_dummy_reg32;
        priv_data->ld_port_BRR = &g_dummy_reg32;
    }

    memset((void*)devctx, 0, sizeof(struct DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->buffer       = (uint8_t*)priv_data->status;
    devctx->bytes_available = sizeof(struct SPIDACStatus);
    devctx->on_command   = spidac_execute;
    devctx->on_read_done = spidac_read_done;

    comm_register_device(devctx);

    // GPIO initialization ----------------------------------------------------------------------------
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
    DECLARE_PIN(dev->mosi_port, 1 << dev->mosi_pin, GPIO_Mode_AF_PP);
    DECLARE_PIN(dev->sck_port, 1 << dev->sck_pin, GPIO_Mode_AF_PP);

    if (dev->ld_port) {
        DECLARE_PIN(dev->ld_port, dev->ld_bit_mask, GPIO_Mode_Out_PP);
        GPIO_WriteBit(dev->ld_port, dev->ld_bit_mask, !dev->ld_rise);
    }

    // Disable NSS (this pin must be pulled up to the VCC)
    DECLARE_PIN(dev->nss_port, 1 << dev->nss_pin, GPIO_Mode_AF_PP);


    // SPI Initialization ----------------------------------------------------------------------------
    SPI_InitTypeDef  init_struct;

    init_struct.SPI_Direction = SPI_Direction_1Line_Tx;

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

    init_struct.SPI_FirstBit =  SPI_FirstBit_MSB; // Always use MSB, actual frame structure is controlled by software.

    init_struct.SPI_CRCPolynomial = 7;

    SPI_Init(dev->spi, &init_struct);
    SPI_SSOutputCmd(dev->spi, ENABLE);
    SPI_I2S_DMACmd(dev->spi, SPI_I2S_DMAReq_Tx, ENABLE);

    // Stop spi ----------------------------------------------------------------------------
    /* spidac_stop(dev); */

    // Pre-initialize SPI TX DMA ----------------------------------------------------------------------------
    DMA_DeInit(dev->tx_dma_channel);
    priv_data->dma_tx_preinit.DMA_PeripheralBaseAddr = (uint32_t) &(dev->spi->DR);
    priv_data->dma_tx_preinit.DMA_MemoryBaseAddr = (uint32_t) dev->sample_buffer_base;
    priv_data->dma_tx_preinit.DMA_DIR = DMA_DIR_PeripheralDST;
    priv_data->dma_tx_preinit.DMA_BufferSize = dev->transaction_size;
    priv_data->dma_tx_preinit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    priv_data->dma_tx_preinit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    priv_data->dma_tx_preinit.DMA_PeripheralDataSize = (SPIDAC_FRAME_SIZE(dev) == 1) ? DMA_PeripheralDataSize_Byte
                                                                                     : DMA_PeripheralDataSize_HalfWord;
    priv_data->dma_tx_preinit.DMA_MemoryDataSize = (SPIDAC_FRAME_SIZE(dev) == 1) ? DMA_MemoryDataSize_Byte
                                                                                 : DMA_MemoryDataSize_HalfWord;
    priv_data->dma_tx_preinit.DMA_Mode = DMA_Mode_Normal;
    priv_data->dma_tx_preinit.DMA_Priority = DMA_Priority_VeryHigh;
    priv_data->dma_tx_preinit.DMA_M2M = DMA_M2M_Disable;

    NVIC_SetPriority(dev->tx_dma_complete_irqn, IRQ_PRIORITY_DMA);
    NVIC_EnableIRQ(dev->tx_dma_complete_irqn);
    DMA_ITConfig(dev->tx_dma_channel, DMA_IT_TC, ENABLE);
    DMA_Init(dev->tx_dma_channel, (DMA_InitTypeDef*)&(priv_data->dma_tx_preinit));
 }

void spidac_init() {
    for (uint16_t i=0; i<SPIDAC_DEVICE_COUNT; i++) {
        struct SPIDACInstance* dev = (struct SPIDACInstance*)g_spidac_devs+i;
        struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&dev->priv_data;
        spidac_init_vdev(dev, i);

        // set default value
        spidac_init_channels_data(
                dev,
                &dev->priv_data,
                dev->default_start_info,
                STOPPED);

        spidac_sample_first(dev, priv_data);
        spidac_wait(dev, priv_data);
        spidac_shutdown(dev, STOPPED);
    }
}

uint8_t spidac_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(cmd_byte);
    struct SPIDACInstance* dev = (struct SPIDACInstance*)g_spidac_devs + devctx->dev_index;
    const uint16_t start_info_len = sizeof(struct SPIDACStartInfo) + dev->channel_count * sizeof(struct SPIDACChannelSamplingInfo);
    const uint16_t upd_phase_len = dev->channel_count * sizeof(struct SPIDACChannelPhaseInfo);
    uint8_t res = COMM_STATUS_OK;
    SPIDAC_COMMAND command = cmd_byte & COMM_CMDBYTE_DEV_SPECIFIC_MASK;

    if (command==UPD_PHASE && length == upd_phase_len) {
        res = spidac_update_phase(dev, (struct SPIDACChannelPhaseInfo*)data);
    } else
    if (command==START && length == start_info_len) {
        res = spidac_start(dev, (struct SPIDACStartInfo*)data, 1);
    } else if (command==START_PERIOD && length == start_info_len) {
        res = spidac_start(dev, (struct SPIDACStartInfo*)data, 0);
    } else if (command==SETDEFAULT && length == dev->sample_size) {
        memcpy(dev->default_sample_base, data, length);
        spidac_stop(dev);
    } else if (command==DATA_START) {
        res = spidac_data(dev, data, length, 1);
    } else if (command==DATA) {
        res = spidac_data(dev, data, length, 0);
    } else if (command==STOP) {
        res = spidac_stop(dev);
    } else {
        res = COMM_STATUS_FAIL;
    }

    return res;
}

uint8_t spidac_read_done(uint8_t device_id, uint16_t length) {
    struct DeviceContext* devctx = comm_dev_context(device_id);
    struct SPIDACInstance* dev = g_spidac_devs + devctx->dev_index;

    UNUSED(dev);
    UNUSED(length);

    return COMM_STATUS_OK;
}

uint8_t spidac_stop(struct SPIDACInstance* dev) {
    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&dev->priv_data;

    DAC_DISABLE_IRQs
        spidac_shutdown(dev, STOPPED);
    DAC_RESTORE_IRQs

    spidac_init_channels_data(
            dev,
            priv_data,
            dev->default_start_info,
            STOPPED);

    spidac_sample_first(dev, priv_data);
    spidac_wait(dev, priv_data);
    spidac_shutdown(dev, STOPPED);

    return COMM_STATUS_OK;
}

uint8_t spidac_data(struct SPIDACInstance* dev, uint8_t* data, uint16_t length, uint8_t first_portion) {
    struct SPIDACPrivData* priv_data = &(dev->priv_data);
    uint16_t next_portion = !first_portion;
    uint16_t new_sample_buffer_size = next_portion*priv_data->sample_buffer_size + length;
    if (priv_data->status->status != STOPPED &&
        priv_data->status->status != STOPPED_ABNORMAL) {
        return COMM_STATUS_FAIL;
    }

    if (length % dev->transaction_size != 0) {
        return COMM_STATUS_FAIL; // Unaligned buffer
    }

    if (new_sample_buffer_size > dev->max_sample_buffer_size) {
        return COMM_STATUS_FAIL; // Buffer size limit is exceeded.
    }

    memcpy((void*)(dev->sample_buffer_base+next_portion*priv_data->sample_buffer_size), data, length);
    priv_data->sample_buffer_size = new_sample_buffer_size;

    return COMM_STATUS_OK;
}

static inline void spidac_init_channels_data(struct SPIDACInstance* dev,
                                             struct SPIDACPrivData* priv_data,
                                             struct SPIDACStartInfo* start_info,
                                             SPIDAC_STATUS overflow_status) {

    uint16_t channel_offset = 0;
    uint8_t force_default = (start_info==dev->default_start_info);

    for (uint16_t ch = 0; ch < dev->channel_count; ch++) {
        // If no sample data available we are using default sample
        struct SPIDACChannelData* ch_info = priv_data->channel_data + ch;
        struct SPIDACChannelSamplingInfo* src_ch_smpl_info = start_info->channel_info + ch;
        struct SPIDACChannelSamplingInfo* dst_ch_smpl_info = priv_data->status->start_info.channel_info + ch;
        memcpy(dst_ch_smpl_info, src_ch_smpl_info, sizeof(struct SPIDACChannelSamplingInfo));

        if (force_default | (!src_ch_smpl_info->loaded_samples_number) ) {
            /// Default sample
            ch_info->first_sample_ptr = dev->default_sample_base + ch * dev->transaction_size;
            ch_info->end_sample_ptr = ch_info->first_sample_ptr + dev->transaction_size;
            ch_info->phase_increment = dev->transaction_size;
            ch_info->current_sample_ptr = ch_info->first_sample_ptr;
        } else {
            /// Uploaded samples
            ch_info->first_sample_ptr = dev->sample_buffer_base + channel_offset * dev->transaction_size;
            ch_info->end_sample_ptr = ch_info->first_sample_ptr + src_ch_smpl_info->loaded_samples_number * dev->transaction_size;
            ch_info->phase_increment = src_ch_smpl_info->phase.phase_increment * dev->transaction_size;

            ch_info->current_sample_ptr = ch_info->first_sample_ptr + (src_ch_smpl_info->phase.phase % src_ch_smpl_info->loaded_samples_number) * dev->transaction_size;
        }
        ch_info->samples_len = ch_info->end_sample_ptr - ch_info->first_sample_ptr;
        ch_info->phase_overflow_status = overflow_status;
        channel_offset += src_ch_smpl_info->loaded_samples_number;
    }

    priv_data->current_channel_data = priv_data->channel_data;
    priv_data->status->start_info.period = start_info->period;
    priv_data->status->start_info.prescaler = start_info->prescaler;

}


uint8_t spidac_start(struct SPIDACInstance* dev, struct SPIDACStartInfo* start_info, uint8_t continuous) {
    uint8_t res = COMM_STATUS_FAIL;
    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&(dev->priv_data);
    struct SPIDACStatus* status = (struct SPIDACStatus*)(priv_data->status);

    DAC_DISABLE_IRQs
    if (status->status == STOPPED || status->status == STOPPED_ABNORMAL) {
        status->status = WAITING;
        DAC_RESTORE_IRQs

        // Prepare for sampling
        spidac_init_channels_data(
                dev,
                priv_data,
                start_info,
                continuous != 0 ? WAITING : STOPPED);

        timer_start_periodic_ex(dev->timer,
                                start_info->prescaler,
                                start_info->period,
                                dev->timer_irqn,
                                IRQ_PRIORITY_DAC_TIMER,
                                0);

        spidac_sample_first(dev, priv_data);

        res = COMM_STATUS_OK;
    } else {
        DAC_RESTORE_IRQs
    }

    return res;
}

uint8_t spidac_update_phase(struct SPIDACInstance* dev, struct SPIDACChannelPhaseInfo* phase_info) {
    uint8_t res;
    struct SPIDACPrivData* priv_data = (struct SPIDACPrivData*)&(dev->priv_data);
    struct SPIDACStatus* status = (struct SPIDACStatus*)(priv_data->status);
    uint8_t last_status;
    struct SPIDACChannelData* ch_data = priv_data->channel_data;

    do {
        DAC_DISABLE_IRQs
        last_status = status->status;
        if (last_status == WAITING) {
            for (uint8_t i=0; i<dev->channel_count; i++, ch_data++, phase_info++) {
                assert_param(phase_info->phase >= 0);
                assert_param((uint32_t)phase_info->phase * dev->transaction_size < ch_data->samples_len);
                assert_param(ch_data->current_sample_ptr >= ch_data->first_sample_ptr);

                uint32_t offset = (uint32_t)ch_data->current_sample_ptr-(uint32_t)ch_data->first_sample_ptr+(uint32_t)(phase_info->phase*dev->transaction_size);
                uint32_t length = ch_data->end_sample_ptr-ch_data->first_sample_ptr;

                assert_param((offset % length) % dev->transaction_size == 0);
                ch_data->current_sample_ptr = ch_data->first_sample_ptr + ( offset % length );
                ch_data->phase_increment = phase_info->phase_increment * dev->transaction_size;
            }
        }
        DAC_RESTORE_IRQs
    } while(last_status==SAMPLING);
    res = last_status == WAITING ? COMM_STATUS_OK : COMM_STATUS_FAIL;

    return res;
}

void spidac_shutdown(struct SPIDACInstance* dev, uint8_t status) {
    struct SPIDACPrivData* priv_data = &(dev->priv_data);
    assert_param(status==STOPPED || status==STOPPED_ABNORMAL);

    // Disable timer
    timer_disable(dev->timer, dev->timer_irqn);

    // Disable DMA
    DMA_DeInit(dev->tx_dma_channel);

    // Disable SPI
    while ((dev->spi->SR & SPI_I2S_FLAG_BSY) != 0) {}
    assert_param(SPI_I2S_GetFlagStatus(dev->spi, SPI_I2S_FLAG_BSY)==RESET);
    SPI_Cmd(dev->spi, DISABLE);

    // Handle statuses
    priv_data->status->status = status;
}



#endif
