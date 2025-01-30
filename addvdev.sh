#!/bin/bash

CUR_SCRIPT=$(readlink -f "${BASH_SOURCE[0]}")
PROJECTDIR="$(cd "$( dirname "${CUR_SCRIPT}" )" >/dev/null 2>&1 && pwd)"
. "${PROJECTDIR}/scripts/hlekenv.sh"

CURDIR="$(pwd)"
cd "${PROJECTDIR}/customizer/customizer"
PROJECTDIR=${PROJECTDIR} "${PROJECTDIR}/.venv/bin/python" Wizard.py "${1}" "new-dev" "add"
cd "${CURDIR}"
