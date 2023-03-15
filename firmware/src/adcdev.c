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
ADCDEV_FW_CHANNELS

/// \brief Global array that stores all virtual ADCDev devices configurations.
volatile ADCDevFwInstance g_adc_devs[] = ADCDEV_FW_DEV_DESCRIPTOR;

/// @}

//---------------------------- FORWARD DECLARATIONS ----------------------------
//void adc_start_timer(volatile ADCDevFwInstance* dev);
void adc_start_int_mode(volatile ADCDevFwInstance* dev);
void adc_singl_channel_conversion(volatile ADCDevFwInstance* dev, uint8_t channel);
void adc_start_dma_mode(volatile ADCDevFwInstance* dev);
void stop_adc(volatile ADCDevFwInstance* dev);
void init_adc_channels(volatile ADCDevFwInstance* dev);
void init_adc_device(volatile ADCDevFwInstance* dev, uint16_t index);

void adc_complete(volatile ADCDevFwInstance *dev, volatile ADCDevFwPrivData* pdata, volatile PCircBuffer circ_buffer) {
    circbuf_commit_block(circ_buffer);
    if (pdata->samples_left) pdata->samples_left--;
    if (pdata->unstoppable || pdata->samples_left) {
        pdata->start_sampling_func(dev); // start new sample or start timer which will start it later
    } else {
        pdata->started = 0;
    }
}


//---------------------------- TIMER IRQ HANDLER ----------------------------
void ADC_COMMON_TIMER_IRQ_HANDLER(uint16_t index) {
    assert_param(index<ADCDEV_DEVICE_COUNT);
    volatile ADCDevFwInstance* dev = g_adc_devs+index;

    if (TIM_GetITStatus(dev->timer, TIM_IT_Update) == RESET) {
        return;
    }

    timer_disable(dev->timer, dev->timer_irqn);

    // start new conversion if required
    if (ADC_INT_MODE(dev)) {
        adc_start_int_mode(dev);
    } else {
        adc_start_dma_mode(dev);
    }
}
ADCDEV_FW_TIMER_IRQ_HANDLERS

//---------------------------- ADC IRQ HANDLER ----------------------------
void ADC_COMMON_ADC_IRQ_HANDLER(uint16_t index) {
    assert_param(index < ADCDEV_DEVICE_COUNT);
    volatile ADCDevFwInstance *dev = g_adc_devs + index;
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);
    volatile PCircBuffer       circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);

    // get actual value
    pdata->dest_buffer[pdata->current_channel++] = ADC_GetConversionValue(dev->adc);

    if (pdata->current_channel < dev->input_count) {
        adc_singl_channel_conversion(dev, pdata->current_channel); // sample next channel
        goto done;
    }

    adc_complete(dev, pdata, circ_buffer);

done:
    return;
}
ADCDEV_FW_ADC_IRQ_HANDLERS

//---------------------------- DMA IRQ HANDLER ----------------------------
void ADC_COMMON_DMA_IRQ_HANDLER(uint16_t index) {
    assert_param(index<ADCDEV_DEVICE_COUNT);

    volatile ADCDevFwInstance* dev = g_adc_devs+index;
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);
    volatile PCircBuffer       circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);

    DMA_ClearITPendingBit(dev->dma_it);
    DMA_Cmd(dev->dma_channel, DISABLE);

    DMA_SetCurrDataCounter(dev->dma_channel, 0);     // Clear DMA counter
    adc_complete(dev, pdata, circ_buffer);
}
ADCDEV_FW_DMA_IRQ_HANDLERS



void adc_start_timer(volatile ADCDevFwInstance* dev) {
    timer_start(dev->timer,
                dev->privdata.prescaller,
                dev->privdata.period,
                dev->timer_irqn,
                IRQ_PRIORITY_ADC_TIMER);
}


void adc_singl_channel_conversion(volatile ADCDevFwInstance* dev, uint8_t channel) {
    ADC_InitTypeDef adcinit;

    assert_param(channel < dev->input_count);
    ADC_Cmd(dev->adc, DISABLE);

    adcinit.ADC_Mode = ADC_Mode_Independent;
    adcinit.ADC_ScanConvMode = ENABLE;
    adcinit.ADC_ContinuousConvMode = DISABLE;
    adcinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adcinit.ADC_DataAlign = ADC_DataAlign_Right;
    adcinit.ADC_NbrOfChannel = 1;
    ADC_Init(dev->adc, &adcinit);

    ADC_RegularChannelConfig(dev->adc, dev->channels[channel].channel, 1, dev->channels[channel].sample_time);

    // Enable ADC
    ADC_Cmd(dev->adc, ENABLE);

    // Setup priorities
    NVIC_SetPriority(dev->scan_complete_irqn, IRQ_PRIORITY_ADC);
    NVIC_EnableIRQ(dev->scan_complete_irqn);
    ADC_ITConfig(dev->adc, ADC_IT_EOC, ENABLE);

    // Enable ADC reset calibration register and wait
    ADC_ResetCalibration(dev->adc);
    while(ADC_GetResetCalibrationStatus(dev->adc));

    // Start ADC calibration and wait
    ADC_StartCalibration(dev->adc);
    while(ADC_GetCalibrationStatus(dev->adc));

    ADC_SoftwareStartConvCmd(dev->adc, ENABLE);
}

