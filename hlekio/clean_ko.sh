#!/bin/bash

. ./bashutil.sh
check_env

# Setting required variables
. ./env.sh
make ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER clean

echo ''
echo ${IP}'Done!'
