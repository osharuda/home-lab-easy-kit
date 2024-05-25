import os.path
import sys

help = \
"""------------------------------------------------------------------------------------------------------------------------
Usage: python3 gen_debug_script.py <module name>
where <module name> is a kernel module file name without extension.

Note: in order this script to produce correct results, you the kernel module must be located in the current directory,
while build linux kernel should be located in 'linux' directory (also located in the current directory).

Note: This script requires root permissions, run it with sudo or using root.
------------------------------------------------------------------------------------------------------------------------
"""

if len(sys.argv) != 2:
    print(help)
    raise RuntimeError('Invalid argument.')

module_name = sys.argv[1]
ko_name = f'{module_name}.ko'
section_path = f'/sys/module/{module_name}/sections'
fn = os.path.join(section_path, '.text')

def read_section_file(fn : str) -> str:
    with open(fn, "r") as f:
        return f.read().strip()

def get_section_pointer(fn: str) -> tuple[str, int]:
    return (os.path.basename(fn), read_section_file(fn))

section_info = ''
text_addr = 0
try:
    for f in os.listdir(section_path):
        fn = os.path.join(section_path, f)
        if os.path.isfile(fn):
            section, address = get_section_pointer(fn)
            if section=='.text':
                text_addr = address
            elif section=='.symtab' or section=='.strtab':
                pass
            else:
                section_info += f' -s {section} {address}'

except PermissionError as e:
    print(f'{e.strerror}. Run as root.')
    print(str(e))
    print(help)
    quit(255)

debug_script = \
f"""#!/bin/bash
gdb-multiarch \\
    --cd=linux \\
    -ex 'set serial baud 115200' \\
    -ex 'target remote /dev/ttyUSB0' \\
    -ex 'add-symbol-file ../{ko_name} {text_addr} {section_info}' \\
    -ex 'alias begin = target remote /dev/ttyUSB0' \\
    vmlinux
"""

print(debug_script)





