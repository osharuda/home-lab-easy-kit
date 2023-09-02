#!/bin/bash
PROJECTDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
cd ${PROJECTDIR}
PROJECTDIR=${PROJECTDIR} python3 ${PROJECTDIR}/customizer/customizer/customizer.py "$@"
