#!/bin/bash
# This script is inteded to be run on target Raspberry PI machine

. ./bashutil.sh

if [ "$EUID" -ne 0 ]
  then echo ${EP}"Please run as root"
  exit
fi


check_target_arch


if [[ ! $(cat /proc/mounts | grep configfs) == */sys/kernel/config* ]]; then
    echo ${IP}'Mounting configfs'
    if mount -t configfs none /sys/kernel/config; then
        echo ${IP}'configfs mounted successfully'
    else
        echo ${EP}'Failed to mount configfs'
	exit 1
    fi
fi

mkdir -p /sys/kernel/config/device-tree/overlays/${DTS_NAME}
cat ${DTS_NAME}.dtbo > /sys/kernel/config/device-tree/overlays/${DTS_NAME}/dtbo


echo ${IP}'Done!'
