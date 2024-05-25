<p align="center"><img src="../doc/images/hlek.svg"></p>

# <p align="center">Configuration</p>

## Table of contents
1. [JSON configuration file](#JSON-configuration-file)
2. [Peripherals types table](#Peripherals-types-table)
3. [Peripherals](#Peripherals)
4. [GPIO pin configuration table](#GPIO-pin-configuration-table)

### JSON configuration file

JSON configuration file has the following structure:

```
{
    "firmware" : {
        "mcu_model" : "stm32f103",
        "i2c_bus" : {
            "clock_speed" : 100000,
            "buffer_size" : 512,
            "address" : 32,
            "requires" :  {"i2c" : "I2C2"}
        }
    },
    "devices" : {
        "DeskDevCustomizer" : { ... },
        "RTCCustomizer" : { ... },
        "IRRCCustomizer" : { ... },
        "UartProxyCustomizer" : { ... },
        "LCD1602aCustomizer" : { ... },
        "GPIODevCustomizer": { ... },
        "SPWMCustomizer": { ... },
        "ADCDevCustomizer": { ... },
        "StepMotorDevCustomizer": { ... },
        "AD9850DevCustomizer": { ... },
        "SPIDACCustomizer": { ... },
        "SPIProxyCustomizer": { ... }
    }
}
```

The first `firmware` key describes basic firmware properties.

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"mcu_model"` | MCU model being used, currently the only MCU is supported | "stm32f103" | Yes |
| `"i2c_bus"` | Describes i2c connection with micro computer and MCU. | Object, see below | Yes |
| `"clock_speed"` | I2C bus clock speed. This project was tested with standard frequency only (100kHz) | 100000 | Yes |
| `"buffer_size"` | Size of the buffer for commands being sent from microcomputer to MCU, in bytes. | Number | Yes |
| `"address"` | I2C bus address. Must be valid 7-bit i2c bus address for your project. | Number | Yes |
| `"requires"` | This object, here and after contain description of peripherals required. | Object | Yes |
| `"i2c"` | Represents I2C peripherals being used for communication | "I2C1" or "I2C2" | Yes |
| `"devices"` | Describes required functionalities. | Object | Yes |

Each functionality is represented by corresponding key/object pair under `devices` key. The only exception is INFODev - it is not described in JSON file at all and is generated automatically based on information provided. Each key under `devices` must have one of these values: `"DeskDevCustomizer"`, `"RTCCustomizer"`, `"IRRCCustomizer"`, `"UartProxyCustomizer"`, `"LCD1602aCustomizer"`, `"GPIODevCustomizer"`, `"SPWMCustomizer"`, `"ADCDevCustomizer"`, `"StepMotorDevCustomizer"`, `"AD9850DevCustomizer"`, `"SPIDACCustomizer"`, `"SPIProxyCustomizer"`. Internally they are called as customizers, to reflect their ability to customize or configure MCU firmware.

Each of these keys should have one or more key/object pairs, where key is a "name" of the device and object describes device itself. Exclusive features may be represented by single device only, non-exclusive features may have more devices described. Presence of all features is not required, so leave those customizers which are required for your project.

Note, `"firmware"` doesn't allow to select MCU frequency. For example, STM32F10x is configured to use 72MHz clock and it's not possible to change. Table below shows some important settings that can't be changed by JSON configuration file. The table below describes some configuration that may not be changed.

|   Parameter   | Description                  | Value  |
|:--------------|:-----------------------------|:-------|
|$$ F_{mcu} $$  | MCU frequency                | 72 MHz |
|$$ F_{gpio} $$ | Maximum output frequency     | 50 MHz |
|$$ D_{AHB} $$  | AHB clock divider            | 1      |
|$$ D_{APB1} $$ | APB1 clock divider           | 2      |
|$$ D_{APB2} $$ | APB2 clock divider           | 1      |

### Peripherals types table

JSON file may refer to some peripherals which are separated by these types:

| Key (Type) | Description | Possible values (for STM32F103C8T6) |
|:-----------|:------------|:------------------------------------|
|`"i2c"`| I2C interface. Currently is used for communication with software only. |"I2C1" or "I2C2" |
|`"gpio"`| General purposes input/output pin. | "PA_0" ... "PA_15", "PB_0" ... "PB_15", "PC_13" ... "PC_15"|
|`"bkp"`| Backup register | "BKP_DR1" ... "BKP_DR42"|
|`"rtc"`| Real time clock | "RTC"|
|`"timer"`| Timer<sup>[1](#ft01)</sup> | "TIM1" ... "TIM17"|
|`"adc"`| Analog to digital converter | "ADC1" or "ADC2"<sup>[2](#ft02)</sup>|
|`"adc_input"`| Analog to digital converter input |"ADC_Channel_0" ... "ADC_Channel_9", "ADC_Channel_TempSensor", "ADC_Channel_Vrefint"|
|`"usart"`| USART interface | "USART1", "USART2", "USART3"|
|`"can"`| CAN bus | "CAN1", "CAN1_REMAP"|
|`"spi"`| SPI interface | "SPI1", "SPI1_REMAP", "SPI2"|


The following table describes pin configurations available in STM32F103x. It is general across many electronic devices, however values are taken from CMSIS library.

### GPIO pin configuration table
`"gpio"` inputs/outputs may have different states as described here:
| Value | Direction | Description |
|:------|:----------|:------------|
|`"GPIO_Mode_IN_FLOATING"`| Input  | Pin is floating
|`"GPIO_Mode_IPU"`        | Input  | Pin is pulled up to VCC
|`"GPIO_Mode_IPD"`        | Input  | Pin is pulled down to GND
|`"GPIO_Mode_Out_OD"`     | Output | Open drain
|`"GPIO_Mode_Out_PP"`     | Output | Push pull

Be very careful with logic levels. Some STM32F103C8T6 pins are 5v tolerant, some not. Improper electrical connection may cause a lot of harm. If doubt, refer documentation provided by manufacturer.

The table below describes ADC inputs available in STM32F103C8T6.
