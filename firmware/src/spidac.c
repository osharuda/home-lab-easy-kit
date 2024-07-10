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
/// \brief Prepares SPIDAC device to using sample buffer.
/// \param priv_data - private data for the device
/// \param sampling - pointer to the SPIDACSampling structure with sampling information
/// \param continuous - non-zero specifies to generate signal indefinitely, until explicitly stopped by software;
///                     zero specifies to generate single period of the signal.
static inline void spidac_set_sampling_buffer(PSPIDACPrivData priv_data, PSPIDACSampling sampling, uint8_t continuous);

/// \brief Prepares SPIDAC device to using default sample.
/// \param priv_data - private data for the device
/// \param data - pointer to the first sample to be used. If NULL default sample from internal device buffer is used;
///               If non-null sample is copied to the internal device buffer and used.
static inline void spidac_set_default_sample(PSPIDACPrivData priv_data, uint8_t* data);

/// \brief Initializes generation and sends the very first sample.
/// \param dev - Device instance to start.
/// \note All other samples and status changes are made inside timer and DMA (TC) IRQs.
static inline void spidac_send_first_sample(PSPIDACInstance dev);

SPIDAC_FW_DEFAULT_VALUES
SPIDAC_FW_BUFFERS

/// \brief Global array that stores all virtual SPIDAC devices configurations.
volatile SPIDACInstance g_spidac_devs[] = SPIDAC_FW_DEV_DESCRIPTOR;

/// \brief Common TX DMA IRQ handler
/// \param index - index of the virtual device
/// \note  We don't disable irq here because priority of DMA request is higher than priority of the DAC timer.
///        Moreover DMA TX handler doesn't access to the DACDev status value. So it is safe to access variables
///        from the private device data from here.
void SPIDAC_COMMON_TX_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIDAC_DEVICE_COUNT);

    PSPIDACInstance dev = (PSPIDACInstance)g_spidac_devs+index;

    // Enable timer update interrupt generation.
    // BO: CLEAR_FLAGS(dev->timer->CR1, TIM_CR1_UDIS);
    dev->timer->CR1 = TIM_CR1_CEN;

    // Clear TX DMA interrupt pending bit (otherwise it will be called again once handler is returned)
    dev->dma->IFCR = dev->dma_tx_it;

    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);
    SPI_TypeDef* spi = dev->spi;

    // Increment sample pointer
    priv_data->sample_ptr += priv_data->status->sampling.phase_increment;

    // Check for sample pointer overflow
    if (priv_data->sample_ptr >= priv_data->sample_ptr_end) {
        // It must be equal, otherwise either buffer or phase_increment are not aligned.
        assert_param(priv_data->sample_ptr==priv_data->sample_ptr_end);
        priv_data->sample_ptr = priv_data->sample_ptr_start;
        priv_data->status->status = priv_data->phase_overflow_status;
    }

    // Wait SPI to complete transaction.
    // Note, you may try to comment line below (with while waiting loop) to achieve some performance improvement in some situations;
    // However this will introduce a bug (for example: LD signal will appear BEFORE SPI transaction ends). Do this very
    // carefully, check results with oscilloscope to make sure code above this comment is enough to 'wait' for SPI
    // transaction end. If not, commenting out this line will cause issues. Do not comment it out when using slow SPI speeds.
    while ((spi->SR & SPI_I2S_FLAG_BSY) != 0) {}

    // Disable SPI
    spi->CR1 = priv_data->spi_cr1_disabled;

    // LD pulse
    *(priv_data->ld_port_BSRR) = dev->ld_bit_mask;
    *(priv_data->ld_port_BRR) = dev->ld_bit_mask;
}
SPIDAC_FW_TX_DMA_IRQ_HANDLERS

