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


from tools import *
import os
import glob
from shutil import copy
from BaseCustomizer import *

class LibhlekCustomizer(BaseCustomizer):
    def __init__(self, common_config):
        super().__init__()
        self.hlek_name = "hlek"
        self.shared_headers = common_config["generation"]["shared"]
        self.install_path = common_config["global"]["install_path"]
        self.libhlek_name = "lib" + self.hlek_name
        self.libhlek_install_path = os.path.join(self.install_path, self.libhlek_name)
        self.sw_inc_templ = os.path.join(self.template_dir, "libhlek/inc")
        self.libhlek_cmakelists_path = os.path.join(self.libhlek_dest_path, self.cmake_script)

        self.add_template(os.path.join(self.fw_inc_templ, self.proto_header), [os.path.join(self.libhlek_inc_dest_path, self.proto_header)])
        self.add_template(os.path.join(self.libhlek_templ_path, self.cmake_script), [self.libhlek_cmakelists_path])
        self.add_common_headers()
        self.add_copy(os.path.join(self.hlekio_dir, self.hlekio_ioctl), [os.path.join(self.libhlek_inc_dest_path, self.hlekio_ioctl)])


    def add_common_headers(self):
        for customizer, info in self.shared_headers.items():
            hlek_lib_common_header, shared_header, fw_header, sw_header, shared_token = info

            self.add_template(os.path.join(self.sw_inc_templ, hlek_lib_common_header),
                              [os.path.join(self.libhlek_inc_dest_path, hlek_lib_common_header)])

            self.add_shared_code(os.path.join(self.shared_templ, shared_header), shared_token)

    def get_cmakelists_path(self):
        return self.libhlek_cmakelists_path

    def customize(self):
        self.vocabulary = self.vocabulary | {
                      "__HLEK_NAME__": self.hlek_name,
                      "__LIBHLEK_NAME__": self.libhlek_name,
                      "__LIBHLEK_INSTALL_PATH__": self.libhlek_install_path
                      }

        self.patch_templates()
