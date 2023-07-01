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
 *   \brief ADC (Analog to Digital Convertor) device C source file.
 *   \author Oleh Sharuda
 */

#include <string.h>
#include "fw.h"
#include "utools.h"
#include "i2c_bus.h"
#include "adcdev.h"

#ifdef ADCDEV_DEVICE_ENABLED

/// \addtogroup group_adc_dev
/// @{

ADCDEV_FW_BUFFERS
ADCDEV_FW_MEASUREMENT_BUFFERS
ADCDEV_FW_CHANNELS
ADCDEV_FW_SAMPLE_TIME_BUFFERS
ADCDEV_FW_ACCUMULATOR_BUFFERS

/// \brief Global array that stores all virtual ADCDev devices configurations.
volatile ADCDevFwInstance g_adc_devs[] = ADCDEV_FW_DEV_DESCRIPTOR;
/// @}

//---------------------------- FORWARD DECLARATIONS (COMMON FUNCTIONS) ----------------------------
//region FORWARD DECLARATIONS
/// \brief Starts ADC sampling.
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \param cmddata - command structure passed in by software (number of samples).
void adc_start(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata, volatile ADCDevCommand* cmddata);

/// \brief Suspends ADC sampling.
/// \param dev - device instance firmware configuration.
/// \note This function suspends sampling. To resume sampling either \ref adc_continue_dma_sampling (for DMA mode) or
///       \ref adc_continue_int_sampling (for ADC interrupt mode) should be called. Typically this calls will be made in
///       context of timer IRQ handler. It is used to temporary disable sampling between two sampling points.
/// \note This function doesn't affect ADC device status ( \ref ADCDevFwPrivData::status ).
static inline void adc_suspend(volatile ADCDevFwInstance* dev);

/// \brief Stops ADC sampling.
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \note This function stops sampling. Stop means timer is disabled, ADC is suspended, DMA/ADC interrupt is disabled.
///       In this sampling is stopped, to start it again \ref adc_start should be called.
/// \note This function affects ADC device status ( \ref ADCDevFwPrivData::status ).
void adc_stop(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata);

/// \brief Resets ADC device completely.
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \note Does complete device data reset. Is used for initial initialization only.
void adc_reset(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata);

/// \brief Resets ADC peripherals.
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \note Resets every peripheral related to ADC sampling: timer, ADC, DMA, device status ( \ref ADCDevFwPrivData::status ).
void adc_reset_peripherals(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata);

/// \brief Resets ADC sampled data (in circular buffer)
/// \param dev - device instance firmware configuration.
/// \note May be called during sampling.
static inline void adc_reset_circ_buffer(volatile ADCDevFwInstance* dev);

/// \brief Finalizes current sample.
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \param circ_buffer - pointer to the device circular buffer.
/// \note Finalization does the following:
///       1. Averages and put sampled data into circular buffer.
///       2. Clears \ref ADCDEV_STATUS_SAMPLING flag to control
///       3. Detects if sampling should be stopped (all requested samples are made or circular buffer is overflown).
void adc_complete(volatile ADCDevFwInstance *dev, volatile ADCDevFwPrivData* pdata, volatile PCircBuffer circ_buffer);



/// \brief Configures sampling (frequency and sampling time (sample and hold ADC circuit).
/// \param dev - device instance firmware configuration.
/// \param pdata - device private data.
/// \param cfgdata - Structure with parameters. Note this is a variable length structure.
/// \param cfgdata_size - Size of the cfgdata structure.
void adc_configure(volatile ADCDevFwInstance* dev,
                   volatile ADCDevFwPrivData* pdata,
                   volatile ADCDevConfig* cfgdata,
                   uint16_t cfgdata_size);

/// \brief Initializes ADC device channels and corresponding GPIO.
/// \param dev - device instance firmware configuration.
void adc_init_channels(volatile ADCDevFwInstance* dev);

//---------------------------- FORWARD DECLARATIONS (DMA MODE FUNCTIONS) ----------------------------

