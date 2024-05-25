#!/bin/bash
sudo su <<EOF
echo g > /proc/sysrq-trigger
EOF
