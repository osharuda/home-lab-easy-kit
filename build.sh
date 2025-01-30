#!/bin/bash

# In order to build without LIBHLEK set NO_LIBHLEK variable like in the example below:
# NO_LIBHLEK=1 ./build.sh <.....>

SCRIPT="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$( dirname "${SCRIPT}" )" >/dev/null 2>&1 && pwd)"
. "${SCRIPT_DIR}/scripts/hlekenv.sh"


set -e
INITDIR="$(pwd)"
JOBCOUNT="$(nproc)"
BUILDDIR="build"
NCPU=$(grep -c ^processor /proc/cpuinfo)
BUILDCONF="Debug"
CLEAN_BUILD=0
SCRIPT_DIR="scripts"
ROOTDIR=$(pwd)
LOGFILE="${ROOTDIR}/build.log"
FINISH_SUCCESS=0

trap 'exit_handler ${FINISH_SUCCESS} ${PROGRESS_PART1_LEN}' EXIT


######## Detect if we are working in different directory. In this case we should use symlink to mirror project in current directory
HLEKDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
PROJECTDIR=$(pwd)

function exit_handler() {
	success=${1}
	PROGRESS_PART1_LEN=${2}
	if [ ${FINISH_SUCCESS} == 0 ]; then
		echo_progess_fail
		echo_box "${RED}F A I L U R E${NORMAL}" 13; 
	fi
}


# Creates sym link
# {1} - src path
# {2} - destination path
# {3} - file or directory name; Note this function uses basename of this path, so it's possible
#       to use find result.
# Note: this function is being exported, to be called from sub-shell during find
make_sym_link () {
    local bn=$(basename ${3})
    local src_file="${1}/${bn}"
    if [ -z "${4}" ]; then
        local dst_file="${2}/${bn}"
    else
        local dst_file="${2}/${4}"
    fi

    # Check if required symlink is already created
    if [ -h ${dst_file} ]; then
        local link_path=$(readlink -f ${dst_file})
        if [ ${link_path} == ${src_file} ]; then
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
        echo_err "May not mirror HLEK into self."
        exit 1
    fi

    echo_progess_begin "Mirroring HLEK project tree"

    local d='customizer'
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='firmware'
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='software'
    make_sym_link ${HLEKDIR} ${PROJECTDIR} ${d}

    local d='libhlek'
    mkdir -p ${d}
    make_sym_link ${HLEKDIR}/${d} ${PROJECTDIR}/${d} "src"
    local lhlek_inc="${d}/inc"
    mkdir -p "${lhlek_inc}"
    find "${HLEKDIR}/${lhlek_inc}" ! -name '*_common.hpp' ! -name 'i2c_proto.h' -name "*.hpp" -exec bash -c "make_sym_link \"${HLEKDIR}/${lhlek_inc}\" \"${PROJECTDIR}/${lhlek_inc}\" '{}'" \;

    local d='hlekio'
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

    echo_progess_ok
}

if [ ${PROJECTDIR} != ${HLEKDIR} ]; then
    hlek_mirror ${HLEKDIR} ${PROJECTDIR}
fi


####### Parse arguments
i=0
JSONS=()
for ARG in $@
do
	if [ ${ARG^^} = "RELEASE" ]
	then
		BUILDCONF="Release"
		continue
	fi

	if [ ${ARG^^} = "DEBUG" ]
	then
		continue
	fi

	if [ ${ARG^^} = "CLEAN" ]
	then
		CLEAN_BUILD=1
		continue
	fi

	JSONS[${i}]=${ARG}
	((++i))
done

function make_subproject {
	echo_progess_begin "CMake: ${CONFIGDIR}/$1/${BUILDCONF}"
	echo "Generating cmake project for ${CONFIGDIR}/$1" >> "${LOGFILE}" 2>&1
	cd ${CONFIGDIR} && cd $1 && mkdir ${BUILDCONF} && cd ${BUILDCONF}
	cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "Ninja" ../ >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
	echo_progess_ok
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

	echo_progess_begin "Build: ${CONFIGDIR}/$1/${BUILDCONF}"
	cd "${CONFIGDIR}/$1/${BUILDCONF}"
	ninja -v -j${NCPU} >> "${LOGFILE}" 2>&1
	echo_progess_ok

	if [ $skip_install == false ]
	then 
		echo_wrn "Installing $1"
		echo "Installing $1" >> "${LOGFILE}" 2>&1
		sudo ninja install >> "${LOGFILE}" 2>&1
	fi
	cd "${INITDIR}"

}


