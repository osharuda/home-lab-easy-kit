#   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
#
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import json
import hashlib
import os
from typing import Pattern
import inspect
import re


def concat_lines(l: list) -> str:
    line_separator = '\n'
    return line_separator.join(l)


def get_leaf_values(d) -> list:
    res = list()
    for v in d.values():
        if isinstance(v, dict):
            # this is dict iterate through recursively
            res.extend(get_leaf_values(v))
        else:
            res.append(v)

    return res


def check_duplicates(l: list) -> bool:
    return len(l) != len(set(l))


def check_instance_count(dev_name: str, dev_data_list: list, count: int) -> bool:
    dev_cnt = len(dev_data_list)
    if dev_cnt == 0:
        return False
    elif dev_cnt > count:
        raise RuntimeError(
            "Error: {0} doesn't support {1} devices per mcu. {2} devices are supported".format(devname, dev_cnt, count))
    return True


def get_duplicates(l: list) -> list:
    uniq = set()
    dups = set()

    for i in l:
        if i not in uniq:
            uniq.add(i)
        else:
            dups.add(i)

    return list(dups)


def int_to_bool(x: int) -> str:
    if x == 0:
        return "false"
    else:
        return "true"


def check_buffer_size_multiplicity(bsize: int, factor: int) -> bool:
    return bsize == (bsize // factor) * factor


def add_to_values(d: dict, prefix=None, suffix=None):
    for k, v in d.items():
        if prefix:
            v = prefix + v
        if suffix:
            v = v + suffix
        d[k] = v


def set_to_ordered_list(st: set) -> list:
    l = list(st)
    l.sort()
    return l


def hash_dict_as_c_array(d: dict) -> str:
    s = json.dumps(d, sort_keys=True)
    hsh = hashlib.sha1()
    hsh.update(s.encode('utf-8'))
    digest = hsh.digest()
    l = list()
    for i in range(0, hsh.digest_size):
        l.append("0x{:02X}".format(int(digest[i])))

    return (", ".join(l), hsh.digest_size)


def hash_string(s: str) -> str:
    hsh = hashlib.sha1()
    hsh.update(s.encode('utf-8'))
    digest = hsh.digest()
    result = str()
    for i in range(0, hsh.digest_size):
        result += "{:02X}".format(int(digest[i]))

    return result


def list_dir_with_rexp(d: str, p) -> str:
    plist = []
    result = []

    if isinstance(p, Pattern):
        plist.append(p)
    elif isinstance(p, list):
        plist = p
    else:
        raise RuntimeError('Not a valid Pattern: {0}'.format(str(p)))

    for pe in plist:
        if not isinstance(pe, Pattern):
            raise RuntimeError("One of the items is not a valid Pattern: {0}".format(str(pe)))

    for root, dirs, files in os.walk(d):
        for fn in files:
            for pe in plist:
                if pe.match(fn):
                    result.append((root, fn))
                    break
    return result


def current_module_path() -> str:
    filename = inspect.getframeinfo(inspect.currentframe()).filename
    return os.path.dirname(os.path.abspath(filename))


def read_text_file(fn: str) -> str:
    with open(fn) as f:
        return f.read()


def write_text_file(fn: str, text: str) -> str:
    if os.path.isfile(fn):
        os.remove(fn)

    with open(fn, "a") as f:
        return f.write(text)

    return

def patch_text(t: str, d: dict) -> str:
    for k,v in d.items():
        t = t.replace(str(k), str(v))
    return t

def doxygen_handle_list_of_defines(s: str, define: str, add: bool, newline='\n') -> str:
    trans = str.maketrans({'\n': None, '\r': None, '\t': None, '\\': None})
    s = s.translate(trans)
    wl = s.split(' ')
    wl = list(filter(lambda emp: len(emp) > 0, wl))

    if len(wl) < 2:
        raise RuntimeError('Bad define: too small: ' + ' '.join(wl))

    if wl[0] == define or wl[1] == define:
        raise RuntimeError('Bad define: may not be "=" or variable name: ' + define)

    if add == (define in s):
        verb = 'present ' if add else 'not present '
        raise RuntimeError('Define "' + define + '" is ' + verb + 'in the code')

    if add:
        wl.append(define)
    else:
        wl.remove(define)

    s = "{0} = ".format(wl[0])

    if len(wl) > 2:
        padding = ' ' * len(s)
        for i in range(2, len(wl)):
            s += wl[i] + ' \\' + newline + padding

        s = s[:-(len(padding) + 1 + len(newline))] + newline
    else:
        s += newline
    return s


def cxx_handle_macro_enum(code: str, name: str, vtype: str, add: bool, newline='\n') -> str:
    def_re = re.compile(r'#define\s+([^\s]+)\s+\(' + vtype + r'\)(\d+)\s*\n')

    macro_dict = dict()
    rev_macro_dict = dict()
    max_val = 0
    exist_val = None

    # build maps for existing values
    for m in def_re.findall(code):
        macro = m[0]
        value = int(m[1])

        if macro in macro_dict:
            raise RuntimeError("Duplicate macro: {0}".format(macro))

        if value in rev_macro_dict:
            raise RuntimeError("Duplicate value: {0}".format(value))

        if macro == name:
            exist_val = value

        if value > max_val:
            max_val = value

        macro_dict[macro] = value
        rev_macro_dict[value] = macro

    # Process information
    if add:
        if name in macro_dict:
            raise RuntimeError("Macro already added: {0}".format(name))
        macro_dict[name] = max_val + 1
        rev_macro_dict[max_val + 1] = name
    else:
        if name not in macro_dict:
            raise RuntimeError("No such macro: {0}".format(name))
        del macro_dict[name]
        del rev_macro_dict[exist_val]

    max_macro_len = 0
    for m in macro_dict.keys():
        if len(m)>max_macro_len:
            max_macro_len = len(m)

    # Build macro block back
    mlist = []
    for value in sorted(rev_macro_dict):
        macro = rev_macro_dict[value]
        padding = " " * (max_macro_len-len(macro))
        macro += padding
        mlist.append("#define {0} (uint8_t){1}".format(macro, value))
    return newline.join(mlist)+newline


