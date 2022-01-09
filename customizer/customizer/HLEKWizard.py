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
from SourceFileParser import *
import re

class HLEKWizard:
    def __init__(self, dev_name):
        self.hlek_root = os.path.dirname(os.path.dirname(current_module_path()))

        self.wizards_path = os.path.join(self.hlek_root, "customizer", "wizards")

        self.new_file_list = dict()
        self.mod_file_list = dict()
        self.pymod_file_list = dict()
        self.substitutions = dict()
        self.open_sub_comment = "-> "
        self.close_sub_comment = "<- "

        # Substitiotions
        self.add_substitution("{DEVNAME}", dev_name.upper())
        self.add_substitution("{devname}", dev_name.lower())
        self.add_substitution("{DevName}", dev_name)

        return

    def add_new_file(self, rel_file_name: str, template: str):
        abs_file_name = os.path.join(self.hlek_root, rel_file_name)

        if abs_file_name in self.new_file_list:
            raise RuntimeError('File "{0}" already specified as new file'.format(abs_file_name))

        if abs_file_name in self.mod_file_list:
            raise RuntimeError('File "{0}" already specified as mod file'.format(abs_file_name))

        self.new_file_list[abs_file_name] = template;
        return

    def add_mod_file(self, rel_file_name: str, mod_token: str, modification: str):
        abs_file_name = os.path.join(self.hlek_root, rel_file_name)

        if abs_file_name in self.new_file_list:
            raise RuntimeError('File "{0}" already specified as new file'.format(abs_file_name))

        d = self.mod_file_list.setdefault(abs_file_name, dict())
        if mod_token in d:
            raise RuntimeError('Token "{0}" was already added to the file "{1}"'.format(mod_token, abs_file_name))

        d[mod_token] = modification

    def add_pymod_file(self, rel_file_name: str, mod_token: str, modification: str):
        abs_file_name = os.path.join(self.hlek_root, rel_file_name)

        if abs_file_name in self.new_file_list:
            raise RuntimeError('File "{0}" already specified as new file'.format(abs_file_name))

        d = self.pymod_file_list.setdefault(abs_file_name, dict())
        if mod_token in d:
            raise RuntimeError('Token "{0}" was already added to the file "{1}"'.format(mod_token, abs_file_name))

        d[mod_token] = modification

    def add_substitution(self, subst_token: str, subst_value: str):
        if subst_token in self.substitutions:
            raise RuntimeError('Substitution already added: "{0}""'.format(subst_token))
        self.substitutions[subst_token] = subst_value
        return

    def load(self, wizdir: str):
        add_file_rexp = re.compile(r'ADD\..+')
        mod_file_rexp = re.compile(r'MOD\.\{\{([^.\{\}]+)\}\}\..+')
        pymod_file_rexp = re.compile(r'PYMOD\.\{\{([^.\{\}]+)\}\}\..+')
        self.wiz_path = os.path.join(self.wizards_path, wizdir)
        self.survey_dict = []

        wiz_files = list_dir_with_rexp(self.wiz_path, [add_file_rexp, mod_file_rexp, pymod_file_rexp])
        for d, f in wiz_files:
            abs_wiz_fname = os.path.join(d, f)
            dst_file_name = patch_text(f, self.substitutions)
            template = read_text_file(abs_wiz_fname)
            rpath = os.path.relpath(d, self.wiz_path)
            if rpath == '.':
                rpath = ''
            else:
                rpath = rpath + os.path.sep

            if add_file_rexp.match(f):
                rel_dst_fname = rpath + dst_file_name[4:]
                self.add_new_file(rel_dst_fname, template)
            elif g := mod_file_rexp.match(f):
                tok = g.group(1)
                rel_dst_fname = rpath + dst_file_name[9 + len(tok):]
                self.add_mod_file(rel_dst_fname, tok, template)
            elif g := pymod_file_rexp.match(f):
                tok = g.group(1)
                rel_dst_fname = rpath + dst_file_name[11 + len(tok):]
                self.add_pymod_file(rel_dst_fname, tok, template)
            else:
                raise RuntimeError('Unknown wizard file detected: "{0}"'.format(abs_wiz_fname))

        return

    def check_files(self, exist=False):
        err_nouns = {True: "doesn't exist",
                     False: "exist"}

        for f, t in self.new_file_list.items():
            if os.path.isfile(f) != exist:
                raise RuntimeError("File {0} {1}".format(f, err_nouns[bool(exist)]))

        for f, d in self.mod_file_list.items():
            if not os.path.isfile(f):
                raise RuntimeError("File {0} {1}".format(f, err_nouns[True]))

    def do_survey(self):
        # Load survey
        survey_path = os.path.join(self.wiz_path, "survey.py")
        _locals = locals()
        exec(read_text_file(survey_path), globals(), locals())
        self.survey_dict = _locals["survey_dict"]

        for i in self.survey_dict:
            question, kv, options, defval = i

            if defval not in options.keys():
                raise RuntimeError('Default value is not found for question "{0}"'.format(question))

            s = ""
            for k in options.keys():
                if k != defval:
                    s += k + ', '
            s += defval + " by default"

            while answer := input('{0} ( {1} ): '.format(question, s)):
                if not answer or answer in options.keys():
                    break
            if not answer:
                answer = defval

            k = patch_text(kv, self.substitutions)
            v = patch_text(options[answer], self.substitutions)

            if k in self.substitutions:
                raise RuntimeError('{0} key from survey.py is found in substitutions'.format(k))

            self.substitutions[k]=v
        return


    def run(self, remove=False):
        self.check_files(remove)

        if not remove:
            self.do_survey()

        for f, t in self.new_file_list.items():

            if remove:
                os.remove(f)
            else:
                s = patch_text(t, self.substitutions)
                write_text_file(f, s)

        for f, d in self.mod_file_list.items():
            modfile = SourceFileParser(f)
            for mt, t in d.items():
                open_comment = modfile.make_comment(self.open_sub_comment + mt)
                close_comment = modfile.make_comment(self.open_sub_comment + mt)

                if remove:
                    open_index = modfile.find(open_comment)
                    close_index = modfile.find(close_comment)
                    modfile.remove(open_index, close_index)
                else:
                    ins_token = modfile.make_comment(mt, "")
                    ins_index = modfile.find(ins_token)
                    s = patch_text(t, self.substitutions)
                    modfile.add(ins_index, [open_comment, s, close_comment])

                modfile.store()

        for f, d in self.pymod_file_list.items():
            pymodfile = SourceFileParser(f)

            for mt, t in d.items():
                # init code block mode
                code_block = pymodfile.find_code_block(mt)

                # Load & execute python code from wizrd file
                _locals = locals()
                _globals = globals()
                pycode = patch_text(t, self.substitutions)
                exec(pycode, _globals, _locals)

                # Some variables
                _globals["code_block"] = code_block
                _globals["remove_data"] = remove
                _globals["newline"] = pymodfile.line_end

                # Some functions
                _globals["cxx_handle_macro_enum"] = cxx_handle_macro_enum
                _globals["doxygen_handle_list_of_defines"] = doxygen_handle_list_of_defines

                # Call handler
                exec("code_block = patch(code_block, remove_data)", _globals, _locals)
                code_block = _locals["code_block"]

                # Apply result
                pymodfile.fix_code_block(code_block)

            pymodfile.store()

        return
