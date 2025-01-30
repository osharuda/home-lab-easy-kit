#!/bin/bash
CUR_SCRIPT=$(readlink -f "${BASH_SOURCE[0]}")
CUR_SCRIPT_DIR="$(cd "$( dirname "${CUR_SCRIPT}" )" >/dev/null 2>&1 && pwd)"

. "${CUR_SCRIPT_DIR}/scripts/hlekenv.sh"

PROJECTDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
cd ${PROJECTDIR}

PROJECTDIR=${PROJECTDIR} "${CUR_SCRIPT_DIR}/.venv/bin/python" ${PROJECTDIR}/customizer/customizer/customizer.py "$@"
