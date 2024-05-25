#!/bin/bash
echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
echo '┃ Using kernel configuration for Raspberry Pi 1, Zero and Zero W, and Raspberry Pi Compute Module 1, 32-bit ┃'
echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'

export KERNEL=kernel
export KERNEL_VER=$(cat Makefile | grep -E -w '^VERSION' | sed -e 's/VERSION//' -e 's/=//' -e 's/\s*//')
export KERNEL_PATCH_LEVEL=$(cat Makefile | grep -E -w '^PATCHLEVEL' | sed -e 's/PATCHLEVEL//' -e 's/=//' -e 's/\s*//')
export TARGET_ARCH=arm
export TARGET_CROSS_COMPILER=arm-linux-gnueabihf-
export TARGET_DEF_CONFIG=bcmrpi_defconfig

