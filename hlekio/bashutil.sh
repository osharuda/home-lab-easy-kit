#!/bin/bash

PROJECT_NAME='hlekio'
DTS_NAME=${PROJECT_NAME}
KO_NAME=${PROJECT_NAME}
KO_FNAME="${PROJECT_NAME}.ko"
EP='▒▒▒ '
IP='჻ '
LOCAL_ARCH=$(uname -m)

set -e

check_is_root () {
    if [ "$EUID" -ne 0 ]; then 
        echo ${EP}"Please run as root"
        exit
    fi
}

check_target_arch () {
    if [ $LOCAL_ARCH == 'armv6l' ]; then
	:
    else
	echo ${EP}'This script should be run on the target machine'
	exit 1
    fi
}

check_sd_card () {
    if [ ! -d ${SD_BOOTFS} ]; then
        echo ${EP}'SD card is not mounted or SD_BOOTFS has wrong value: '${SD_BOOTFS}
        exit 1
    fi

    if [ ! -d ${SD_ROOTFS} ]; then
        echo ${EP}'SD card is not mounted or SD_ROOTFS has wrong value: '${SD_ROOTFS}
        exit 1
    fi
}

check_env () {
    if [ ! -f 'env.sh' ]; then
        echo ${EP}'Enviroment file is not specified. Please create a symbolic link env.h to one of these scripts:'
        echo ${EP}'env_rpi0.sh'
        echo ${EP}'env_rpi2.sh'
        echo ${EP}'env_rpi4.sh'
        echo ${EP}'env_rpi3_64.sh'
        echo ${EP}'env_rpi5_64.sh'
        echo ${EP}
        echo ${EP}'Please run the following command to create corresponding symlink:'
        echo ${EP}'ln -s environments/<env file script> env.sh'

        exit 1
    fi
}

check_mod_loaded() {
	res=$(lsmod | grep -e '^${1}\s')
	echo $res
}
