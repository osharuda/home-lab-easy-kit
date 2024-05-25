<p align="center"><img src="doc/images/hlek.svg"></p>

# <p align="center">Home Lab Easy Kit</p></b>



## Description

Purpose for Home Lab Easy Kit project is to give amateurs possibility to build experimental setups with less efforts and price to save resources for other more important objectives like experiment logic, calculations, etc. Efficiency is achieved by providing C++ libraries which implement a lot of low level work, leaving users with experiment logic and hardware wiring.

For some specific hardware-related tasks (interrupts, gpio pins polling, timers, etc.) project utilizes STM32F103C8T6 micro controller (MCU) with communication by i2c bus. The firmware and configuration libraries are generated from user provided JSON file. Also, using this MCU provides such features as ADC, waveform signal generation, multichannel PWM, etc. However there are devices that may be used without MCU, using microcomputer based interfaces (UART, SPI, I2C).

There are number of benefits provided by this solution:

- Logic and heavy calculations may be moved from micro controller to computer, which performance is generally much better.
- Use of dedicated MCU helps to achieve real-time capabilities in some cases, while main software will use conventional preemptive scheduling.
- It is possible to use other libraries for database access, data science, networking etc.
- Communication utilizes I2C in 7-bit addressing mode. In theory you can connect up to 112 micro controllers to single I2C port.
- Firmware architecture is intended to use MCU peripherals economically to allow as much peripherals to be used per micro controller as possible.
- It is possible to use project for some type of devices without MCU at all.

For sure, user should have some theoretical and practical background related to micro controllers, electronics and what he is doing. Home Lab Easy Kit doesn't void this requirement, it just makes entrance easier and allows to save time.

Note, this project doesn't describe electrical connections and schematics, it's purpose to provide firmware and software for your design. Hardware is your responsibility. Documentation may have a few notes related electrical circuits, treat them as advice or suggestion from inexperienced engineer. For better information regarding electrical circuits and connections read corresponding documentation and literature.

Author doesn't take any responsibility for any possible consequences. You are responsible for everything you do with this project. Be very careful with electricity, it hits fast, unexpectedly and without any hesitation. Get corresponding knowledge before doing anything on practice.

## Table of contents
1. [Introduction](doc/INTRODUCTION.md)
2. [Architecture](doc/ARCHITECTURE.md)
2. [Firmware protocol](doc/PROTOCOL.md)
3. [Configuration](doc/CONFIGURATION.md)
4. [Devices](doc/DEVICES.md)
5. [Workflow](doc/WORKFLOW.md)

## Contributing

Everyone is welcome to participate this project. Discuss changes you want to make with author first before doing pull request. I do not promise the answer will be quick, however I'll try to respond to everyone.

Contribution may be made in several forms:
1. Testing. There were some testing, but it's not enough. I am certain there are bugs, I hope not many of them. Using the project and submitting issues will be very helpful.

2. Code review. This is my first open source project with STM32, and I understand lack of experience I have. So, it would be great if some one more experienced review this project, approaches I used, etc. Probably, this will result some features to be rewritten from the ground.

3. Documentation is very important. Currently documentation required to start work with the project is ready, however there are still many dark corners to be documented. Also, English is not my native language, so, fixing grammar, syntax mistakes is always good.

## Further development

There are a lot of ideas how to extend this project and move it forward. Among of others are:

1. Python3 module source code generation.
2. Adding support to other peripherals PWM (hardware), USB (as new protocol).
3. Estimating RAM requirements for firmware.
4. Possibility to use other MCU models.

## Contact

If you want to contact me you can reach me at oleh.sharuda@gmail.com

## License
```
Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## References and literature

1. [The Art of Electronics, by Paul Horowitz and Winfield Hill](https://en.wikipedia.org/wiki/The_Art_of_Electronics) - must have if you learn electronics.
2. STM32f103 related datasheets:
    - [DS5319](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf) - Datasheet for STM32F103C8 family of MCU.
    - [RM0008](https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf) - Datasheet for STM3210x family of MCU.
3. [SIM800 Series AT Command Manual](https://usermanual.wiki/Pdf/SIM80020SeriesAT20Command20ManualV109.183482162.pdf) - required if you plan to use SIM800 based GSM modem.
4. Software part is written with C++11. [This is](https://en.cppreference.com/w/cpp/11) one of the major resources for C++11.
5. Short video demonstration of flash light radiation diagram scanning: [video](https://www.youtube.com/watch?v=8mFc6aBiLas)
