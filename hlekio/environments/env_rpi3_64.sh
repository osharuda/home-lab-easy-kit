#!/bin/bash
echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃ Using kernel configuration for Raspberry Pi 3, 3+, 4, 400 and Zero 2 W, and Raspberry Pi Compute Modules 3, 3+ and 4, 64-bit ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'

KERNEL=kernel8
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm64
TARGET_CROSS_COMPILER=aarch64-linux-gnu-
TARGET_DEF_CONFIG=bcm2711_defconfig


