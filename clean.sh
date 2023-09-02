#!/bin/bash

rm -rf hashes
rm -rf libhlek/Debug
rm -rf libhlek/Release
rm -rf build.log
rm -rf libhlek/CMakeLists.txt
rm -rf libhlek/inc/i2c_proto.h
rm -rf libhlek/inc/*_common.hpp
rm -rf software/testtool/circbuffer.*
rm -rf software/testtool/utools.*
rm -rf software/testtool/CMakeLists.txt

i=0
JSONS=()
for ARG in $@
do
	json=$(basename -- "$ARG")
	rm -rf "${json%.*}"
done