/// \brief Common TIMER IRQ handler
/// \param index - index of the virtual device
void SPIDAC_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<SPIDAC_DEVICE_COUNT);
    PSPIDACInstance dev       = (PSPIDACInstance)g_spidac_devs+index;
    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);

    // Restart timer, but do not enable interrupt yet (will be enabled in DMA complete IRQ handler)
    dev->timer->SR  = (uint16_t)~TIM_IT_Update;
    dev->timer->CR1 = TIM_CR1_CEN | TIM_CR1_UDIS; // DISABLE UPDATE INTERRUPT GENERATION BECAUSE OF setting EGR->UG
    dev->timer->ARR = priv_data->status->sampling.period; // Note, no need to change prescaler value.
    dev->timer->CNT = 0;
    dev->timer->EGR = TIM_PSCReloadMode_Immediate;

    if (priv_data->status->status == STARTED) {
        // Enable SPI
        dev->spi->CR1 = priv_data->spi_cr1_enabled;

        // Restart DMA
        dev->tx_dma_channel->CCR   = priv_data->dma_ccr_disabled;
        dev->tx_dma_channel->CMAR  = (uint32_t)priv_data->sample_ptr;
        dev->tx_dma_channel->CNDTR = (uint32_t)dev->frames_per_sample;
        dev->tx_dma_channel->CCR   = priv_data->dma_ccr_enabled;
    } else {
        switch (priv_data->status->status) {
            case STARTING:
                priv_data->status->status = STARTED;
                spidac_send_first_sample(dev);
                break;

            case STOPPING:
                priv_data->status->status = RESETTING;
                spidac_set_default_sample(priv_data, NULL);

                // Setup timer to trigger as soon as possible (interrupt is already disabled above)
                dev->timer->SR = (uint16_t)~TIM_IT_Update; // WHY???
                dev->timer->PSC = 0;
                dev->timer->ARR = 1;
                dev->timer->CNT=0;
                dev->timer->EGR = TIM_PSCReloadMode_Immediate;

                // Enable SPI
                dev->spi->CR1 |= (uint16_t)0x0040;

                // Start DMA
                dev->tx_dma_channel->CCR &= (~DMA_CCR1_EN);
                dev->tx_dma_channel->CMAR = (uint32_t)priv_data->sample_ptr;
                dev->tx_dma_channel->CNDTR = (uint32_t)dev->frames_per_sample;
                dev->tx_dma_channel->CCR |= DMA_CCR1_EN;
                break;

            case RESETTING:
                // We disabled timer update generation above, re-enable it.
                CLEAR_FLAGS(dev->timer->CR1, TIM_CR1_UDIS); // ENABLE UPDATE INTERRUPT GENERATION

                // Disable timer
                timer_disable(dev->timer, dev->timer_irqn);

                // Initialize the last default transaction
                spidac_shutdown(dev);
                break;
            default:
                assert_param(0);
        }
    }
}
SPIDAC_FW_TIMER_IRQ_HANDLERS

/// @}

