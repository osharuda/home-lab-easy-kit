#!/bin/bash
rm -rf ./.venv
python3 -m venv ./.venv
./.venv/bin/python3 -m pip install --upgrade pip
./.venv/bin/python3 -m pip install termcolor tabulate colorama
./.venv/bin/python3 -m pip install clang-html

