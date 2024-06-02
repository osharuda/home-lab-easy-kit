#!/bin/bash

. ./bashutil.sh
check_env

# Downloading kernel
rm -rf linux
git clone --depth=1 https://github.com/raspberrypi/linux
#tar xzf ./backup/linux_kernel.tar.gz
cd linux

# Setting required variables
. ../env.sh
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER $TARGET_DEF_CONFIG


# Configuring kernel
./scripts/config --enable DEBUG_INFO_DWARF5
./scripts/config --enable VMLINUX_MAP
./scripts/config --enable IKCONFIG
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER menuconfig


# Build kernel
echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃                                           BUILDING KERNEL                                                 ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER zImage modules dtbs

echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃                                           BUILDING GDB SCRIPTS                                            ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER scripts_gdb  

echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃                                           BUILDING DOCUMENTATION                                          ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER htmldocs
