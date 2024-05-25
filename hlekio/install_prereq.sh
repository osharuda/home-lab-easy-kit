#!/bin/bash

# Installing prerequisites:
sudo apt update
sudo apt install git bc bison flex libssl-dev make libc6-dev libncurses5-dev
# 32-bit
sudo apt install crossbuild-essential-armhf

# 64-bit
sudo apt install crossbuild-essential-arm64
