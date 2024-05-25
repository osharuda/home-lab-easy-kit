#!/bin/bash

. ./bashutil.sh
check_env

rm -rf linux
tar xzf ./backup/linux_kernel_compiled.tar.gz
