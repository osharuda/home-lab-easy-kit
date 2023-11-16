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

def get_project_root() -> str:
    return os.path.abspath(get_env_var("PROJECTDIR"))

def get_env_var(name: str) -> str:
    if name not in os.environ:
        raise RuntimeError(f'Environment variable "{name}" is not set!')

    return os.environ[name]

def get_leaf_values(d) -> list:
    res = list()
    for v in d.values():
        if isinstance(v, dict):
            # this is dict iterate through recursively
            res.extend(get_leaf_values(v))
        else:
            res.append(v)

    return res




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

class Sorted_JSON_Encoder(json.JSONEncoder):
    def default(self, item):
        if isinstance(item, set):
            result = list(item)
            result.sort()
            return result
        else:
            return json.JSONEncoder.default(self, item)

def hash_dict_as_c_array(d: dict) -> str:
    digest = hash_dict_as_bytes(d)
    l = list()
    for i in range(0, len(digest)):
        l.append("0x{:02X}".format(int(digest[i])))

    return (", ".join(l), len(digest))


def hash_text(s: str) -> bytes:
    hsh = hashlib.sha1()
    hsh.update(s.encode('utf-8'))
    return hsh.digest()


def hash_text_file(fn: str)-> str:
    if os.path.isfile(fn):
        s = read_text_file(fn)
        return hash_text(s).hex()
    else:
        return ""

def hash_dict_as_bytes(d: dict) -> str:
    s = json.dumps(d, sort_keys=True, cls=Sorted_JSON_Encoder)
    digest = hash_text(s)
    return digest

def hash_merge_digests(dl: list[bytes]):
    len = len(dl[0])
    result = bytes(len)
    for d in dl:
        if len(d) != len:
            raise RuntimeError('Digest length mismatch')

        for i in range(0, len):
            result[i] = result[i] ^ d[i]

    return result



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


def frequency_to_int(f: str) -> int:
    unit = f[-3:].lower()
    if unit == "khz":
        return int(f[:-3])*1000
    elif unit == "mhz":
        return int(f[:-3]) * 1000000
    elif unit == "ghz":
        return int(f[:-3]) * 1000000000
    else:
        return int(f)


arg_switch_re = re.compile(r'^--([a-zA-Z0-9]+-)*[a-zA-Z0-9]+$')
def is_switch_arg(a: str)-> bool:
    res = bool(arg_switch_re.match(a))
    return res

arg_equal_re = re.compile(r'^--([a-zA-Z0-9]+-)*[a-zA-Z0-9]+=([^=]+)$')
def is_equal_arg(a: str)-> str:
    m = arg_equal_re.match(a)
    if m:
        value = m.group(2)
        key = a[0: len(a)-len(value)-1]
        return key, value
    else:
        return None, None


def parse_cmd_line(argv: list[str], defaults: dict) -> dict:
    result = defaults.copy()
    available_keys = set()
    for a in argv:
        if is_switch_arg(a):
            if a not in result or\
                    type(result[a][0]) is not bool:
                raise RuntimeError(f'Invalid argument "{a}"')
            else:
                available_keys.add(a)
                result[a][0] = bool(not result[a][0])
                continue

        key, value = is_equal_arg(a)

        if not key:
            raise RuntimeError(f'Invalid argument "{key}"')

        if type(result[key][0])==str:
            result[key][0] = value
        elif type(result[key][0]==int):
            result[key][0] = int(value)
        else:
            raise RuntimeError(f'Invalid argument "{key}"')
        available_keys.add(key)

    for a in available_keys:
        incompatible_keys = result[a][1]
        for i in incompatible_keys:
            if i in available_keys:
                raise RuntimeError(f'Arguments {a} and {i} are not compatible.')

    return result


#region VALIDATION TOOLS

def check_items(d: dict,
               required_items: dict[str, type],
               optional_items: dict[str, type]=None):
    for ri in required_items.items():
        key = ri[0]
        if key not in d.keys():
            raise RuntimeError(f'required key "{key}" is not found')

    all_items = required_items
    if optional_items:
        all_items.update(optional_items)

    for key, item in d.items():
        if key not in all_items.keys():
            raise RuntimeError(f'"{key}" is not allowed')
        if type(item) != all_items[key]:
            raise RuntimeError(f'"type of the {key}" key is not {str(item)}')


def check_buffer_size_multiplicity(bsize: int, factor: int) -> bool:
    return bsize == (bsize // factor) * factor

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

#endregion