/// \brief ADC DMA mode specific part for \ref adc_reset_peripherals.
/// \param d - device instance firmware configuration (requires casting).
/// \param p - device private data (requires casing).
void adc_dma_reset(volatile void* d, volatile void* p);

/// \brief ADC DMA mode specific function to resume ADC sampling.
/// \param d - device instance firmware configuration (requires casting).
/// \param p - device private data (requires casing).
void adc_continue_dma_sampling(volatile void* d, volatile void* p);

//---------------------------- FORWARD DECLARATIONS (ADC INTERRUPT MODE FUNCTIONS) ----------------------------
/// \brief ADC interrupt mode specific part for \ref adc_reset_peripherals.
/// \param d - device instance firmware configuration (requires casting).
/// \param p - device private data (requires casing).
void adc_continue_int_sampling(volatile void* d, volatile void* p);

/// \brief ADC interrupt mode specific function to resume ADC sampling.
/// \param d - device instance firmware configuration (requires casting).
/// \param p - device private data (requires casing).
void adc_int_reset(volatile void* d, volatile void* p);
//endregion

//---------------------------- TIMER IRQ HANDLER ----------------------------
void ADC_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<ADCDEV_DEVICE_COUNT);
    volatile ADCDevFwInstance* dev = g_adc_devs+index;
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);

    // <CHECKIT> Do we actually need this check?
    if (TIM_GetITStatus(dev->timer, TIM_IT_Update) == RESET) {
        return;
    }
    TIM_ClearITPendingBit(dev->timer, TIM_IT_Update);

    // Set sampling flag
    if (IS_SET(pdata->status, (uint16_t)ADCDEV_STATUS_SAMPLING)) {
        // Two subsequent interrupts have triggered while sampling and ADC/INT interrupt didn't trigger yet.
        // It means sampling is too fast, we have to stop sampling and set error code.
        SET_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_TOO_FAST);
        adc_stop(dev, pdata);
    } else {
        // Start new conversion
        SET_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_SAMPLING);
        pdata->adc_continue_sampling_ptr(dev, pdata);
    }
}
ADCDEV_FW_TIMER_IRQ_HANDLERS

//---------------------------- IMPLEMENTATION (COMMON FUNCTIONS) ----------------------------
//region COMMON FUNCTIONS

static inline void adc_suspend(volatile ADCDevFwInstance* dev) {
    // Special note: it seems like SR may not be set until conversion is stopped. ADC is configured in scan mode (group),
    // therefor we have these options:
    // 1. Disable ADC. It is ineffective because this requires ADC configuration, calibration, etc.
    // 2. Disable scan mode, which should be more efficient. Below this option is made.
    CLEAR_FLAGS(dev->adc->CR2, (uint32_t)(ADC_CR2_FLAG_EXT_TRIG | ADC_CR2_FLAG_DMA | ADC_CR2_FLAG_CONT));
    CLEAR_FLAGS(dev->adc->CR1, (uint32_t)(ADC_CR1_FLAG_SCAN));
    dev->adc->SR = 0;
}

void adc_stop(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata) {
    DISABLE_IRQ
    if (IS_SET(pdata->status, (uint16_t)ADCDEV_STATUS_STARTED)) {
        adc_suspend(dev);
        timer_disable_no_irq(dev->timer, dev->timer_irqn);
        NVIC_DisableIRQ(dev->scan_complete_irqn);
        CLEAR_FLAGS(pdata->status, (uint16_t) (ADCDEV_STATUS_STARTED | ADCDEV_STATUS_SAMPLING));
    }
    ENABLE_IRQ
}


