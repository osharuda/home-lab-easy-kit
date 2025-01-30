#!/bin/bash
HE_SCRIPT=$(readlink -f "${BASH_SOURCE[0]}")
HE_SCRIPT_DIR="$(cd "$( dirname "${HE_SCRIPT}" )" >/dev/null 2>&1 && pwd)"
HLEK_BASE_DIR="${HE_SCRIPT_DIR}/.."

source "${HE_SCRIPT_DIR}/utils.sh"

function export_json_var() {
    local file_name="${1}"
    for s in $(cat "${file_name}" | jq -r 'keys[] as $k | "\($k)=\"\(.[$k])\""'); do
        export $s > /dev/null
    done
}

function make_venv() {
    echo_progess_begin "Installing python virtual environment"
    "${HLEK_BASE_DIR}/make_venv.sh" >/dev/null 2>&1
    echo_progess_ok
}

export_json_var "${HE_SCRIPT_DIR}/../global.json"

if [ ! -d "${HLEK_BASE_DIR}/.venv" ] ; then
    make_venv
fi



