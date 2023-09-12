#!/bin/bash

# Install required packages
sudo apt update
sudo apt install nfs-common

# Setup NFS share
sudo mkdir {REMOTE_NFS_MOUNT_POINT}
sudo mount -t nfs {SERVER_HOST}:{SERVER_NFS_SHARE} {REMOTE_NFS_MOUNT_POINT}




