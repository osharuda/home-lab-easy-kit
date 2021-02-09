#!/bin/bash
PROJECTDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
CURDIR="$(pwd)"
FILEPATH="$(cd "$(dirname "${1}")"; pwd -P)/$(basename "${1}")"
cd "${PROJECTDIR}/doxygen"
doxygen ./doxygen_firmware.conf  
doxygen ./doxygen_software.conf

cd "${CURDIR}"

SOFTWARE_HTML="file://${CURDIR}/doxygen/software/html/index.html"
FIRMWARE_HTML="file://${CURDIR}/doxygen/firmware/html/index.html"

echo "------------------------------------------------------------------"
echo "Software documentation: ${SOFTWARE_HTML}"
echo "Firmware documentation: ${FIRMWARE_HTML}"
echo ""
echo "Done."
