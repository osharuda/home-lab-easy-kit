#!/bin/bash

set -e
INITDIR="$(pwd)"
JOBCOUNT="$(nproc)"
BUILDDIR="build"
NCPU=$(grep -c ^processor /proc/cpuinfo)
BUILDCONF="Debug"
ROOTDIR=$(pwd)
LOGFILE="${ROOTDIR}/build.log"
NULL_JSON="null.json"

####### Parse arguments
i=0
JSONS=()
for ARG in $@
do
	if [ ${ARG^^} == "RELEASE" ]
	then
		BUILDCONF="Release"
		continue
	fi

	JSONS[${i}]=${ARG}
	((++i))
done


function build_subproject {
	local skip_install=true
	if [ $# == 2 ] && [ $2 == true ]
	then
		skip_install=false
	fi

	echo "Building $1 [${BUILDCONF}]"
	cd ${CONFIGDIR} && cd $1 && rm -rf build && mkdir build && cd build
	cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "CodeBlocks - Unix Makefiles" ../ >> "${LOGFILE}" 2>&1
	make -j${NCPU} >> "${LOGFILE}" 2>&1
	if [ $skip_install == false ]
	then 
		echo "Installing $1"
		sudo make install >> "${LOGFILE}" 2>&1
	fi
	cd ../../..
}

function build_libhlek {
		echo "Building LIBHLEK [${BUILDCONF}]"
		./customize.sh "${NULL_JSON}" >> "${LOGFILE}" 2>&1
		cd libhlek && rm -rf build && mkdir build && cd build
		cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "CodeBlocks - Unix Makefiles" ../ >> "${LOGFILE}" 2>&1
		make -j${NCPU} >> "${LOGFILE}" 2>&1
		echo "Installing LIBHLEK"
		sudo make install >> "${LOGFILE}" 2>&1
		cd ../..
		rm -rf ./null
}

function build_firmware {
	echo "Building firmware [${BUILDCONF}]"
	cd ${CONFIGDIR} && cd firmware && rm -rf build && mkdir build && cd build
	cmake -G"Unix Makefiles" -DCMAKE_DEPENDS_USE_COMPILER:BOOL=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE:PATH="../../firmware/toolchain.cmake"  .. >> "${LOGFILE}" 2>&1
	make -j${NCPU} >> "${LOGFILE}" 2>&1
	cd ../../..
}


####### Preparing log file
rm -rf "${LOGFILE}"

# Build & install libhlek
build_libhlek

####### Build specified configurations
for j in ${JSONS[@]}
do
	# Customize configuration
	json=$(basename -- "$j")
	CONFIGDIR="${json%.*}"
	echo "Customizing ${json}"
	rm -rf ${CONFIGDIR}
	./customize.sh ${json} >> "${LOGFILE}" 2>&1

	# Build & install libconf
	build_subproject "lib${CONFIGDIR}" true

	# Build monitor
	build_subproject "monitor"
	

	# Build example
	build_subproject "example"

	# Build firmware
	build_firmware
done
