#!/bin/bash

# Usage:
# =====================================================================================
# build.sh <json 1> ... <json N> <Release>  <Clean>
# - Run this scipt in the same dirrectory with all json files.
# - <Release> parameter instructs to build release version of firmware/software components.
#             Optional, by default debug version is built.
# - <Clean> parameter instructs to clean project before building. Optional.
# =====================================================================================

BUILDCONF="Debug"
DOCLEAN=0
for ARG in $@
do
    if [ ${ARG^^} == "RELEASE" ]
    then
        BUILDCONF="Release"
        continue
    fi

    if [ ${ARG^^} == "CLEAN" ]
    then
        DOCLEAN=1
        continue
    fi
done

echo "======   Building ${BUILDCONF} configuration   ======"

# Clean if required
if [ ${DOCLEAN} != 0 ]; then
    ./clean.sh
fi

# Generate and build HLEK components
../../build.sh tb_ad9850dev.json tb_timetrackerdev.json ${BUILDCONF}

# Configure hlekio, do not build since build is going to be cross-compiled
rm -rf hlekio/env.sh
ln -s ./environments/env_rpi0.sh ./hlekio/env.sh
./customize.sh --hlekio=hlekio.json --verbose


# Generate CMake project
rm -f build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "Ninja" ../

# Build it
ninja


