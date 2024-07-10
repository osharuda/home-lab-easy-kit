#!/bin/bash

OPENOCD_SCRIPTS=/usr/share/openocd/scripts
ADAPTER="${OPENOCD_SCRIPTS}"/interface/stlink-v2-1.cfg
TARGET="${OPENOCD_SCRIPTS}"/target/stm32f1x.cfg
LOCATION=${1}

killall openocd
openocd -f "${ADAPTER}" -f "${TARGET}" &

cd ${LOCATION}
gdb-multiarch hlekfw.elf \
  -ex 'target remote localhost:3333' \
  -ex 'monitor reset halt'
