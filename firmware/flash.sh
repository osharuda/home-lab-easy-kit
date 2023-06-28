#!/bin/bash
killall openocd

until st-flash --format ihex write ./build/hlekfw.hex; do sleep 1; done

