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
import os.path
from typing import Pattern

class SourceFileParser:
    def __init__(self, file_name: str):
        self.ext_info = {
            ".py": "# ",
            ".h": "// ",
            ".hpp": "// ",
            ".c": "// ",
            ".cpp": "// "}
        self.file_name_token = "///"
        self.file_name = file_name
        self.file_base, self.file_ext = os.path.splitext(file_name)
        if self.file_ext not in self.ext_info:
            raise RuntimeError('Source: "{0}" is not supported'.format(file_name))
        self.comment = self.ext_info[self.file_ext]
        self.lines = []
        self.load()
        self.line_end = '\n'

        return

    def load(self):
        with open(self.file_name, "r") as f:
            self.lines = f.readlines()
        return

    def store(self):
        if not self.lines:
            raise RuntimeError('Can''t store empty file: "{0}"'.format(self.file_name))
        with open(self.file_name,"w") as f:
            f.writelines(self.lines)
        return

    def add(self, index: int, code_block):
        if not code_block:
            raise RuntimeError('May not add empty code block in "{0}"'.format(self.file_name))

        if isinstance(code_block, str):
            code = code_block.splitlines(keepends=True)
        elif isinstance(code_block, list):
            code = []
            for i in code_block:
                s = str(i)
                if s and s[-1] != self.line_end:
                    s += self.line_end
                code.insert(len(code), s)
        else:
            raise RuntimeError('Unsupported argument')

        self.lines[index:index] = code

    def remove(self, start: int, end: int):
        del self.lines[start:end+1]
        return

    def find(self, expression) -> int:
        index = 0
        if isinstance(expression, str):
            strip_exp = expression.strip()
            for l in self.lines:
                if l.strip() == strip_exp:
                    return index
                index += 1
        elif isinstance(expression, Pattern):
            for l in self.lines:
                if expression.match(l):
                    return index
                index += 1
        else:
            raise RuntimeError('Unsupported expression argument')
        raise RuntimeError('Expression not found')

    def print(self):
        for l in self.lines:
            print(l, end='')