void adc_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    uint16_t index = comm_dev_context(cmd_byte)->dev_index;
    volatile ADCDevFwInstance* dev    = (volatile ADCDevFwInstance*)g_adc_devs+index;
    volatile ADCDevFwPrivData* pdata  = &(dev->privdata);
    volatile ADCDevConfig* cfgdata    = (volatile ADCDevConfig*)data;
    volatile ADCDevCommand* startdata = (volatile ADCDevCommand*)data;

    DISABLE_IRQ
    uint8_t adc_started = IS_SET(pdata->status, (uint16_t)ADCDEV_STATUS_STARTED);
    ENABLE_IRQ

    uint8_t status = COMM_STATUS_FAIL;
    uint8_t start_adc = CHECK_FLAGS(cmd_byte, COMM_CMDBYTE_DEV_SPECIFIC_MASK, ADCDEV_START);
    uint8_t stop_adc = ( adc_started != 0 ) && ( ( cmd_byte & (ADCDEV_STOP | ADCDEV_CONFIGURE)) != 0 );
    uint8_t reset_adc = stop_adc;

    // Check configure params are OK
    if ( (cmd_byte & ADCDEV_CONFIGURE) &&
         ( (length < sizeof(ADCDevConfig)) || (length > (sizeof(ADCDevConfig) + dev->input_count)) ||
           (cfgdata->measurements_per_sample > dev->max_measurement_per_sample) ||
           (cfgdata->measurements_per_sample < 1))) {
        goto done;
    }

    // Check start params are OK
    if (start_adc && ( (length!=sizeof(ADCDevCommand)) || adc_started)) {
        goto done;
    }

    if (stop_adc) {
        adc_stop(dev, pdata);
    }

    if (cmd_byte & ADCDEV_CONFIGURE) {
        adc_configure(dev, pdata, cfgdata, length);

        // A special note: Configuration must re-initialize hardware despite ADC was stopped or not.
        adc_reset_peripherals(dev, pdata);
        reset_adc = 0;
    }

    if (reset_adc) {
        // A special note: if ADC sampling is aborted, ADC peripherals may stay in undetermined state, therefore
        // peripherals reset is required if sampling was stopped by software.
        adc_reset_peripherals(dev, pdata);
    }

    if (cmd_byte & ADCDEV_CLEAR) {
        adc_reset_circ_buffer(dev);
    }

    if (start_adc) {
        adc_start(dev, pdata, startdata);
    }

    status = COMM_STATUS_OK;
done:
    comm_done(status);
}

void adc_configure(volatile ADCDevFwInstance* dev,
                   volatile ADCDevFwPrivData* pdata,
                   volatile ADCDevConfig* cfgdata,
                   uint16_t cfgdata_size) {
    pdata->prescaller = cfgdata->timer_prescaller;
    pdata->period     = cfgdata->timer_period;
    pdata->measurement_per_sample = cfgdata->measurements_per_sample;

    uint16_t st_num = cfgdata_size - sizeof(ADCDevConfig);
    uint16_t ch=0;

    // Configured values
    for (;ch<st_num; ch++) {
        dev->sample_time_buffer[ch] = cfgdata->channel_sampling[ch];
    }

    // Default values
    for (;ch<dev->input_count; ch++) {
        dev->sample_time_buffer[ch] = dev->channels[ch].sample_time;
    }
}

void adc_start( volatile ADCDevFwInstance* dev,
                volatile ADCDevFwPrivData* pdata,
                volatile ADCDevCommand* cmddata) {
    if (cmddata->sample_count == 0) {
        SET_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_UNSTOPPABLE);
        pdata->samples_left = 0;
    } else {
        CLEAR_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_UNSTOPPABLE);
        pdata->samples_left = cmddata->sample_count;
    }

    // Set virtual device status
    assert_param(IS_CLEARED(pdata->status, (uint16_t)ADCDEV_STATUS_STARTED | ADCDEV_STATUS_SAMPLING));
    SET_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_STARTED);

    pdata->measurement_count = dev->input_count * pdata->measurement_per_sample;
    pdata->current_measurement = dev->measurement_buffer;
    pdata->end_measurement = dev->measurement_buffer + pdata->measurement_count;
    NVIC_EnableIRQ(dev->scan_complete_irqn);
    timer_start_ex(dev->timer,
                   dev->privdata.prescaller,
                   dev->privdata.period,
                   dev->timer_irqn,
                   dev->privdata.interrupt_priority,
                   1);
}

