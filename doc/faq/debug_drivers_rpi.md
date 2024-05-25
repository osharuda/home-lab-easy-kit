# <p align="center">Debugging drivers with kdb</p>
<p align="center"><img src="../images/hlek.svg"></p>

This instructions will guide you to raspberry pi kernel mode debugging with kdb.



## Introduction

Debug kernel is required in order to make the following:
* Enable kdb debugging using serial interface
* To have a kernel symbols
* To enable some additional kernel debugging features.

It is assumed debug kernel will be cross-compiled on Debian based Linux computer.

You need two computers. The first one is target Raspberry Pi being debuged, the other computer will be used to work with kernel debugger kdb. It could be another Raspberry Pi. These computers should have serial ports connected. 

It is ***highly recommended*** to run them using the same power source.

Provided instructions below are mixture and some light changes to the publically available information provided by:

https://eastrivervillage.com/KGDB-KDB-over-serial-with-RaspberryPi/

https://www.raspberrypi.com/documentation/computers/linux_kernel.html



## Install required prerequisites for cross-compilation:

```
sudo apt install git bc bison flex libssl-dev make libc6-dev libncurses5-dev
```

Additionally for ***32-bit*** kernels:
```
sudo apt install crossbuild-essential-armhf
```

For ***64-bit*** kernels:
```
sudo apt install crossbuild-essential-arm64
```

## Building debug kernel
* Create a directory for the Raspberry Pi kernel and clone kernel repository:
```
git clone --depth=1 https://github.com/raspberrypi/linux
cd linux
```
* Set `KERNEL` environment variable and default configuration corresponding to your Raspberry Pi. Other environemnt variables such as `TARGET_CROSS_COMPILER`, `TARGET_ARCH`,  `KERNEL_VER` and `KERNEL_PATCH_LEVEL` are set, they are used later for other scripts simplicity.

### Raspberry Pi 1, Zero and Zero W, and Raspberry Pi Compute Module 1, ***32-bit***:
```
KERNEL=kernel
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm
TARGET_CROSS_COMPILER=arm-linux-gnueabihf-
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER bcmrpi_defconfig
```

### Raspberry Pi 2, 3, 3+ and Zero 2 W, and Raspberry Pi Compute Modules 3 and 3+, ***32-bit***:
```
KERNEL=kernel7
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm
TARGET_CROSS_COMPILER=arm-linux-gnueabihf-
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER bcm2709_defconfig
```

### Raspberry Pi 4 and 400, and Raspberry Pi Compute Module 4, ***32-bit***:
```
KERNEL=kernel7l
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm
TARGET_CROSS_COMPILER=arm-linux-gnueabihf-
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER bcm2711_defconfig
```

### Raspberry Pi 3, 3+, 4, 400 and Zero 2 W, and Raspberry Pi Compute Modules 3, 3+ and 4, ***64-bit***:
```
KERNEL=kernel8
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm64
TARGET_CROSS_COMPILER=aarch64-linux-gnu-
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER bcm2711_defconfig
```

### Raspberry Pi 5, ***64-bit***:
```
KERNEL=kernel_2712
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm64
TARGET_CROSS_COMPILER=aarch64-linux-gnu-
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER bcm2712_defconfig
```

## Compile and run menuconfig:
```
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER menuconfig
```

Add all required debugging options to the kernel configuration.

Useful options:
`DEBUG_INFO_DWARF5`:
```
    Kernel hacking
        -> Compile-time checks and compiler options
            -> Debug information
```
or
```
linux/scripts/config --enable DEBUG_INFO_DWARF5
```
`VMLINUX_MAP`:
```         
    Kernel hacking
        -> Compile-time checks and compiler options
            -> Generate vmlinux.map file when linking
```
or
```
linux/scripts/config --enable VMLINUX_MAP
```

`IKCONFIG`:
```
General Setup
    -> Kernel .config support
```
or
```
linux/scripts/config --enable IKCONFIG
```


