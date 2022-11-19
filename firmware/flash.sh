#!/bin/bash
killall openocd
st-flash --format ihex write ./build/hlekfw.hex

