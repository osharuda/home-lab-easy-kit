#!/bin/bash
# This script is inteded to be run on target Raspberry PI machine

. ./bashutil.sh

# check_target_arch

alias dtcpp="cpp -nostdinc -undef -x assembler-with-cpp"
rm -rf ${DTS_NAME}.dts.preprocessed
cpp -nostdinc -I ./linux/include -I ./linux/arch  -undef -x assembler-with-cpp ${DTS_NAME}.dts ${DTS_NAME}.dts.preprocessed
dtc -@ -I dts -O dtb -o ${DTS_NAME}.dtbo ${DTS_NAME}.dts.preprocessed

echo ${IP}'Done!'
