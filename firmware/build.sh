#!/bin/bash
rm -rf build
mkdir build
cd build
cmake -G"CodeBlocks - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE:PATH="./toolchain.cmake"  ..
make
