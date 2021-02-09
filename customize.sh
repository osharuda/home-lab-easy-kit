#!/bin/bash
PROJECTDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
CURDIR="$(pwd)"
FILEPATH="$(cd "$(dirname "${1}")"; pwd -P)/$(basename "${1}")"
cd "${PROJECTDIR}/customizer/customizer"
python3 customizer.py "${FILEPATH}"
cd "${CURDIR}"
