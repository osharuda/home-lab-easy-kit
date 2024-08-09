#!/bin/bash

EXCL_FILE=cloc_exclude.txt
CLOC_DEBUG_DIR=clock_debug
STRIP_EXT=STRIPPED

ls -1 $(pwd)/software/testtool/*.h >  ${EXCL_FILE}
ls -1 $(pwd)/software/testtool/*.c >> ${EXCL_FILE}

rm -rf ${CLOC_DEBUG_DIR}
mkdir ${CLOC_DEBUG_DIR}


cloc --exclude-lang=XML,JSON,'DOS Batch' \
    --exclude-list-file=${EXCL_FILE} \
    --original-dir --strip-comments=${STRIP_EXT} \
    --exclude-dir='linux','venv','packages','__pycache__','.idea','build','Debug','Release','cmake-build-debug','cmake-build-release' \
    ./customizer \
    ./firmware \
    ./software \
    ./hlekio \
    ./libhlek \
    ./misc/devconsole \
    | grep -v Wrote


CODESIZE=$(LC_ALL=C find . -iname '*.STRIPPED' -type f -xtype f -print0 | du --human-readable --apparent-size --total --summarize --files0-from=- | tail --lines=1)
FILECOUNT=$(LC_ALL=C find . -iname '*.STRIPPED' -type f -xtype f | wc -l)

for f in $(find . -name "*.${STRIP_EXT}"); do rm $f; done
rm ${EXCL_FILE}

echo "Codebase size (no comments and empty lines) is ${CODESIZE} in ${FILECOUNT} files."