function make_libhlek {
    echo_progess_begin "CMake: libhlek/${BUILDCONF}"
    echo "Generating cmake project for LIBHLEK" >> "${LOGFILE}" 2>&1
    cd libhlek && mkdir ${BUILDCONF} && cd ${BUILDCONF}
    cmake -DCMAKE_BUILD_TYPE=${BUILDCONF} -G "Ninja" ../ >> "${LOGFILE}" 2>&1	
    cd ${INITDIR}
    echo_progess_ok
}

function build_libhlek {
	echo "Building LIBHLEK [${BUILDCONF}]" >> "${LOGFILE}" 2>&1

	echo_progess_begin "Customizing: libhlek"
	./customize.sh --libhlek --verbose >> "${LOGFILE}" 2>&1
	echo_progess_ok

	if [ -d "libhlek/${BUILDCONF}" ]
	then
		echo "Skipping LIBHLEK project generation" >> "${LOGFILE}" 2>&1
	else
		make_libhlek
	fi

	echo_progess_begin "Build: libhlek/${BUILDCONF}"
	cd "libhlek/${BUILDCONF}"
	ninja -v -j${NCPU} >> "${LOGFILE}" 2>&1
	echo_progess_ok
	echo_wrn "Installing libhlek"
	echo "Installing LIBHLEK" >> "${LOGFILE}" 2>&1
	sudo ninja install >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
}

function make_firmware {
	echo_progess_begin "CMake: firmware/${BUILDCONF}"
	echo "Generating cmake project for firmware/${CONFIGDIR}" >> "${LOGFILE}" 2>&1
	cd ${CONFIGDIR} && cd firmware && mkdir ${BUILDCONF} && cd ${BUILDCONF}
	cmake -G"Unix Makefiles" -DHLEK_ROOT="${INITDIR}" -DHLEK_CONFIG="${CONFIGDIR}" -DCMAKE_DEPENDS_USE_COMPILER=FALSE -DCMAKE_BUILD_TYPE=${BUILDCONF} -DCMAKE_TOOLCHAIN_FILE:PATH="../toolchain.cmake"  .. >> "${LOGFILE}" 2>&1
	cd ${INITDIR}
	echo_progess_ok
}

function build_firmware {
	echo "Building firmware [${BUILDCONF}]" >> "${LOGFILE}" 2>&1
	if [ -d "${CONFIGDIR}/firmware/${BUILDCONF}" ]
	then
		echo "Skipping firmware (${CONFIGDIR}) project generation" >> "${LOGFILE}" 2>&1
	else
		make_firmware
	fi

	echo_progess_begin "Build: firmware/${BUILDCONF}"
	cd "${CONFIGDIR}/firmware/${BUILDCONF}"
	make -j${NCPU} >> "${LOGFILE}" 2>&1
	cd "${INITDIR}"
	echo_progess_ok

    if [ ${BUILDCONF} = "Debug" ]; then
        echo_progess_begin "Creating static analyzer symlink"
        make_sym_link "${HLEKDIR}/${SCRIPT_DIR}" "${CONFIGDIR}/firmware" static_check_fw.sh static_check.sh
        echo_progess_ok
    fi

}

function clean_build {
	echo_progess_begin "Cleaning project build tree"
	rm -rf "libhlek/${BUILDCONF}"
	rm -rf hashes
	for j in ${JSONS[@]}
	do
		local CONFIGDIR="${json%.*}"
		rm -rf "${CONFIGDIR}"
	done
	echo_progess_ok
}


####### Preparing log file
rm -rf "${LOGFILE}"


if [ ${CLEAN_BUILD} != 0 ]; then
	clean_build
fi

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

FINISH_SUCCESS=1
echo_box "${GREEN}S U C C E S S${NORMAL}" 13

