#!/bin/bash

rm -rf hashes
rm -rf libhlek/Debug
rm -rf libhlek/Release
rm -rf build.log
rm -rf libhlek/CMakeLists.txt
rm -rf libhlek/inc/i2c_proto.h
rm -rf libhlek/inc/*_common.hpp
rm -rf libhlek/cmake-build-debug
rm -rf libhlek/cmake-build-release
rm -rf libhlek/hlekConfig.cmake
rm -rf libhlek/Debug
rm -rf libhlek/Release
rm -rf software/testtool/circbuffer.*
rm -rf software/testtool/utools.*
rm -rf software/testtool/CMakeLists.txt

# Misc IDE and cache files
rm -rf misc/devconsole/.idea/
rm -rf libhlek/.idea/
rm -rf customizer/.idea
rm -rf customizer/customizer/__pycache__
rm -rf misc/devconsole/__pycache__
rm -rf misc/devconsole/*~

i=0
JSONS=()
for ARG in $@
do
	json=$(basename -- "$ARG")
	rm -rf "${json%.*}"
done

