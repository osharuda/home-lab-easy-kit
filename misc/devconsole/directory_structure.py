import re
from enum import Flag, auto
import os as os
import pytools

class NodeFlags(Flag):
    Optional = auto()
    SymLink = auto()
    MakeDirectory = auto()
    MakeCopy = auto()
    ProcessToolchainTemplate = auto()

class ProjectBuilder():

    def __init__(self, local_hlek_dir: str, local_cmsis_dir: str, mount_point: str, project_name: str):
        self.mount_point = mount_point
        self.local_hlek_dir = local_hlek_dir
        self.remote_hlek_dir = pytools.path_rebase(self.mount_point, self.mount_point, self.local_hlek_dir)
        self.local_cmsis_dir = local_cmsis_dir
        self.remote_cmsis_dir = pytools.path_rebase(self.mount_point, self.mount_point, self.local_cmsis_dir)
        self.project_name = project_name
        self.local_project_dir = os.path.join(mount_point, project_name)
        self.remote_project_dir = pytools.path_rebase(self.mount_point, self.mount_point, self.local_project_dir)
        self.project_structure = [
            ('customizer', NodeFlags.SymLink),
            ('devconsole', NodeFlags.Optional),
            ('doxygen', NodeFlags.Optional),
            ('firmware', NodeFlags.MakeDirectory),
            ('firmware/inc', NodeFlags.SymLink),
            ('firmware/ldscripts', NodeFlags.SymLink),
            ('firmware/src', NodeFlags.SymLink),
            ('firmware/flash.sh', NodeFlags.MakeCopy),
            ('libhlek', NodeFlags.MakeDirectory),
            ('libhlek/inc', NodeFlags.SymLink),
            ('libhlek/src', NodeFlags.SymLink),
            ('misc', NodeFlags.Optional),
            ('software', NodeFlags.SymLink),
            (('.', '.sh'), NodeFlags.SymLink),
            (('.', '.json'), NodeFlags.SymLink),
            (('.', '.md'), NodeFlags.SymLink),
            ('.clang-format', NodeFlags.SymLink)
        ]

    def process_item(self, s: str, flags) -> list:
        print(f'--- {s} : {flags}')
        result = []

        if NodeFlags.Optional in flags:
            return result

        if NodeFlags.MakeDirectory in flags:
            result += [f'mkdir "{os.path.join(self.remote_project_dir, s)}"']

        if NodeFlags.MakeCopy in flags:
            result += [f'cp "{os.path.join(self.remote_hlek_dir, s)}" "{os.path.join(self.remote_project_dir, s)}"']

        if NodeFlags.SymLink in flags:
            result += [f'ln -s "{os.path.join(self.remote_hlek_dir, s)}" "{os.path.join(self.remote_project_dir, s)}"']

        if NodeFlags.ProcessToolchainTemplate in flags:
            toolchan_fn = os.path.join(self.remote_project_dir, s)
            temp_fn = toolchan_fn + '.tmp'
            result += [f"sed -E 's?__STDPERIF_PATH__?{os.path.abspath(self.remote_cmsis_dir)}?g' {toolchan_fn} > {temp_fn}",
            f'rm {toolchan_fn}',
            f'mv {temp_fn} {toolchan_fn}']

        return result

    def make_project_tree(self):
        result = [f'mkdir "{self.remote_project_dir}"']
        for d, flags in self.project_structure:
            if type(d) is str:
                result += self.process_item(d, flags)
            elif type(d) is tuple:
                # Enumerate files
                dir = os.path.join(self.local_hlek_dir, d[0])
                for f in os.listdir(dir):
                    if f.endswith(d[1]):
                        abs_file_name = os.path.abspath(os.path.join(dir, f))
                        rel_file_name = os.path.relpath(abs_file_name, self.local_hlek_dir)
                        result += self.process_item(rel_file_name, flags)

        return result






