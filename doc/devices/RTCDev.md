# <p align="center">RTCDev</p>
<p align="center"><img src="../images/hlek.svg"></p>

RTCDev is a virtual device that provides access to the STM32F103C8T6 real time clock. This is very useful especially when micro computer doesn't have real time clock on a board. This might be an issue if no internet access is available and computer may be powered down periodically for some reason.

Example JSON configuration is given below:
```
"RTCCustomizer" : {
  "rtc": {
      "dev_id": 2,
      "requires": { "bkp" : "BKP_DR1",
                    "rtc" : "RTC"}
  }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"requires"` | This object contain description of peripherals required. | Object | Yes |
| `"bkp"` | Backup register to be used | `"bkp"` | Yes |
| `"rtc"` | RTC to be used | `"rtc"` | Yes |
