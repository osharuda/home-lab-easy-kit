#!/bin/bash
# This script is inteded to be run on target Raspberry PI machine

. ./bashutil.sh

check_target_arch

gcc hlekio_test_in.c -o testin -ggdb
gcc hlekio_test_out.c -o testout -ggdb

echo ${IP}'Done!'
