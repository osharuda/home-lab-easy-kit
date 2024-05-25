# <p align="center">DESKDev</p>
<p align="center"><img src="../images/hlek.svg"></p>

DESKDev is a virtual device with four buttons for "up", "down", "left", "right" and one encoder. It's purpose to organize very simple menu control in conjunction with LCD1602ADev. Example JSON configuration for `DeskDevCustomizer` key describing DESKDev is given below.

```
"DeskDevCustomizer" : {
    "controls" : {
        "dev_id" : 1,
        "requires" : {  
            "up"      : { "gpio" : "PA_3" },
            "down"    : { "gpio" : "PA_2" },
            "left"    : { "gpio" : "PA_5" },
            "right"   : { "gpio" : "PA_4" },
            "encoder" : {
                "A" : { "gpio" : "PA_7"},
                "B" : {"gpio" : "PA_6"}
            }
       }
  }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, from 1 to 16 | Yes |
| `"requires"` | This object contain description of peripherals required. | Object | Yes |
| `"up"` | GPIO pin for "up" button | `"gpio"` | Yes |
| `"down"` | GPIO pin for "down" button | `"gpio"` | Yes |
| `"left"` | GPIO pin for "left" button | `"gpio"` | Yes |
| `"right"` | GPIO pin for "right" button | `"gpio"` | Yes |
| `"A"` | GPIO pin for "A" line of the encoder | `"gpio"` | Yes |
| `"B"` | GPIO pin for "B" line of the encoder | `"gpio"` | Yes |


All buttons must be normally opened, so when corresponding button is pushed (pin is pulled up), MCU will trigger an external interrupt. One button terminal must be connected to the corresponding pin, another terminal should be grounded.

<p align="center"><img src="../../doxygen/images/under_construction.png"></p>