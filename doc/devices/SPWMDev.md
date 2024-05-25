# <p align="center">SPWMDev</p>
<p align="center"><img src="../images/hlek.svg"></p>


SPWMDev provides software PWM. It means that hardware PWM is not used, because of less flexible pin usage. This implementation use just one timer, and is capable to utilize all gpio outputs. However there is a price: hardware implementation is more accurate, stable, reliable and wont spent execution time in interrupt handler. Use it if you need something that doesn't require all the benefits of hardware PWM.

```
"SPWMCustomizer": {
    "spwm" : {
        "dev_id" : 7,
        "prescaler" : 21,
        "description" : {
            "L0" : {
                "type" : "GPIO_Mode_Out_PP",
                "gpio" : "PB_1",
                "default" : 1
            }
        },
        "requires" :  {"timer" :  "TIM4"}
    }
}
```
| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"description"` | This object contain descriptions of SPWMDev outputs | Object | Yes |
| `"prescaler"` | Timer prescaller value (see below) | Number | Yes |
| `"L0"` | Name of the output. Object that stores description for this specific output. | Object | Yes |
| `"type"` | Output pin configuration. May not be input! | [Pin configuration](#GPIO-pin-configuration-table) | Yes |
| `"gpio"` | GPIO pin to be used as output | `"gpio"` | Yes
| `"default"` | Default value for the output. Default values may be either on or off| 0 or 1 | Yes |
| `"requires"` | This object contain timer description to be used by SPWMDev. | Object | Yes |

`"Prescaller"` specifies divider for timer clock. Greater values allow to get longer PWM pulse duration, lower values decrease it. Timer counter clock frequency is described by the formula:

$$ F_{cnt} = \dfrac{F_{t}}{prescaler+1} $$


Where $ F_{cnt} $ is a counter clock frequency, $ F_{t} $ source timer clock, which is set in this project equal to the maximum MCU frequency $ F_{mcu}=72MHz $. In this case prescaller is 1, therefore counter clock frequency is $ F_{cnt}=36MHz $. Refer to MCU datasheet in order to get more information regarding timer clocking.

Note, `"default"` value doesn't allow to set something between on and off. This is made intentionally to protect hardware from possible mistakes (like unintentional MCU reset).

Example:

Let's say there is BLDC motor from old HDD and ESC motor driver which is regulated with PWM. The goal is to control motor from software. Most of the ESC drivers require PWM pulse frequency equal to 50Hz. The formula above may be transformed as:


$$ prescaller = \dfrac{F_{t}}{65536\cdot F_{pwm}}-1 $$

where $ F_{pwm}=50Hz $, $ F_{t}=72MHz $. In this case $ prescaller=21 $. Note, many ESC motor drivers require 5V PWM pulses, however STM32F103x produce 3.2V pulses, so some logical level conversion may be required in this case.