void adc_read_done(uint8_t device_id, uint16_t length) {
    uint16_t index = comm_dev_context(device_id)->dev_index;
    volatile ADCDevFwInstance* dev = g_adc_devs+index;
    volatile PCircBuffer circbuf = (volatile PCircBuffer)&(dev->circ_buffer);
    uint8_t status = circbuf_get_ovf(circbuf) ? COMM_STATUS_OVF : COMM_STATUS_OK;
    circbuf_stop_read(circbuf, length);
    circbuf_clear_ovf(circbuf);
    comm_done(status);
}

static inline void adc_reset_circ_buffer(volatile ADCDevFwInstance* dev) {
    volatile PCircBuffer circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);
    DISABLE_IRQ
    circbuf_init(circ_buffer, (uint8_t *)dev->buffer, dev->buffer_size);
    circbuf_init_block_mode_no_irq(circ_buffer, dev->sample_block_size);
    circbuf_init_status(circ_buffer,
                        (volatile uint8_t*)&(dev->privdata.status),
                        STRUCT_MEMBER_SIZE(ADCDevFwPrivData,status));
    dev->dev_ctx.bytes_available = 0;
    ENABLE_IRQ
}

void adc_init() {
    // Set ADC clock: ADCCLK = 12 MHz (72Mhz/6)
    START_PIN_DECLARATION
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    for (uint16_t i=0; i<ADCDEV_DEVICE_COUNT; i++) {
        volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)g_adc_devs+i;
        volatile ADCDevFwPrivData* pdata = &(dev->privdata);
        volatile PDeviceContext devctx = (volatile PDeviceContext)&(dev->dev_ctx);
        memset((void*)devctx, 0, sizeof(DeviceContext));

        // Initialize device context structure
        dev->dev_ctx.device_id = dev->dev_id;
        dev->dev_ctx.dev_index = i;
        dev->dev_ctx.on_command = adc_dev_execute;
        dev->dev_ctx.on_read_done = adc_read_done;
        dev->dev_ctx.circ_buffer = (volatile PCircBuffer)( &(dev->circ_buffer) );

        // Initialize circular buffer
        adc_reset_circ_buffer(dev);


        // Initialize for the first time.
        // Note: ADC channels are not initialized, but gpio and sample_time_buffer are configured.
        for (uint16_t ch=0; ch<dev->input_count; ch++) {
            volatile  ADCDevFwChannel* channel_data = (volatile  ADCDevFwChannel*)(dev->channels+ch);
            if ( (channel_data->channel == ADC_Channel_TempSensor) ||
                 (channel_data->channel == ADC_Channel_Vrefint))
                continue;

            dev->sample_time_buffer[ch] = dev->channels[ch].sample_time;
            DECLARE_PIN(channel_data->port, channel_data->pin, GPIO_Mode_AIN)
        }

        // Reset hardware and internal structures
        adc_reset(dev, pdata);

        // Register device context
        comm_register_device(&dev->dev_ctx);
    }
}

