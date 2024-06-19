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


######## Detect if we are working in different directory. In this case we should use symlink to mirror project in current directory
HLEKDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
PROJECTDIR=$(pwd)


# Creates sym link
# {1} - src path
# {2} - destination path
# {3} - file or directory name; Note this function uses basename of this path, so it's possible
#       to use find result.
# Note: this function is being exported, to be called from sub-shell during find
make_sym_link () {
    local bn=$(basename ${3})
    local src_file="${1}/${bn}"
    local dst_file="${2}/${bn}"

    # Check if required symlink is already created
    if [ -h ${dst_file} ]; then
        local link_path=$(readlink -f ${dst_file})
        if [ ${link_path} == ${src_file} ]; then
            echo "${dst_file} is already created and ok."
            return
        fi
    fi

    if [ ! -e ${src_file} ]; then
       echo "'${src_file}' doesn't exist."
       exit 1
    fi
 
    if [ -e ${dst_file} ]; then
       echo "'${dst_file}' already exist."
       exit 1
    fi
 
    ln -s ${src_file} ${dst_file}
}
export -f make_sym_link

# Mirrors HLEK project using symlinks
# {1} - HLEK directory
# {2} - Target directory
hlek_mirror () {
    if [ ${PROJECTDIR} == ${HLEKDIR} ]; then
        echo "May not mirror HLEK into self."
        exit 1
    fi

    local d='customizer'
    echo "Creating symlinks for ${d}..."
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='firmware'
    echo "Creating symlinks for ${d}..."
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='software'
    echo "Creating symlinks for ${d}..."
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='libhlek'
    echo "Creating symlinks for ${d}..."
    mkdir -p ${d}
    make_sym_link ${HLEKDIR}/${d} ${PROJECTDIR}/${d} "src"
    local lhlek_inc="${d}/inc"
    mkdir -p "${lhlek_inc}"
    find "${HLEKDIR}/${lhlek_inc}" ! -name '*_common.hpp' ! -name 'i2c_proto.h' -name "*.hpp" -exec bash -c "make_sym_link \"${HLEKDIR}/${lhlek_inc}\" \"${PROJECTDIR}/${lhlek_inc}\" '{}'" \;

    local d='hlekio'
    echo "Creating symlinks for ${d}..."
    mkdir -p ${d}
    make_sym_link ${HLEKDIR}/${d} ${PROJECTDIR}/${d} "linux"
    make_sym_link ${HLEKDIR}/${d} ${PROJECTDIR}/${d} "environments"
    make_sym_link ${HLEKDIR}/${d} ${PROJECTDIR}/${d} "Makefile"
    find "${HLEKDIR}/${d}" -maxdepth 1 -name '*.h' -exec bash -c "make_sym_link \"${HLEKDIR}/${d}\" \"${PROJECTDIR}/${d}\" '{}'" \;
    find "${HLEKDIR}/${d}" -maxdepth 1 -name '*.c' -exec bash -c "make_sym_link \"${HLEKDIR}/${d}\" \"${PROJECTDIR}/${d}\" '{}'" \;
    find "${HLEKDIR}/${d}" -maxdepth 1 -name '*.sh' -exec bash -c "make_sym_link \"${HLEKDIR}/${d}\" \"${PROJECTDIR}/${d}\" '{}'" \;
    find "${HLEKDIR}/${d}" -maxdepth 1 -name '*.py' -exec bash -c "make_sym_link \"${HLEKDIR}/${d}\" \"${PROJECTDIR}/${d}\" '{}'" \;

    make_sym_link ${HLEKDIR} ${PROJECTDIR} global.json
    make_sym_link ${HLEKDIR} ${PROJECTDIR} customize.sh
}

if [ ${PROJECTDIR} != ${HLEKDIR} ]; then
    hlek_mirror ${HLEKDIR} ${PROJECTDIR}
fi


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

	if [ ${ARG^^} == "DEBUG" ]
	then
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
	./customize.sh --libhlek --verbose >> "${LOGFILE}" 2>&1

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
	./customize.sh --json=${json_file_name} --verbose >> "${LOGFILE}" 2>&1

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
