CUR_SCRIPT=$(readlink -f "${BASH_SOURCE[0]}")
CUR_SCRIPT_DIR="$(cd "$( dirname "${CUR_SCRIPT}" )" >/dev/null 2>&1 && pwd)"
. "${CUR_SCRIPT_DIR}/hlekenv.sh"

clang-tidy \
    -checks="*" \
    -p ./Debug \
    --use-color \
    inc/*.h \
    ../../firmware/inc/*.h \
    ../../firmware/src/*.c \
    -header-filter=.* | tee static_check.log

"${CUR_SCRIPT_DIR}/../.venv/bin/python" -m clang_html static_check.log -o static_check.html