void adc_reset_peripherals(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata) {
    ADC_InitTypeDef adcinit;

    // Private data must be initialized.
    assert_param(pdata!=0);
    assert_param((pdata->adc_continue_sampling_ptr==adc_continue_int_sampling) ||
                 (pdata->adc_continue_sampling_ptr==adc_continue_dma_sampling));

    assert_param((pdata->interrupt_priority==IRQ_PRIORITY_ADC_LO_PRIO) ||
                 (pdata->interrupt_priority==IRQ_PRIORITY_ADC_HI_PRIO));

    assert_param((pdata->adc_hw_reset_ptr==adc_int_reset) ||
                 (pdata->adc_hw_reset_ptr==adc_dma_reset));

    // Must be stopped
    assert_param(IS_CLEARED(pdata->status, ADCDEV_STATUS_STARTED));

    TIM_DeInit(dev->timer);
    timer_disable_ex(dev->timer);
    ADC_DeInit(dev->adc);
    CLEAR_FLAGS(dev->adc->CR2, (uint32_t)ADC_CR2_FLAG_ADON); // Disable ADC

    pdata->measurement_count = dev->input_count * pdata->measurement_per_sample;
    pdata->current_measurement = dev->measurement_buffer;
    pdata->end_measurement = pdata->current_measurement + pdata->measurement_count;

    // Init ADC
    adcinit.ADC_Mode = ADC_Mode_Independent;
    adcinit.ADC_ScanConvMode = ENABLE;
    adcinit.ADC_ContinuousConvMode = ENABLE;
    adcinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adcinit.ADC_DataAlign = ADC_DataAlign_Right;
    adcinit.ADC_NbrOfChannel = dev->input_count;
    ADC_Init(dev->adc, &adcinit);

    adc_init_channels(dev);

    SET_FLAGS(dev->adc->CR2, (uint32_t)ADC_CR2_FLAG_ADON); // Enable ADC

    // ADC calibration:
    delay_loop(12);    // <CHECKIT> - Make sure it is long enough, it should 12 CPU clocks.

    ADC_ResetCalibration(dev->adc);
    while(ADC_GetResetCalibrationStatus(dev->adc));

    ADC_StartCalibration(dev->adc);
    while(ADC_GetCalibrationStatus(dev->adc));

    pdata->status = 0;
    pdata->adc_hw_reset_ptr(dev, pdata);
}

void adc_reset(volatile ADCDevFwInstance* dev, volatile ADCDevFwPrivData* pdata) {
    TIM_DeInit(dev->timer);
    timer_disable_ex(dev->timer);

    // Init private data (by default zeroed)
    memset((void*)pdata, 0, sizeof(ADCDevFwPrivData));

    // By default 1 sample per 1 seconds
    pdata->prescaller = 1098;
    pdata->period = 65513;
    pdata->measurement_count = 1;
    pdata->measurement_per_sample = 1;   // by default one sample
    // Reset function pointers
    if (ADC_INT_MODE(dev)) {
        // Interrupt mode uses lower priority. It should be used for very slow sample rates only, because it causes
        // a lot of IRQs to be handled.
        pdata->interrupt_priority = IRQ_PRIORITY_ADC_LO_PRIO;
        pdata->adc_continue_sampling_ptr = adc_continue_int_sampling;
        pdata->adc_hw_reset_ptr = adc_int_reset;
    } else {
        pdata->interrupt_priority = IRQ_PRIORITY_ADC_HI_PRIO;
        pdata->adc_continue_sampling_ptr = adc_continue_dma_sampling;
        pdata->adc_hw_reset_ptr = adc_dma_reset;
    }

    // Clear circular buffer
    adc_reset_circ_buffer(dev);
    adc_reset_peripherals(dev, pdata);
}

void adc_complete(volatile ADCDevFwInstance *dev, volatile ADCDevFwPrivData* pdata, volatile PCircBuffer circ_buffer) {
    // State sanity check
    assert_param(IS_SET(pdata->status, (uint16_t)(ADCDEV_STATUS_SAMPLING | ADCDEV_STATUS_STARTED)));
    uint8_t stop_sampling = 0;

    // Put to circular buffer.
    volatile uint16_t* block = circbuf_reserve_block(circ_buffer);
    if (block != 0) {
        // Calculate average
        memset((void*)dev->accumulator_buffer, 0, dev->input_count*sizeof(uint32_t));
        volatile uint32_t* acc_ptr_end = dev->accumulator_buffer + dev->input_count;

        for (volatile uint16_t* m_ptr = dev->measurement_buffer; m_ptr < pdata->end_measurement; ) {
            for (volatile uint32_t* acc_ptr = dev->accumulator_buffer; acc_ptr < acc_ptr_end; acc_ptr++, m_ptr++) {
                *acc_ptr += *m_ptr;
            }
        }

        for (volatile uint32_t* acc_ptr = dev->accumulator_buffer; acc_ptr < acc_ptr_end; acc_ptr++, block++) {
            *block = *acc_ptr / pdata->measurement_per_sample;
        }
        circbuf_commit_block(circ_buffer);

        // Do we need more samples?
        if (pdata->samples_left) pdata->samples_left--;
    } else {
        // Check for circular buffer overflow
        stop_sampling = 1;
    }

    stop_sampling |= IS_CLEARED(pdata->status, (uint16_t)ADCDEV_STATUS_UNSTOPPABLE) &&
                     (pdata->samples_left == 0);


    // Note: No need to disable interrupt here, because we are inside IRQ handler now
    if (stop_sampling) {
        pdata->samples_left = 0;
        adc_stop(dev, pdata);
    } else {
        CLEAR_FLAGS(pdata->status, (uint16_t)ADCDEV_STATUS_SAMPLING);
    }
}

