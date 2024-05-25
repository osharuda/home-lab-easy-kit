# <p align="center">UARTProxyDev</p>
<p align="center"><img src="../images/hlek.svg"></p>

STM32F103C8T6 has three USART interfaces. UARTDev virtual device feature add possibility to use these interfaces to connected different UART devices.

```
"UartProxyCustomizer" : {
    "uart_proxy_0" : {
        "dev_id" : 4,
        "hint" : "gsmmodem",
        "buffer_size" : 2000,
        "baud_rate" : 9600,
        "requires" : { "usart" : "USART1"}
    }
}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"uart_proxy_0"` | Name of the UARTDev virtual device. | String | Yes |
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"hint"` | Optional hint, specifies which device is connected | "gsmmodem" | No |
| `"buffer_size"` | Size of the receive device buffer, in bytes | Number | Yes |
| `"baud_rate"` | Baud speed of communications | Number | Yes  |
| `"requires"` | This object contain description of peripherals required. Must has dependency to `"usart"` interface. | Object | Yes |

Note, this feature use two lines only: RX and TX. Other lines available for USART interfaces are not used.