void adc_start_int_mode(volatile ADCDevFwInstance* dev) {
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);
    volatile PCircBuffer       circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);
    assert_param(pdata->samples_left!=0);

    pdata->current_channel = 0;
    pdata->dest_buffer = (volatile uint16_t*)circbuf_reserve_block(circ_buffer);

    // Start ADC
    if (pdata->dest_buffer!=0 && // buffer allocation must be ok
        (dev->privdata.unstoppable!=0 || dev->privdata.samples_left>0)) {
        pdata->stop = 0;
        pdata->started = 1;
        adc_singl_channel_conversion(dev, pdata->current_channel);
    } else {
        dev->privdata.started = 0;
    }
}

volatile uint16_t* adc_init_dma(volatile ADCDevFwInstance* dev) {
    DMA_InitTypeDef dmainit;
    volatile PCircBuffer circ_buffer = (volatile PCircBuffer)&(dev->circ_buffer);
    volatile ADCDevFwPrivData* pdata = (&dev->privdata);
    pdata->dest_buffer = (volatile uint16_t*)circbuf_reserve_block(circ_buffer);
    if (pdata->dest_buffer==0) {
        goto done;
    }

    // Init DMA
    DMA_DeInit(dev->dma_channel);
    dmainit.DMA_PeripheralBaseAddr = dev->adc_dr_address;
    dmainit.DMA_MemoryBaseAddr = (uint32_t)pdata->dest_buffer;
    dmainit.DMA_DIR = DMA_DIR_PeripheralSRC;
    dmainit.DMA_BufferSize = dev->input_count; // in data units
    dmainit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmainit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmainit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmainit.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dmainit.DMA_Mode = DMA_Mode_Circular;
    dmainit.DMA_Priority = DMA_Priority_High;
    dmainit.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(dev->dma_channel, &dmainit);

    // Enable DMA channel
    DMA_Cmd(dev->dma_channel, ENABLE);

    // Enable DMA interrupt
    NVIC_SetPriority(dev->scan_complete_irqn, IRQ_PRIORITY_DMA);
    NVIC_EnableIRQ(dev->scan_complete_irqn);
    DMA_ITConfig(dev->dma_channel, DMA_IT_TC, ENABLE);

done:
    return pdata->dest_buffer;
}

void adc_start_dma_mode(volatile ADCDevFwInstance* dev) {
    ADC_InitTypeDef adcinit;


    if ((dev->privdata.samples_left | dev->privdata.unstoppable)==0) {
        return;
    }

    ADC_Cmd(dev->adc, DISABLE);

    if (0==adc_init_dma(dev)) {
        return; // failed to allocate data
    }

    adcinit.ADC_Mode = ADC_Mode_Independent;
    adcinit.ADC_ScanConvMode = ENABLE;
    adcinit.ADC_ContinuousConvMode = DISABLE;
    adcinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adcinit.ADC_DataAlign = ADC_DataAlign_Right;
    adcinit.ADC_NbrOfChannel = dev->input_count;
    ADC_Init(dev->adc, &adcinit);

    for (uint16_t i=0; i<dev->input_count; i++) {
        volatile  ADCDevFwChannel* channel_data = (volatile  ADCDevFwChannel*)(dev->channels+i);
        uint8_t channel = channel_data->channel;

        if (channel==ADC_Channel_TempSensor || channel==ADC_Channel_Vrefint) {
            // Enable temp. sensor and vref sensor if required
            ADC_TempSensorVrefintCmd(ENABLE);
        }

        ADC_RegularChannelConfig(dev->adc, channel, i + 1, dev->channels[i].sample_time);
    }

    // Enable ADC
    ADC_Cmd(dev->adc, ENABLE);
    ADC_ITConfig(dev->adc, ADC_IT_EOC, DISABLE);
    ADC_DMACmd(dev->adc , ENABLE ) ;


    // Enable ADC reset calibration register and wait
    ADC_ResetCalibration(dev->adc);
    while(ADC_GetResetCalibrationStatus(dev->adc));

    // Start ADC calibration and wait
    ADC_StartCalibration(dev->adc);
    while(ADC_GetCalibrationStatus(dev->adc));

    // Start ADC
    dev->privdata.stop = 0;
    dev->privdata.started = 1;
    ADC_SoftwareStartConvCmd(dev->adc, ENABLE);
}

