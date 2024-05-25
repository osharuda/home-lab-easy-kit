#!/bin/bash

. ./bashutil.sh
check_env

# Setting required variables
. ./env.sh
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER clean
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER alldebug
${TARGET_CROSS_COMPILER}objdump -D ${KO_NAME}.ko > objdump.txt
readelf -a ${KO_NAME}.ko > relf.txt

echo ''
echo ${IP}'Done!'
