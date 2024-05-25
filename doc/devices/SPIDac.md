# <p align="center">SPIDac</p>
<p align="center"><img src="../images/hlek.svg"></p>


SPIDAC is a virtual device that may communicate with different spi based digital to analog convertors (DACs) and generate arbitrary waveforms.

**Note, currently the only DAC tested so far is DAC7611 (configuration below is written for DAC7611), others are not guaranteed to work. It is very possible this feature will be changed in a future to provide better and more convenient interface.**

```
"SPIDACCustomizer": {
  "spidac" : {
    "dev_id" : 1,
    "samples_number" : 100,
    "clock_phase" : "second",
    "clock_polarity" : "idle_high",
    "clock_speed" : "9MHz",
    "frame_format" : "msb",
    "frame_size" : 16,
    "frames_per_sample" : 1,
    "ld_mode" : "fall",
    "bits_per_sample" : 12,
    "channels" : {
        "dac_out" : {
          "index" : 0,
          "min_value" : 0.0,
          "max_value" : 4.0,
          "default_value" : 2.0
        }
    },
    "requires" :  {"SPI" : {"spi": "SPI2"},
                   "TIMER" : {"timer": "TIM1"},
                   "LD" : {"gpio" : "PA_12"}}
    }
  }
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"spidac"` | Name of the virtual device. | String | Yes |
| `"dev_id"` | Device id. | Number, [1, 15] | Yes |
| `"samples_number"` | Defines size of the internal bufer to store signal. Each sample includes values for all channels. | Number | Yes |
| `"clock_phase"` | SPI clock phase. | `first`, `second` | Yes |
| `"clock_polarity"` | SPI clock polarity. | `idle_high` or `idle_low` | Yes |
| `"clock_speed"` | SPI clock speed. Note, some values may be not supported on `SPI1` or `SPI2`.|  `70khz`, `140khz`, `281khz`, `562khz`, `1.125mhz`, `2.25mhz`, `4.5mhz`, `9mhz`, `18mhz`, `36mhz` | Yes |
| `"frame_format"` | SPI frame format (The most or least significant bit). | Either `msb` or `lsb` | Yes |
| `"frames_per_sample"` | Frames per sample (all channels). | Number | Yes |
| `"ld_mode"` | Describes front type of the LD pin. Do not specify if `"LD"` pin is not used. | `fall` or `rise` | No |
| `"bits_per_sample"` | Describes number of bits per sample (single channel value). | Number | Yes |
| `"channels"` | Describes DAC's channels. | Object with values describing channels | Yes |
| `"index"` | Channel index (as it goes in SPI transaction). | String | Yes |
| `"min_value"` | Minimum signal value. | FP value | Yes |
| `"max_value"` | Maximum signal value. | FP value | Yes |
| `"default_value"` | Default signal value. | FP value | Yes |
| `"requires"` | Describes peripherals required by the virtual device. | Object with required peripherals | Yes |
| `"SPI"` | SPI to be used. | `"SPI1"`, `"SPI2"` | Yes |
| `"TIMER"` | Word load clock signal | `"timer"` | Yes |
| `"LD"` | Optional, this is a pin to pass LD signal for some DACs. Do not specify it if LD pin is not used. | `"gpio"` | No |

<p align="center"><img src="../../doxygen/images/under_construction.png"></p>