# <p align="center">SPIProxyDev</p>
<p align="center"><img src="../images/hlek.svg"></p>


SPI proxy may act as proxy for SPI devices. Surely, you may use SPI interface from microcomputer, but, if it's not available or busy by something else it is possible to use the one from MCU.

```
"SPIProxyCustomizer": {
      "spiproxy" : {
        "dev_id" : 1,
        "use_dma" : 1,
        "bidirectional" : 1,
        "buffer_size" : 256,
        "clock_phase" : "first",
        "clock_polarity" : "idle_low",
        "clock_speed" : "281KHz",
        "frame_format" : "msb",
        "frame_size" : 8,
        "requires" :  { "SPI" : {"spi": "SPI1"}}
      }
    }
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"spiproxy"` | Name of the virtual device. | String | Yes |
| `"dev_id"` | Device id. | Number, [1, 15] | Yes |
| `"use_dma"` | Instructs to use SPI in dma mode. | `0`, `1` | Yes |
| `"bidirectional"` | Bidirectional communication. | `0`, `1` | Yes |
| `"buffer_size"` | Size of the buffer. | Number | Yes |
| `"clock_phase"` | SPI clock phase. | `first`, `second` | Yes |
| `"clock_polarity"` | SPI clock polarity. | `idle_high` or `idle_low` | Yes |
| `"clock_speed"` | SPI clock speed. Note, some values may be not supported on `SPI1` or `SPI2`.|  `70khz`, `140khz`, `281khz`, `562khz`, `1.125mhz`, `2.25mhz`, `4.5mhz`, `9mhz`, `18mhz`, `36mhz` | Yes |
| `"frame_format"` | SPI frame format (The most or least significant bit). | Either `msb` or `lsb` | Yes |
| `"frame_size"` | Size of the frame in bits. | `8`, `16` | Yes |
| `"requires"` | Describes peripherals required by the virtual device. | Object with required peripherals | Yes |
| `"SPI"` | SPI to be used. | `"SPI1"`, `"SPI2"` | Yes |