void stop_adc(volatile ADCDevFwInstance* dev) {
    assert_param(dev->privdata.started!=0);

    // Disable ADC timer
    timer_disable(dev->timer, dev->timer_irqn);

    // Set flag indicating we've stopped
    dev->privdata.stop = 1;
}


void init_adc_channels(volatile ADCDevFwInstance* dev) {
    START_PIN_DECLARATION
    for (uint16_t i=0; i<dev->input_count; i++) {
        volatile  ADCDevFwChannel* channel_data = (volatile  ADCDevFwChannel*)(dev->channels+i);
        uint8_t channel = channel_data->channel;

        if (channel==ADC_Channel_TempSensor || channel==ADC_Channel_Vrefint) {
            // Enable temp. sensor and vref sensor if required
            ADC_TempSensorVrefintCmd(ENABLE);
        } else {
            // Configure GPIO
            DECLARE_PIN(channel_data->port, channel_data->pin, GPIO_Mode_AIN);
        }
    }
}

void init_adc_device(volatile ADCDevFwInstance* dev, uint16_t index) {
    // Init circular buffer in block mode
    volatile PCircBuffer circbuf = (volatile PCircBuffer) &(dev->circ_buffer);
    circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
    circbuf_init_block_mode(circbuf, dev->sample_block_size);

    volatile PDeviceContext devctx = (volatile PDeviceContext)&(dev->dev_ctx);
    memset((void*)devctx, 0, sizeof(DeviceContext));

    devctx->device_id = dev->dev_id;
    devctx->dev_index = index;
    devctx->on_command = adc_dev_execute;
    devctx->on_read_done = adc_read_done;
    devctx->circ_buffer = circbuf;
    comm_register_device(devctx);
}

void init_adc() {
    // Disable temp. sensor and vref channels
    ADC_TempSensorVrefintCmd(DISABLE);

    for (uint16_t i=0; i<ADCDEV_DEVICE_COUNT; i++) {
        volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)g_adc_devs+i;
        init_adc_channels(dev);
        init_adc_device(dev, i);
    }
}

void adc_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    uint16_t index = comm_dev_context(cmd_byte)->dev_index;
    volatile ADCDevFwInstance* dev = (volatile ADCDevFwInstance*)g_adc_devs+index;
    volatile ADCDevFwPrivData* priv = &(dev->privdata);
    volatile ADCDevCommand* cmddata = (volatile ADCDevCommand*)data;

    if (priv->started) {
        stop_adc(dev);
    }

    if (cmd_byte & ADCDEV_RESET_DATA) {
        volatile PCircBuffer circbuf = (volatile PCircBuffer) &(dev->circ_buffer);
        circbuf_init(circbuf, (uint8_t *)dev->buffer, dev->buffer_size);
        circbuf_init_block_mode(circbuf, dev->input_count*sizeof(uint16_t));
    }

    if (length!=sizeof(ADCDevCommand)) {
        goto done;
    }

    priv->unstoppable = (cmd_byte & ADCDEV_UNSTOPPABLE) != 0;
    priv->samples_left = priv->unstoppable ? 0 : cmddata->sample_count;
    priv->prescaller = cmddata->timer_prescaller;
    priv->period = cmddata->timer_period;



    if (priv->unstoppable!=0 || priv->samples_left>0) {
        if (ADC_INT_MODE(dev)) {
            priv->start_sampling_func = (priv->prescaller | priv->period)==0 ?
                    (ADCStartSamplingFunc)adc_start_int_mode :
                    (ADCStartSamplingFunc)adc_start_timer;

            adc_start_int_mode(dev);
        } else {
            priv->start_sampling_func = (priv->prescaller | priv->period)==0 ?
                                        (ADCStartSamplingFunc)adc_start_dma_mode :
                                        (ADCStartSamplingFunc)adc_start_timer;
            adc_start_dma_mode(dev);
        }
    }

done:
    comm_done(0);
}

void adc_read_done(uint8_t device_id, uint16_t length) {
    uint16_t index = comm_dev_context(device_id)->dev_index;
    volatile ADCDevFwInstance* dev = g_adc_devs+index;
    volatile PCircBuffer circbuf = (volatile PCircBuffer)&(dev->circ_buffer);
    circbuf_stop_read(circbuf, length);
    circbuf_clear_ovf(circbuf);
    comm_done(0);
}

#endif
