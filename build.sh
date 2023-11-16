#!/bin/bash

# In order to build without LIBHLEK set NO_LIBHLEK variable like in the example below:
# NO_LIBHLEK=1 ./build.sh <.....>

set -e
INITDIR="$(pwd)"
JOBCOUNT="$(nproc)"
BUILDDIR="build"
NCPU=$(grep -c ^processor /proc/cpuinfo)
BUILDCONF="Debug"
ROOTDIR=$(pwd)
LOGFILE="${ROOTDIR}/build.log"

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

function make_subproject {
	echo "Generating cmake project for ${CONFIGDIR}/$1" >> "${LOGFILE}" 2>&1
	cd ${CONFIGDIR} && cd $1 && mkdir ${BUILDCONF} && cd ${BUILDCONF}
	cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "Ninja" ../ >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
}

function build_subproject {
	local skip_install=true
	if [ $# == 2 ] && [ $2 == true ]
	then
		skip_install=false
	fi

	echo "Building $1 [${BUILDCONF}]" >> "${LOGFILE}" 2>&1
	if [ -d "${CONFIGDIR}/$1/${BUILDCONF}" ]
	then
		echo "Skipping cmake project generation for ${CONFIGDIR}/$1" >> "${LOGFILE}" 2>&1
	else
		make_subproject $1
	fi
	cd "${CONFIGDIR}/$1/${BUILDCONF}"

	ninja -v -j${NCPU} >> "${LOGFILE}" 2>&1
	if [ $skip_install == false ]
	then 
		echo "Installing $1" >> "${LOGFILE}" 2>&1
		sudo ninja install >> "${LOGFILE}" 2>&1
	fi
	cd "${INITDIR}"
}


function make_libhlek {
    echo "Generating cmake project for LIBHLEK" >> "${LOGFILE}" 2>&1
    cd libhlek && mkdir ${BUILDCONF} && cd ${BUILDCONF}
    cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "Ninja" ../ >> "${LOGFILE}" 2>&1	
    cd ${INITDIR}
}

function build_libhlek {
	echo "Building LIBHLEK [${BUILDCONF}]" >> "${LOGFILE}" 2>&1
	./customize.sh --libhlek >> "${LOGFILE}" 2>&1

	if [ -d "libhlek/${BUILDCONF}" ]
	then
		echo "Skipping LIBHLEK project generation" >> "${LOGFILE}" 2>&1
	else
		make_libhlek
	fi
	cd "libhlek/${BUILDCONF}"
	ninja -v -j${NCPU} >> "${LOGFILE}" 2>&1
	echo "Installing LIBHLEK" >> "${LOGFILE}" 2>&1
	sudo ninja install >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
}

function make_firmware {
	echo "Generating cmake project for firmware/${CONFIGDIR}" >> "${LOGFILE}" 2>&1
	cd ${CONFIGDIR} && cd firmware && mkdir ${BUILDCONF} && cd ${BUILDCONF}
	cmake -G"Unix Makefiles" -DHLEK_ROOT="${INITDIR}" -DHLEK_CONFIG="${CONFIGDIR}" -DCMAKE_DEPENDS_USE_COMPILER=FALSE -DCMAKE_BUILD_TYPE=${BUILDCONF} -DCMAKE_TOOLCHAIN_FILE:PATH="../toolchain.cmake"  .. >> "${LOGFILE}" 2>&1
	cd ${INITDIR}
}

function build_firmware {
	echo "Building firmware [${BUILDCONF}]" >> "${LOGFILE}" 2>&1
	if [ -d "${CONFIGDIR}/firmware/${BUILDCONF}" ]
	then
		echo "Skipping firmware (${CONFIGDIR}) project generation" >> "${LOGFILE}" 2>&1
	else
		make_firmware
	fi
	cd "${CONFIGDIR}/firmware/${BUILDCONF}"
	make -j${NCPU} >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
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
	json_file_name="$(cd "$(dirname "$j")"; pwd -P)/$(basename "$j")"
	CONFIGDIR="${json%.*}"
	echo "Customizing ${json}" >> "${LOGFILE}" 2>&1
	./customize.sh --json=${json_file_name} >> "${LOGFILE}" 2>&1

	# Build & install libconf
	build_subproject "lib${CONFIGDIR}" true

	# Build monitor
	build_subproject "monitor"
	

	# Build example
	build_subproject "example"

	# Build firmware
	build_firmware
done

./customize.sh --update-global
