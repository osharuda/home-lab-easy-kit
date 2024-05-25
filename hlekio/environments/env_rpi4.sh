#!/bin/bash
echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃ Using kernel configuration for Raspberry Pi 4 and 400, and Raspberry Pi Compute Module 4, 32-bit ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'

KERNEL=kernel7l
KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
TARGET_ARCH=arm
TARGET_CROSS_COMPILER=arm-linux-gnueabihf-
TARGET_DEF_CONFIG=bcm2711_defconfig


