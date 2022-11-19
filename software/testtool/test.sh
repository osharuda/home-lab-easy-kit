#!/bin/bash
set -e
INITDIR="$(pwd)"
JOBCOUNT="$(nproc)"
BUILDDIR="build"

rm -rf "${BUILDDIR}"
mkdir "${BUILDDIR}"

cd "${BUILDDIR}"

cmake -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles" ../
make -j ${JOBCOUNT}

./build/debug/testtool.bin

cd "${INITDIR}"

