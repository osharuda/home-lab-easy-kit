# <p align="center">Devices</p>
<p align="center"><img src="../doc/images/hlek.svg"></p>

## Supported devices classification

HLEK project provides support for some devices, which may be used to construct your hobbist experimental rig. Historically several terms to classify devices were introduced. Below these terms and device types are explained:

| Term                                | Meaning
|:------------------------------------|:----------|
|`"virtual device"`                   | These devices may be connected or used through MCU only. All C++ classes represented by this kind of devices are derived from `EKitVirtualDevice` class. These devices may be accessed only by HLEK i2c-based protocol (`EKitFirmware`)
|`"generic device"` or `"device"`| These devices may be connected to the MCU or microcomputer using one of the supported standard interfaces: I2C, SPI, UART. `"proxy"` device should be used to connect `generic device` through MCU, otherwise it may be connected and used directly to microcomputers (using either of `EKitI2CBus`, `EKitSPIBus`, `EKitUARTBus`). All C++ classes that represent this kind of devices derive from `EKitDeviceBase`.
|`"exclusive device"`                 | is a type of `"virtual device"` which may be present in specified MCU by the only instance. It's impossible to have multiple `"exclusive device"` on the same MCU. This is defined by actual device implementation.
|`"non-exclusive device"`             | is a type of `"virtual device"` which is not limited by single instance on the same MCU. You may have several devices, however the number may be limited by available MCU resources.
|`"proxy"`                            | Proxy, is a specific device which may expose one of the bus interfaces. Currently there are two proxies are implemented: `SPIProxyDev`, `UARTDev` (I2C proxy is not implemented yet). `"Generic devices"` may use `"proxy"` to be connected through the MCU.

## Devices documentation
### Measurements related devices
| Device | Description | Type | Exclusive | Status | Notes      
|:-|:-|:-|:-|:-|:-|
[INFODev](devices/INFODev.md) |Provides identification information about particular firmware| `virtual` | Yes        |`Completed`| 
|[TimeTrackerDev](devices/TimeTrackerDev.md) |Time measurement support| `virtual` | No        |`Completed`| 
|[ADCDev](devices/ADCDev.md) |Analogue to Digital Converter| `virtual` | No        |`Completed`| External synchronization to be added.
|[SPIDac](devices/SPIDac.md) |SPI Digital to Analog Converters| `virtual` | No        |`Completed`| External synchronization to be added.
|[StepMotorDev](devices/StepMotorDev.md) |Stepper motors support| `virtual` | No        |`Completed`| External synchronization to be added.
|[PaceMakerDev](devices/PaceMakerDev.md) |External synchronization| `virtual` | No        |`Completed`|
|[SPWMDev](devices/SPWMDev.md) |Timer based software PWM| `virtual` | Yes        |`Completed`| To be converted to non-exclusive device.
|[GPIODev](devices/GPIODev.md) |GPIO support| `virtual` | Yes        |`Completed`| To be converted to non-exclusive device with excternal synchronization.
|[AD9850Dev](devices/AD9850Dev.md) |AD9850 signal generator support| `virtual` | No        |`Completed`| External synchronization to be added.
|[ADXL345](devices/ADXL345.md) |3-axis accelerometer support| `generic` | N/A        |`In progress`| 
|[HMC5883L](devices/HMC5883L.md) |Magnetic field measurement support| `generic` | N/A        |`Planned`| 

### Miscellaneous bus support
| Device | Description | Type | Exclusive | Status | Notes      
|:-|:-|:-|:-|:-|:-
|[CanDev](devices/CanDev.md) |CAN bus support| `virtual` | No        |`Completed`|
|[UARTProxyDev](devices/UARTProxyDev.md) |UART proxy| `virtual` | No        |`Completed`|
|[SPIProxyDev](devices/SPIProxyDev.md) |SPI proxy| `virtual` | No        |`Completed`|

### Other devices
| Device | Description | Type | Exclusive | Status | Notes      
|:-|:-|:-|:-|:-|:-
|[LCD1602ADev](devices/LCD1602ADev.md) |Simple 16x2 LCD screen support| `virtual` | Yes        |`Completed`|
|[IRRCDev](devices/IRRCDev.md) |Infra-red remote control support| `virtual` | Yes        |`Completed`|
|[RTCDev](devices/RTCDev.md) |Real Time Clock support| `virtual` | Yes        |`Completed`|
|[DESKDev](devices/DESKDev.md) |Simple keyboard consisting of 4 buttons| `virtual` | Yes        |`Completed`|
|[GSMModem](devices/GSMModem.md) |SIM900 GSM modem support| `generic` | N/A        |`Completed`|


