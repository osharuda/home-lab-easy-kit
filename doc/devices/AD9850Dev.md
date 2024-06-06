# <p align="center">AD9850Dev</p>
<p align="center"><img src="../images/hlek.svg"></p>


AD9850 chip (from Analog Devices) is a well accessed, not cheap DDS generator of sine signals. It allows control of signal frequency and phase up to 40MHz (depends on output LP filter).

```
"AD9850DevCustomizer": {
      "dds" : {
        "dev_id" : 2,
        "clock_frequency": "125MHz",
        "requires" : {
          "D0" :    {"gpio" : "PA_0"},
          "D1" :    {"gpio" : "PA_1"},
          "D2" :    {"gpio" : "PA_2"},
          "D3" :    {"gpio" : "PA_3"},
          "D4" :    {"gpio" : "PA_4"},
          "D5" :    {"gpio" : "PA_5"},
          "D6" :    {"gpio" : "PA_6"},
          "D7" :    {"gpio" : "PA_7"},
          "W_CLK" : {"gpio" : "PA_8"},
          "FQ_UD" : {"gpio" : "PA_9"},
          "RESET" : {"gpio" : "PA_10"}}
      }
    }
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dds"` | Name of the virtual device. | String | Yes |
| `"dev_id"` | Device id. | Number, [1, 15] | Yes |
| `"clock_frequency"` | Clock frequency used to clock AD9850. It is used to calculate frequency correctly.| Number, in MHz | Yes |
| `"requires"` | Describes peripherals required by the virtual device. | Object with `"gpio"` required pins | Yes |
| `"D0"`-`"D7"` | Data pins | `"gpio"` | Yes |
| `"W_CLK"` | Word load clock signal | `"gpio"` | Yes |
| `"FQ_UD"` | Frequency update signal | `"gpio"`| Yes |
| `"RESET"` | Reset signal | `"gpio"`| Yes |


<p align="center"><img src="../../doxygen/images/under_construction.png"></p>