void adc_init_channels(volatile ADCDevFwInstance* dev) {
    START_PIN_DECLARATION

    for (uint16_t ch=0; ch<dev->input_count; ch++) {
        volatile  ADCDevFwChannel* channel_data = (volatile  ADCDevFwChannel*)(dev->channels+ch);
        uint8_t channel = channel_data->channel;

        if (channel==ADC_Channel_TempSensor || channel==ADC_Channel_Vrefint) {
            ADC_TempSensorVrefintCmd(ENABLE); // Enable temp. sensor and vref sensor if required
        }  else {
            // Configure GPIO pin
            DECLARE_PIN(channel_data->port, channel_data->pin, GPIO_Mode_AIN);
        }

        ADC_RegularChannelConfig(dev->adc, channel, ch + 1, dev->sample_time_buffer[ch]);
    }
}
//endregion

//---------------------------- IMPLEMENTATION (DMA MODE FUNCTIONS) ----------------------------
// region DMA MODE FUNCTIONS

//---------------------------- DMA IRQ HANDLER ----------------------------
void ADC_COMMON_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<ADCDEV_DEVICE_COUNT);

    volatile ADCDevFwInstance* dev = g_adc_devs+index;
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);

    // Disable sampling
    adc_suspend(dev);
    adc_complete(dev, pdata, (PCircBuffer)&dev->circ_buffer);

    DMA_ClearITPendingBit(dev->dma_it);
}
ADCDEV_FW_DMA_IRQ_HANDLERS

void adc_dma_reset(volatile void* d, volatile void* p) {
    DMA_InitTypeDef dmainit;

    volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)d;
    volatile ADCDevFwPrivData* pdata = (volatile ADCDevFwPrivData*)p;

    // Init DMA
    DMA_DeInit(dev->dma_channel);
    dmainit.DMA_PeripheralBaseAddr  = dev->adc_dr_address;
    dmainit.DMA_MemoryBaseAddr      = (uint32_t)pdata->current_measurement;
    dmainit.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    dmainit.DMA_BufferSize          = pdata->measurement_count; // in data units
    dmainit.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    dmainit.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    dmainit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    dmainit.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    dmainit.DMA_Mode                = DMA_Mode_Normal;
    dmainit.DMA_Priority            = DMA_Priority_High;
    dmainit.DMA_M2M                 = DMA_M2M_Disable;
    DMA_Init(dev->dma_channel, &dmainit);

    ADC_ITConfig(dev->adc, ADC_IT_EOC, DISABLE);

    // Enable DMA channel and interrupt
    NVIC_SetPriority(dev->scan_complete_irqn, dev->privdata.interrupt_priority);
    DMA_ClearITPendingBit(dev->dma_it);
    NVIC_ClearPendingIRQ(dev->scan_complete_irqn);
    NVIC_DisableIRQ(dev->scan_complete_irqn);
    DMA_ITConfig(dev->dma_channel, DMA_IT_TC, ENABLE);

    // After reset ADC should be prepared for sampling, but it shouldn't be started
    pdata->status = 0;
    assert_param((dev->adc->CR2 & ADC_CR2_SWSTART)==0);
    assert_param((dev->adc->SR & ADC_FLAG_STRT) == 0);

    // Enable
    DMA_Cmd(dev->dma_channel, ENABLE);
    ADC_DMACmd(dev->adc , ENABLE);
}

