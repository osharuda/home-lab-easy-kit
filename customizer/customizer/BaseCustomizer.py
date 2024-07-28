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

import os
from tools import *
from shutil import copy2

class BaseCustomizer:
    def __init__(self):
        self.template_list  = dict()
        self.file_copy_list = dict()
        self.shared_code    = dict()
        self.project_dir = get_project_root()
        self.cmake_script = "CMakeLists.txt"
        self.customizer_dir = os.path.join(self.project_dir, "customizer")
        self.template_dir = os.path.join(self.customizer_dir, "templates")
        self.shared_templ = os.path.join(self.template_dir, "shared")
        self.libhlek_templ_path = os.path.join(self.template_dir, "libhlek")
        self.libhlek_dest_path = os.path.join(self.project_dir, "libhlek")
        self.libhlek_inc_dest_path = os.path.join(self.libhlek_dest_path, "inc")
        self.proto_header = "i2c_proto.h"
        self.fw_inc_templ = os.path.join(self.template_dir, "firmware/inc")
        self.fw_path = os.path.join(self.project_dir, "firmware")
        self.fw_inc_source_path = os.path.join(self.fw_path, "inc")
        self.fw_src_source_path = os.path.join(self.fw_path, "src")
        self.sw_testtool_dest = os.path.join(self.project_dir, "software/testtool")
        self.hlekio_ioctl = "hlekio_ioctl.h"
        self.hlekio_dir = os.path.join(get_project_root(), "hlekio")


        self.device_context_initializer = "{0}"
        self.sequential_lock_initializer = "{0,0}"
        self.circular_buffer_initializer = f"{{ {{0}}, {{0}}, 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }}"

        # get info uuid length
        h, hash_len = hash_dict_as_c_array("")

        self.vocabulary = {
            "__INFO_UUID_LEN__": hash_len,
            "__COMM_MAX_DEV_ADDR__": 15,
            "__I2C_FIRMWARE_ADDRESS__": 0,
            "__COMM_BUFFER_LENGTH__": 0
        }

    def customize(self):
        pass

    def add_template(self, in_file: str, out_file_list: list):
        self.template_list[in_file] = out_file_list

    def add_copy(self, in_file: str, out_file_list: list):
        self.file_copy_list[in_file] = out_file_list

    def add_shared_code(self, in_file: str, token: str):
        header_separator = "/* --------------------> END OF THE TEMPLATE HEADER <-------------------- */"
        with open(os.path.abspath(in_file), 'r') as f:
            template = f.read()

            # cut the file header
            indx = template.find(header_separator)

            self.shared_code[token] = template[indx+len(header_separator):]

    def extracted_from_file(self, in_file: str, open_token: str, close_token) -> str:
        with open(os.path.abspath(in_file), 'r') as f:
            source = f.read()
            begin_index = source.find(open_token) + len(open_token)
            if begin_index < 0:
                raise RuntimeError(f'Open token "{open_token}" was not found')
            end_index = source.find(close_token, begin_index)
            if end_index < 0:
                raise RuntimeError(f'Open token "{close_token}" was not found')

            return source[begin_index: end_index]


    def patch_templates(self):
        for in_file, out_file_list in self.template_list.items():
            # read in_file
            with open(os.path.abspath(in_file), 'r') as f:
                template = f.read()

            # patch shared code first
            for k,v in self.shared_code.items():
                template = template.replace("{"+k+"}", v)

            # patch tokens
            template = template.format(**self.vocabulary)

            for fn in out_file_list:
                with open(os.path.abspath(fn), 'w') as f:
                    f.write(template)

        # Process file copy
        for in_file, out_file_list in self.file_copy_list.items():
            for fn in out_file_list:
                copy2(in_file, fn)
