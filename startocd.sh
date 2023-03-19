#!/bin/bash

OPENOCD_SCRIPTS=/usr/share/openocd/scripts
ADAPTER="${OPENOCD_SCRIPTS}"/interface/stlink-v2-1.cfg
TARGET="${OPENOCD_SCRIPTS}"/target/stm32f1x.cfg

openocd -f "${ADAPTER}" -f "${TARGET}" &
