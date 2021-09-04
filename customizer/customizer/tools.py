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
