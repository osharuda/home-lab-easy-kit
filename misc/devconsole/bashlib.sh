#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

install_package() {
	dpkg -s $1
	local res=$? 
	echo $res
	if [ $res -ne 0 ]; then
		set -e
		echo "installing $1"
		sudo apt-get -y install $1
		set +e
	else
		echo "$1 is already installed."
	fi
}

update_message() {
	echo $1 >> status_msg.txt
}