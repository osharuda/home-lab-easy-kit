#!/bin/bash
PROJECTDIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
CURDIR="$(pwd)"
cd "${PROJECTDIR}/customizer/customizer"
python3 Wizard.py "${1}" "new-dev" "remove"
cd "${CURDIR}"
