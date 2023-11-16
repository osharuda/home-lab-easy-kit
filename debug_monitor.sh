#!/bin/bash
set -e
LOCATION=${1}

cd ${LOCATION}
PID=$(cat ./monitor.bin.pid)

gdb -se ./monitor.bin  -ex "attach ${PID}"