## Build the kernel:

```
make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER zImage modules dtbs
```

## Write kernel, dts and kernel modules to SD card
There are many ways to mount SD card. I use card reader. When SD card is inserted two partitions are opened. I will define partitions as `SD_BOOTFS` and `SD_ROOTFS`. Change these variables accordingly for your system.

```
SD_BOOTFS=/media/oleg/bootfs
SD_ROOTFS=/media/oleg/rootfs
```

Then assign some name to your kernel
```
TARGET_KERNEL_NAME=kernel_${KERNEL_VER}.${KERNEL_PATCH_LEVEL}_debug
```

Copy kernel modules onto SD card:
```
sudo -E env PATH=$PATH make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER INSTALL_MOD_PATH=$SD_ROOTFS modules_install
```

Copy kernel and device tree onto SD card:

For ***32-bit*** configurations:
```
sudo -E cp arch/arm/boot/zImage $SD_BOOTFS/$TARGET_KERNEL_NAME.img
DTS_SUBPATH=''
if (($KERNEL_VER >= 6 && $KERNEL_PATCH_LEVEL >= 5)); then DTS_SUBPATH='/broadcom'; fi
sudo -E cp arch/arm/boot/dts${DTS_SUBPATH}/*.dtb $SD_BOOTFS/
sudo -E cp arch/arm/boot/dts/overlays/*.dtb* $SD_BOOTFS/overlays/
sudo -E cp arch/arm/boot/dts/overlays/README $SD_BOOTFS/overlays/
```

For ***64-bit*** configurations:
```
sudo -E cp arch/arm64/boot/Image $SD_BOOTFS/$TARGET_KERNEL_NAME.img
sudo -E cp arch/arm64/boot/dts/broadcom/*.dtb $SD_BOOTFS/
sudo -E cp arch/arm64/boot/dts/overlays/*.dtb* $SD_BOOTFS/overlays/
sudo -E cp arch/arm64/boot/dts/overlays/README $SD_BOOTFS/overlays/
```

## Update config.txt
```
sudo -E rm -rf ${SD_BOOTFS}/config.bak
sudo -E cp ${SD_BOOTFS}/config.txt ${SD_BOOTFS}/config.bak
sudo -E cat ${SD_BOOTFS}/config.bak | sed -e '/kernel=/d' > ${SD_BOOTFS}/config.txt
sudo -E echo "kernel=${TARGET_KERNEL_NAME}.img" >> ${SD_BOOTFS}/config.txt
```

## Sync and unmount SD card:
```
sync
sudo -E umount $SD_BOOTFS
sudo -E umount $SD_ROOTFS
```

## Other things
* Enable UART
* Reboot
* To clean linux kernel run:
```
make mrproper
```
* To make currenly loaded kernel config available via `/proc/config.gz`:

```
sudo modprobe configs
```

* To mount configfs:
```
mount -t configfs none /sys/kernel/config
```



## Triggering debugger
Login via serial from another computer:
```
sudo minicom -D /dev/serial0
```

Use the following keystroke to send magic SysRq sequance (in terminal) to the kdb `Ctrl`-`A` `f` `g`


On Raspberry Pi:
```
sudo su
echo ttyAMA0 > /sys/module/kgdboc/parameters/kgdboc 
echo g > /proc/sysrq-trigger
```

## Using kgdb
Add the following command to the  ~/.gdbinit (if you miss it you will likely to be warned by debugger to do so).
```
add-auto-load-safe-path <linux path>/scripts/gdb/vmlinux-gdb.py
```

Start gdb on remote computer:
```
gdb-multiarch \
--cd=linux \
-ex 'set serial baud 57600' \
-ex 'target remote /dev/serial0' \
vmlinux
```

## Build device tree overlay and apply it in runtime
### Overlay file
```
some file
```

### Compile *.dts into *.dtbo file
```
commands
```

### 

### Use VS Code for Linux ko developlment

https://github.com/amezin/vscode-linux-kernel