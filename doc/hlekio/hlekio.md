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

# 2. Building HLEKIO driver and dts
All the following operations are done inside `hlekio` directory
```
cd hlekio
```

You may want to try to use Raspberry Pi only, but it will take a lot of time to build linux kernel. The solution is to use crosscompiler. The process of building hlekio driver supports usage of crossplatform compiler, however to achieve that `home-lab-easy-kit` directory must be shared using NFS between your Raspberry Pi and development computer. It will save a lot of your time.

**Make sure that using NFS share is secure in your case, otherwise find anothe way to crosscompile kernel or use Raspberry Pi.**

- Install prerequisites, if required:
```
sudo ./install_prereq.sh
```

- Download kernel on target machine
Download Raspi linux kernel:
```
git clone --depth=1 https://github.com/raspberrypi/linux
```

- Create symbolic link for requied configuration. Choose one of the files from `environments` subdirectory.
```
ln -s ./environments/env_rpi0.sh ./env.sh
```

- Crossplatform build the kernel
```
./build_kernel.sh
```
- Mount SD-card with Raspi linux installed. Modify `write_kernel.sh` script and change definition of these variables accordingly your system:
    * `SD_BOOTFS` for boot partition
    * `SD_ROOTFS` for root partition

- Install kernel
```
./write_kernel.sh
```
This script will install kernel and set it as the one for boot. Remove SD-card, insert it to your Raspberry Pi and boot.

- Build the driver:
```
./build_ko.sh
```

**The following operations are to be run from yout Raspberry Pi.**

- Boot Raspberry Pi and change directory to `home-lab-easy-kit` (assume you established NFS sharing between developer computer and Raspberry Pi)
```
cd home-lab-easy-kit
```

- Customize DTS tree for hlekio driver:
Modify `hlekio.json` and customize or use another JSON file:
```
./customize.sh --hlekio=hlekio.json --verbose
```
This command will create DTS directory in `hlekio`

- Build DTS tree
```
./build_dtb_overlay.sh
```

- Load DTB file

# 2. Loading DTB and KO
```
sudo ./load_dtb_overlay.sh
sudo ./load_ko.sh
```

# 4. Using from libhlek library
<p align="center"><img src="../../doxygen/images/under_construction.png"></p>
