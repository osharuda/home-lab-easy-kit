# <p align="center">ADCDev</p>
<p align="center"><img src="../images/hlek.svg"></p>

ADCDev virtual device establish simple access to 12-bit ADC peripherals available in STM32F10x. STM32F103C8T6 has two ADC, both of them may be used. ADC may be used in either interrupt or DMA (Direct Memory Access) mode. Current implementation doesn't give as much performance benefits as DMA may provide. This functionality shouldn't be treated as a kind of oscilloscope for quick signal measurements. Instead of this, feature is designed to be multichannel multimeter, capable to provide data logging for slow signals. STM32F10x also allows to measure internal temperature and internal reference voltage. For fast signals it is better to consider some specialized hardware.

```
"ADCDevCustomizer": {
    "adc_0" : {
        "dev_id" : 8,
        "buffer_size" : 96,
        "use_dma" : 1,
        "vref" : 3.2,
        "sample_time" : {
            "default" : "ADC_SampleTime_7Cycles5",
            "override" :  {"input_0" : "ADC_SampleTime_28Cycles5"}
        },
        "requires" :  {
            "ADC" : {"adc" : "ADC1"},
            "TIMER" : {"timer" :  "TIM2"},
            "input_0" : {"adc_input" : "ADC_Channel_0"},            
            "input_vref" : {"adc_input" : "ADC_Channel_Vrefint"},
            "input_temp" : {"adc_input" : "ADC_Channel_TempSensor"}
        }
    }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"adc_0"` | Name of the ADCDev virtual device. | String | Yes |
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"buffer_size"` | Size of the internal circular buffer. It is recommended to be multiple to the number of inputs.|  Number | Yes |
| `"use_dma"` | Instructs to use either DMA (1) or interrupt mode (0) | 0, 1 | Yes |
| `"vref"` | Reference voltage, used by software to return measurements in volts. Specify voltage that correspond to maximum ADC value<sup>[3](#ft03)</sup>. | Value in volts | Yes |
| `"sample_time"` | This object specifies sample timings for inputs. Values correspond to the ones used by CMSIS library. | Object | Yes |
| `"default"` | Default sampling time for all inputs. |  [Sampling time](#ADC-input-table) | Yes |
| `"override"` |Contains input name/sampling times to override default sample time for some inputs. | Input name/[Sampling time](#ADC-input-table) | Yes |
| `"requires"` | This object contain peripherals and inputs required by ADCDev. | Object | Yes |
| `"ADC"` | Object holding ADC peripheral used with this virtual device. Key value ("ADC") may be changed. | Object with `"adc"` | Yes |
| `"TIMER"` | Object holding timer to be used with this virtual device. Timer is required for both interrupt and DMA mode. Key value ("TIMER") may be changed.| Object with `"timer"` | Yes |
| `"input_0"` | Object holding ADC input. Key value ("input_0") specifies input name and is used in software. | Object with `"adc_input"` | No |
| `"input_vref"` | Object holding ADC internal Vref input. Key value ("input_vref") specifies input name and is used in software. | Object with `"adc_input"` | No |
| `"input_temp"` | Object holding ADC internal temperature sensor input. Key value ("input_temp") specifies input name and is used in software. | Object with `"adc_input"` | No |