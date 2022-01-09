#!/bin/bash
#
set -e
./build.sh
cd build
sudo make install
cd ..

