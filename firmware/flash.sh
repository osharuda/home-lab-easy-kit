#!/bin/bash
killall openocd

# Check file is OK
FILE=./${1}/hlekfw.hex
if ! test -f "${FILE}"; then
    echo "Build ${1} first"
    exit 1
fi
 
# Check st-util is working
if ! st-info --chipid; then
    echo "Programmer is not connected"
    exit 1
fi

until st-flash --format ihex write ${FILE}; do sleep 1; done

