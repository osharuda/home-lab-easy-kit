#!/bin/bash

INITDIR="$(pwd)"
JOBCOUNT="$(nproc)"
BUILDDIR="build"

rm -rf "${BUILDDIR}"
mkdir "${BUILDDIR}"

cd "${BUILDDIR}"

cmake -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles" ../
make -j ${JOBCOUNT}

cd "${INITDIR}"

