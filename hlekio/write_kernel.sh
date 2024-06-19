#!/bin/bash

. ./bashutil.sh

SD_BOOTFS=/media/oleg/bootfs
SD_ROOTFS=/media/oleg/rootfs
TARGET_KERNEL_NAME=kernel_${KERNEL_VER}.${KERNEL_PATCH_LEVEL}_debug

check_sd_card
check_env

cd linux

# Setting required variables
. ../env.sh

# Install kernel modules
sudo -E env PATH=$PATH make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER INSTALL_MOD_PATH=$SD_ROOTFS modules_install

if [ $TARGET_ARCH == 'arm' ]; then
    echo ${IP}'ARM 32-bits'

    sudo -E cp arch/arm/boot/zImage $SD_BOOTFS/$TARGET_KERNEL_NAME.img
    DTS_SUBPATH=''
    if (($KERNEL_VER >= 6 && $KERNEL_PATCH_LEVEL >= 5)); then DTS_SUBPATH='/broadcom'; fi
    sudo -E cp arch/arm/boot/dts${DTS_SUBPATH}/*.dtb $SD_BOOTFS/
    sudo -E cp arch/arm/boot/dts/overlays/*.dtb* $SD_BOOTFS/overlays/
    sudo -E cp arch/arm/boot/dts/overlays/README $SD_BOOTFS/overlays/

elif [ $TARGET_ARCH == 'arm64' ]; then
    echo ${IP}'ARM 64-bits'

    sudo -E cp arch/arm64/boot/Image $SD_BOOTFS/$TARGET_KERNEL_NAME.img
    sudo -E cp arch/arm64/boot/dts/broadcom/*.dtb $SD_BOOTFS/
    sudo -E cp arch/arm64/boot/dts/overlays/*.dtb* $SD_BOOTFS/overlays/
    sudo -E cp arch/arm64/boot/dts/overlays/README $SD_BOOTFS/overlays/
else
    echo ${EP}'Uknown architecture'
    exit 1
fi

# Update config.txt
sudo -E rm -rf ${SD_BOOTFS}/config.bak
sudo -E cp ${SD_BOOTFS}/config.txt ${SD_BOOTFS}/config.bak
sudo -E cat ${SD_BOOTFS}/config.bak | sed -e '/kernel=/d' > ${SD_BOOTFS}/config.txt
sudo -E echo "kernel=${TARGET_KERNEL_NAME}.img" >> ${SD_BOOTFS}/config.txt

# Sync and unmount
sync
sudo -E umount $SD_BOOTFS
sudo -E umount $SD_ROOTFS

echo ${IP}'Kernel write is complete!'
echo ${IP}'Please remove SD card from card reader.'
