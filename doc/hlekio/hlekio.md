# <p align="center">HLEKIO</p>
<p align="center"><img src="../images/hlek.svg"></p>

# 1. Purpose
`I2C` is a bus where communication may be initiated by master only. However, devices require trigger some events on software side. A good example is situation where device buffer is almost full, and software must take appropriate steps to avoid buffer overflow as soon as possible.

The first and intuitive solution is to request amount of data in device buffer periodically and react when it becomes critical. This aproach has two big disadvantages:
- Useless waste of CPU time, possibly required for other purposes.
- `I2C` bus will be ocupied, therefor other devices might become less responsive.

Standard way to address this issues are interrupts. Device may generate interrupt and microcomputer may react and wake up waiting thread.

`HLEKIO` is a kernel module or driver that establishes waiting for interrupts.`HLEKIO` may be configured with two types of devices: inputs and outputs. Outputs may be used to control `GPIO` pin states. Inputs may be used to reading input `GPIO` pin state, or waiting for interrupt on this pin.

Currently, there is GPIO Sysfs interface to user mode programs available on Linux, so kernel module isn't really nescessary. However, this functionality is depricated, and sooner or latter will become a history. Also, this functionality doesn't allow some extra features, like counting interrupts. You can read more about it [here](https://www.kernel.org/doc/Documentation/gpio/sysfs.txt).

Keeping all that in mind, `HLEKIO` kernel module was introduced into `Home Lab Easy Kit` project. Module is being configured with Device Tree Overlay, which in turn is generated using user provided json file.

# 2. Building HLEKIO
<p align="center"><img src="../../doxygen/images/under_construction.png"></p>

# 3. Generating Device Tree Overlay
<p align="center"><img src="../../doxygen/images/under_construction.png"></p>

# 4. Using from libhlek library
<p align="center"><img src="../../doxygen/images/under_construction.png"></p>