void spidac_init_vdev(volatile SPIDACInstance* dev, uint16_t index) {
    assert_param( dev->buffer_size > 0 );

    volatile PDeviceContext devctx = (volatile PDeviceContext)&(dev->dev_ctx);
    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);


    // There should be at least one sample
    assert_param(dev->buffer_size >= sizeof(SPIDACStatus) + dev->frames_per_sample * SPIDAC_FRAME_SIZE(dev));
    priv_data->sample_size = (dev->frame_size+1) * dev->frames_per_sample;
    priv_data->max_sample_buffer_size = dev->buffer_size - sizeof(SPIDACStatus) - priv_data->sample_size;
    priv_data->sample_buffer_size = 0;
    priv_data->status             = (PSPIDACStatus)dev->buffer;
    priv_data->status->status     = SHUTDOWN;
    priv_data->status->repeat_count = 0;

    priv_data->default_sample_base = dev->buffer + sizeof(SPIDACStatus);
    memcpy((void*)priv_data->default_sample_base, (void*)dev->default_values, priv_data->sample_size);
    priv_data->sample_buffer_base = dev->buffer + sizeof(SPIDACStatus) + priv_data->sample_size;
    priv_data->sample_ptr = NULL; // Must be set immediately before operation
    priv_data->sample_ptr_end = NULL; // Must be set immediately before operation

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
        priv_data->ld_port_BSRR = &(priv_data->dummy_register);
        priv_data->ld_port_BRR = &(priv_data->dummy_register);
    }

    memset((void*)devctx, 0, sizeof(DeviceContext));
    devctx->device_id    = dev->dev_id;
    devctx->dev_index    = index;
    devctx->buffer       = (uint8_t*)priv_data->status;
    devctx->bytes_available = sizeof(SPIDACStatus);
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
    spidac_stop(dev);

    // Pre-initialize SPI TX DMA ----------------------------------------------------------------------------
    DMA_DeInit(dev->tx_dma_channel);
    priv_data->dma_tx_preinit.DMA_PeripheralBaseAddr = (uint32_t) &(dev->spi->DR);
    priv_data->dma_tx_preinit.DMA_MemoryBaseAddr = (uint32_t) priv_data->sample_buffer_base;
    priv_data->dma_tx_preinit.DMA_DIR = DMA_DIR_PeripheralDST;
    priv_data->dma_tx_preinit.DMA_BufferSize = dev->frames_per_sample;
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
        volatile SPIDACInstance* dev = (volatile SPIDACInstance*)g_spidac_devs+i;
        PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);
        spidac_init_vdev(dev, i);

        // set default value
        spidac_set_default_sample(priv_data, NULL);
        spidac_start(dev);
    }
}

void spidac_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(cmd_byte);
    volatile SPIDACInstance* dev = (volatile SPIDACInstance*)g_spidac_devs + devctx->dev_index;
    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);
    uint8_t res = COMM_STATUS_OK;
    SPIDAC_COMMAND command = cmd_byte & COMM_CMDBYTE_DEV_SPECIFIC_MASK;

    if (command==START && length == sizeof(SPIDACSampling)) {
        spidac_set_sampling_buffer(priv_data, (PSPIDACSampling)data, 1);
        res = spidac_start(dev);
    } else if (command==START_PERIOD && length == sizeof(SPIDACSampling)) {
        spidac_set_sampling_buffer(priv_data, (PSPIDACSampling)data, 0);
        res = spidac_start(dev);
    } else if (command==SETDEFAULT && length == priv_data->sample_size) {
        spidac_set_default_sample(priv_data, data);
        res = spidac_start(dev);
    } else if (command==DATA && length <= priv_data->max_sample_buffer_size) {
        res = spidac_data(dev, data, length);
    } else if (command==STOP) {
        res = spidac_stop(dev);
    } else {
        res = COMM_STATUS_FAIL;
    }

    comm_done(res);
}

void spidac_read_done(uint8_t device_id, uint16_t length) {
    volatile PDeviceContext devctx = comm_dev_context(device_id);
    volatile SPIDACInstance* dev = g_spidac_devs + devctx->dev_index;

    UNUSED(dev);
    UNUSED(length);

    comm_done(0);
}

uint8_t spidac_stop(PSPIDACInstance dev) {
    uint8_t res = COMM_STATUS_FAIL;

    DISABLE_IRQ
    if (dev->priv_data.status->status == STARTED) {
        dev->priv_data.status->status = STOPPING;
        ENABLE_IRQ
        res = COMM_STATUS_OK;
    } else {
        ENABLE_IRQ
    }

    return res;
}

