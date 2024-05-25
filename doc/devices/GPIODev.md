# <p align="center">GPIODev</p>
<p align="center"><img src="../images/hlek.svg"></p>

GPIODev virtual device makes possible to control specific MCU pins. It's possible to use them as inputs and outputs. Other pin configurations are also available. Each pin has a name, which is a key for description objects specified in `"description"`. Pins are configured during project customization, and can't be reconfigured later in run time. Example below lists two pins, one for input and one for output.

```
"GPIODevCustomizer": {
    "gpio" : {
        "dev_id": 6,
        "description" : {
            "in_0": {  "type" : "GPIO_Mode_IPU",
                       "gpio" : "PA_11"
            },
            "out_1": { "type" : "GPIO_Mode_Out_PP",
                       "gpio": "PC_13",
                       "default": "1"
            }
        }
    }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"description"` | This object contain descriptions of pins for this virtual device | Object | Yes |
| `"in_0"` and `"out_1"` | These objects contain descriptions of pins they represent | Object | Yes |
| `"type"` | Pin configuration | [Pin configuration](#GPIO-pin-configuration-table) | Yes |
| `"gpio"` | Dependency to gpio pin | `"gpio"` | Yes |
| `"default"` | Specifies default state for output pins. | 0 or 1 | Required for outputs, has no effect for inputs |