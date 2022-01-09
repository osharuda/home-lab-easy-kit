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
from tools import hash_string

class SourceFileParser:
    def __init__(self, file_name: str):
        self.ext_info = {
            ".py": "#",
            ".h": "//",
            ".hpp": "//",
            ".c": "//",
            ".cpp": "//",
            ".conf": "#",
            ".txt": "#"}
        self.file_name_token = "///"
        self.file_name = file_name
        self.file_base, self.file_ext = os.path.splitext(file_name)
        if self.file_ext not in self.ext_info:
            raise RuntimeError('Source: "{0}" is not supported'.format(file_name))
        self.comment = self.ext_info[self.file_ext]
        self.lines = []
        self.line_end = '\n'
        self.prefix = None
        self.suffix = None
        self.code_block = False

        self.load()
        return

    def load(self):
        if os.path.isfile(self.file_name):
            with open(self.file_name, "r", newline=self.line_end) as f:
                self.lines = f.readlines()
        return

    def store(self):
        if not self.lines:
            raise RuntimeError('Can''t store empty file: "{0}"'.format(self.file_name))
        with open(self.file_name,"w", newline=self.line_end) as f:
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

                lines = s.splitlines(keepends=True)
                code.extend(lines)
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
            raise RuntimeError('Unsupported expression argument: {0}'.format(expression))
        raise RuntimeError('Expression not found: {0} in a file "{1}"'.format(expression, self.file_name))

    def make_comment(self, s: str, comment_hash=None):
        if isinstance(comment_hash, str) and len(comment_hash) == 0:
            # no hash
            return format('{0} {1}{2}'.format(self.comment, s, self.line_end))
        elif comment_hash is None:
            # hash from s
            return format('{0} {1} | HASH: {2}{3}'.format(self.comment, s, str(hash_string(s)), self.line_end))
        else:
            # hash from parameter
            return format('{0} {1} | HASH: {2}{3}'.format(self.comment, s, str(comment_hash), self.line_end))

    def find_code_block(self, token: str) -> str:
        comment = "{0} {1}".format(self.comment, token)
        comment_len = len(comment)
        text = ''.join(self.lines)
        start = text.find(comment)

        if start == -1:
            raise RuntimeError("Can't find opening '{0}'' comment".format(comment))

        start = text.find(self.line_end, start + comment_len)
        if start == -1:
            raise RuntimeError("Can't find line ending for the line with opening comment: '{0}'".format(comment))
        start += 1

        end = text.find(comment, start)
        if end == -1:
            raise RuntimeError("Can't find closing '{0}'' comment".format(comment))

        dup = text.find(comment, end + comment_len)
        if dup != -1:
            raise RuntimeError("More than two '{0}'' comments detected".format(comment))

        self.prefix = text[:start]
        self.suffix = text[end:]
        self.code_block = True

        return text[start: end]

    # Requires previous call to find_code_block
    def fix_code_block(self, code_block):
        if not self.code_block:
            raise RuntimeError("find_code_block() must be called first.")

        text = self.prefix + code_block + self.suffix
        self.lines = map(lambda l: l + self.line_end, text.splitlines())
        self.code_block = False
        return

    def print(self):
        for l in self.lines:
            print(l, end='')



