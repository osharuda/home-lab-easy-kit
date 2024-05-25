# <p align="center">IRRCDev</p>
<p align="center"><img src="../images/hlek.svg"></p>

IRRCDev virtual device allows to control your experimental build with IR remote controller. It must use NEC standard. This will allow you to control your experiments from short distance, from sofa for example.

Example JSON configuration for IRRCDev:

```
"IRRCCustomizer" : {
    "irrc": {
        "dev_id": 3,
        "buffer_size" : 64,
        "requires": {"data" : {"gpio" : "PA_1"}}
    }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"buffer_size"` | Size of the circular buffer to store IR remote control commands, in bytes | Number, must be even | Yes |
| `"requires"` | This object contain description of peripherals required. | Object | Yes |
| `"data"` | Describes data input. Must contain `"gpio"` dependency. | `"gpio"` | Yes |

There are wide spectrum of special three terminal receivers (like TSOP384 series) to handle IR remote controllers optical signals. One of these, or equivalent must be connected to data pin in order to make this functionality working.

Note, there are many different types of IR remote controls. They may be different by a signal standard and by IR signal carrier frequency. In order to make your IR remote control working, the following must be true:
- IR remote control works in NEC standard. Here is a a good description of [NEC standard](https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol).
- IR signal carrier frequency of your IR remote control should match with your TSOP receiver. Read IR receiver documentation carefully. This [datasheet](https://www.vishay.com/docs/82491/tsop382.pdf) describes TSOP382x and TSOP384x families of such receivers.
