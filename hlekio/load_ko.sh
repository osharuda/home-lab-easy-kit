#!/bin/bash

. ./bashutil.sh
check_env
set +e

check_is_root

# Setting required variables
. ./env.sh

# Check if module is loaded, if loaded unload
MOD_RE="^${KO_NAME}\s"
LMOD=$(lsmod | grep -e ${MOD_RE} | wc -c)
# Load module
if [ ${LMOD} -gt "0" ];
then
    echo $IP"Unloading module . . ."
    rmmod ${KO_FNAME}
fi

echo Loading module . . .
insmod ${KO_FNAME}
python3 ./gen_start_kgdb.py ${KO_NAME} > ./debug.sh
chmod +x ./debug.sh

if [ $? -eq 0 ]; then
  echo $IP"Module loaded successfully."
else
  echo $EP"Module load failed; Exiting."
  exit 1
fi

# Prepare add-symbol-file command for gdb and print it

# Build kernel
#echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
#echo '┃                                           CHECK IF DTBO IS                                                  ┃'
#echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
#make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER zImage modules dtbs

#echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
#echo '┃                                           BUILDING GDB SCRIPTS                                            ┃'
#echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
#make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER scripts_gdb  

#echo '┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓'
#echo '┃                                           BUILDING DOCUMENTATION                                          ┃'
#echo '┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛'
#make -j $(nproc) ARCH=$TARGET_ARCH CROSS_COMPILE=$TARGET_CROSS_COMPILER htmldocs
