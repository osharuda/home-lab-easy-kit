# <p align="center">LCD1602ADev</p>
<p align="center"><img src="../images/hlek.svg"></p>

LCD1602ADev	virtual device allows to use simple LCD to display some short information, messages or notifications. This feature is used in conjunction with LCD1602a screens which are cheap and widely present on the market. This screen is used in 4 bit data mode to save GPIO lines. Back-light support is available as well.

```
"LCD1602aCustomizer" : {
    "lcd": {
        "dev_id" : 5,
        "welcome" : ["-=RPi Extender=-","   loading ..."],
        "requires" : {"enable" : {"gpio" : "PB_14"},
                      "reg_sel" : {"gpio" : "PB_13"},
                      "data4" : {"gpio" : "PB_6"},
                      "data5" : {"gpio" : "PB_7"},
                      "data6" : {"gpio" : "PB_8"},
                      "data7" : {"gpio" : "PB_9"},
                      "light" : {"gpio" : "PB_0"}
        }
    }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"welcome"` | Array with two strings. First string is default top line text, second string is default bottom line text.| Array with two strings | Yes
| `"requires"` | This object contain description of peripherals required. | Object | Yes |
| `"enable"` | GPIO pin for enable signal | `"gpio"` | Yes |
| `"reg_sel"` |  GPIO pin for Instruction/Data Register Selection signal | `"gpio"` | Yes |
| `"data4"` | GPIO pin for data 4 signal | `"gpio"` | Yes |
| `"data5"` | GPIO pin for data 5 signal | `"gpio"` | Yes |
| `"data6"` | GPIO pin for data 6 signal | `"gpio"` | Yes |
| `"data7"` | GPIO pin for data 7 signal | `"gpio"` | Yes |
| `"light"` | GPIO pin for back light signal | `"gpio"` | Yes |