void adc_continue_dma_sampling(volatile void* d, volatile void* p) {
    volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)d;
    volatile ADCDevFwPrivData* pdata = (volatile ADCDevFwPrivData*)p;

    // Reinitialize DMA
    pdata->current_measurement = dev->measurement_buffer;
    DMA_Cmd(dev->dma_channel, DISABLE);
    CLEAR_FLAGS(dev->dma_channel->CCR, (uint32_t)DMA_CCR1_EN);

    dev->dma_channel->CNDTR = pdata->measurement_count;
    dev->dma_channel->CMAR = (uint32_t)pdata->current_measurement;
    dev->dma_channel->CPAR = (uint32_t)dev->adc_dr_address;
    DMA_Cmd(dev->dma_channel, ENABLE);
    SET_FLAGS(dev->dma_channel->CCR, (uint32_t)DMA_CCR1_EN);

    // Start ADC
    while (dev->adc->SR) dev->adc->SR = 0;  // Wait when hardware cleans all flags. <CHECKIT> Do we need it?
    SET_FLAGS(dev->adc->CR1, (uint32_t)ADC_CR1_FLAG_SCAN);
    SET_FLAGS(dev->adc->CR2, (uint32_t)(ADC_CR2_FLAG_SWSTART | ADC_CR2_FLAG_EXT_TRIG | ADC_CR2_FLAG_DMA | ADC_CR2_FLAG_CONT));
}
//endregion

//---------------------------- IMPLEMENTATION (ADC INTERRUPT MODE FUNCTIONS) ----------------------------
//region ADC INTERRUPT MODE FUNCTIONS

//---------------------------- ADC IRQ HANDLER ----------------------------
void ADC_COMMON_ADC_IRQ_HANDLER(uint16_t index) {
    assert_param(index < ADCDEV_DEVICE_COUNT);
    volatile ADCDevFwInstance *dev = g_adc_devs + index;
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);
    volatile PCircBuffer       circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);

    // get actual value and increase current pointer.
    *pdata->current_measurement = ADC_GetConversionValue(dev->adc);
    pdata->current_measurement++;

    if (pdata->current_measurement < pdata->end_measurement) {
        adc_continue_int_sampling(dev, pdata);
    } else {
        adc_suspend(dev);
        adc_complete(dev, pdata, circ_buffer);
    }

    // Clear pending bits
    ADC_ClearITPendingBit(dev->adc, ADC_IT_EOC);
    NVIC_ClearPendingIRQ(dev->scan_complete_irqn);
}
ADCDEV_FW_ADC_IRQ_HANDLERS

void adc_int_reset(volatile void* d, volatile void* p) {
    volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)d;
    volatile ADCDevFwPrivData* pdata = (volatile ADCDevFwPrivData*)p;

    // Setup interrupt, priorities, etc.

    ADC_ITConfig(dev->adc, ADC_IT_EOC, ENABLE);

    NVIC_SetPriority(dev->scan_complete_irqn, dev->privdata.interrupt_priority);
    ADC_ClearITPendingBit(dev->adc, ADC_IT_EOC);
    NVIC_ClearPendingIRQ(dev->scan_complete_irqn);
    NVIC_DisableIRQ(dev->scan_complete_irqn);

    // After reset ADC should be prepared for sampling, but it shouldn't be started
    pdata->status = 0;
    assert_param((dev->adc->CR2 & ADC_CR2_SWSTART)==0);
    assert_param((dev->adc->SR & ADC_FLAG_STRT) == 0);
}

void adc_continue_int_sampling(volatile void* d, volatile void* p) {
    volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)d;
    UNUSED(p);

    // Start ADC
    while (dev->adc->SR) dev->adc->SR = 0;  // Wait when hardware cleans all flags. <CHECKIT> Do we need it?
    SET_FLAGS(dev->adc->CR1, (uint32_t)ADC_CR1_FLAG_SCAN);
    SET_FLAGS(dev->adc->CR2, (uint32_t)(ADC_CR2_FLAG_SWSTART | ADC_CR2_FLAG_EXT_TRIG | ADC_CR2_FLAG_CONT));
}
//endregion
#endif