uint8_t spidac_data(PSPIDACInstance dev, uint8_t* data, uint16_t length) {
    volatile SPIDACPrivData* priv_data = &(dev->priv_data);

    if (priv_data->status->status != SHUTDOWN) {
        return COMM_STATUS_FAIL;
    }

    if (length % priv_data->sample_size != 0) {
        return COMM_STATUS_FAIL; // Unaligned buffer
    }

    assert_param(COMM_BUFFER_LENGTH >= priv_data->max_sample_buffer_size); // Make sure we have configured it right. Check it in python. <!CHECKIT!>
    memcpy((void*)(priv_data->sample_buffer_base), data, length);
    priv_data->sample_buffer_size = length;

    return COMM_STATUS_OK;
}

uint8_t spidac_start(PSPIDACInstance dev) {
    uint8_t res = COMM_STATUS_FAIL;
    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);
    PSPIDACStatus status = (PSPIDACStatus)&(priv_data->status->status);

    if (priv_data->sample_ptr==priv_data->sample_ptr_end) {
        goto done;  // no data
    }

    DISABLE_IRQ
    if (status->status == SHUTDOWN) {
        status->status = STARTING;
        ENABLE_IRQ

        timer_start(dev->timer,
                    0,  // Minimal prescaller is used
                    1,    // Minimal period is used
                    dev->timer_irqn,
                    IRQ_PRIORITY_DAC_TIMER);

        res = COMM_STATUS_OK;
    } else {
        ENABLE_IRQ
    }
done:
    return res;
}

void spidac_shutdown(PSPIDACInstance dev) {
    volatile SPIDACPrivData* priv_data = &(dev->priv_data);

    DISABLE_IRQ
    // Disable DMA
    DMA_DeInit(dev->tx_dma_channel);

    // Disable SPI
    while ((dev->spi->SR & SPI_I2S_FLAG_BSY) != 0) {}
    assert_param(SPI_I2S_GetFlagStatus(dev->spi, SPI_I2S_FLAG_BSY)==RESET);
    SPI_Cmd(dev->spi, DISABLE);

    // Handle statuses
    priv_data->status->status = SHUTDOWN;
    priv_data->sample_ptr = NULL;
    priv_data->sample_ptr_end = NULL;
    ENABLE_IRQ
}

static inline void spidac_set_sampling_buffer(PSPIDACPrivData priv_data, PSPIDACSampling sampling, uint8_t continuous) {
    priv_data->sample_ptr_start = priv_data->sample_buffer_base;
    priv_data->sample_ptr = priv_data->sample_ptr_start;
    priv_data->sample_ptr_end = priv_data->sample_ptr_start + priv_data->sample_buffer_size;
    priv_data->phase_overflow_status = continuous!=0 ? STARTED : STOPPING;
    priv_data->status->sampling.period = sampling->period;
    priv_data->status->sampling.prescaler = sampling->prescaler;
    priv_data->status->sampling.phase_increment = sampling->phase_increment;
}

static inline void spidac_set_default_sample(PSPIDACPrivData priv_data, uint8_t* data) {
    priv_data->sample_ptr_start = priv_data->default_sample_base;
    priv_data->sample_ptr = priv_data->sample_ptr_start;
    priv_data->sample_ptr_end = priv_data->sample_ptr_start + priv_data->sample_size;
    priv_data->phase_overflow_status = RESETTING;
    priv_data->status->sampling.period = 1;
    priv_data->status->sampling.prescaler = 0;
    priv_data->status->sampling.phase_increment = priv_data->sample_size;
    if (data != NULL) {
        memcpy((void *) priv_data->sample_ptr_start, data, priv_data->sample_size);
    }
}

static inline void spidac_send_first_sample(PSPIDACInstance dev) {
    PSPIDACPrivData priv_data = (PSPIDACPrivData)&(dev->priv_data);

    assert_param(priv_data->sample_ptr_start != NULL);
    assert_param(priv_data->sample_ptr != NULL);
    assert_param(priv_data->sample_ptr_end != NULL);

    priv_data->dma_tx_preinit.DMA_MemoryBaseAddr = (uint32_t)priv_data->sample_ptr;

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

#endif
