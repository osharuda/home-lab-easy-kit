#!/bin/bash

BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
LIME_YELLOW=$(tput setaf 190)
POWDER_BLUE=$(tput setaf 153)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
BLINK=$(tput blink)
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

ERR_PREFIX='${RED}🯀🯀🯀${NORMAL}  '
WRN_PREFIX="${YELLOW}🯁🯂🯃${NORMAL}  "
INF_PREFIX='🮌     '
RESULT_OK="[${GREEN}✓${NORMAL}]"
RESULT_FAIL="[${RED}𐄂${NORMAL}]"

function echo_info() {
	printf "${INF_PREFIX}${1}\n"
}

function echo_wrn() {
	printf "${WRN_PREFIX}${1}\n"
}

function echo_err() {
	printf "${ERR_PREFIX}${1}\n"
}

# param1: length of the line
# param2: character to repeat
function echo_line() {
	local n=${1}
	local s=${2}
	local i=0

	for (( ; i<${n}; i++ ))
	do 
		echo -n "${s}"
	done
}

# param1 - text to print
# param2 - optional, length of the text. Use it in the case of colour messages.
function echo_box() {
	local outer_offset=3
	local inner_offset=3
	local len=${#1}
	local text=${1}

	if [ ${2} ]; then
		len=${2}
	fi

	# New line
	echo ""

	# Top line
	echo_line ${outer_offset} ' '
	echo -n '╒'
	echo_line ${len}+${inner_offset}*2 '═'
	echo '╕'

	# Middle line
	echo_line ${outer_offset} ' '
	echo -n '│'
	echo_line ${inner_offset} ' '
	echo -n ${text}
	echo_line ${inner_offset} ' '
	echo '│'


	# Bottom line
	echo_line ${outer_offset} ' '
	echo -n '╘'
	echo_line ${len}+6 '═'
	echo '╛'
}

# Prints first part of the messages like this:
# Do somegning ......                  [OK]
# <---------- PART 1 -----------------><P2>
# param 1 - text to be printed
# param 2 - length of the part 2. Optional, if not specified value from previous call is used.
PROGRESS_PART1_LEN=40
function echo_progess_begin() {
	if [ ${2+1} ]; then 
		PROGRESS_PART1_LEN=${2}
	fi

	local len=${#1}
	echo -n "${1}"

	if [ $len -lt $PROGRESS_PART1_LEN ]; then 
		echo_line ${PROGRESS_PART1_LEN}-${len} " "
	fi
}

function echo_progess_ok() {
	echo "${RESULT_OK}"
}

function echo_progess_fail() {
	echo "${RESULT_FAIL}"
}
