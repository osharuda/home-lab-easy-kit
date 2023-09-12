#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

. "${SCRIPT_DIR}"/bashlib.sh

NFS_LOCATION=$1

install_package nfs-common
install_package nfs-kernel-server 

set -e
tmpdir=$(mktemp -d /mnt/test_nfs.XXXXXX)
set +e

sudo mount -v -t nfs "${NFS_LOCATION}" "${tmpdir}"
res=$?
if [ $res -ne 0 ]; then
	sudo rmdir ${tmpdir}
	echo FAILURE
	exit 1
fi
echo SUCCESS
sudo umount ${tmpdir}
sudo rmdir ${tmpdir}	



