#!/bin/bash

sudo tee /etc/udev/rules.d/70-st-link.rules > /dev/null <<'EOF'
# ST-LINK V2
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2_%n"

# ST-LINK V2.1
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2-1_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2-1_%n"

# ST-LINK V3
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374d", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3loader_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374f", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3753", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
EOF

sudo udevadm control --reload-rules
sudo udevadm trigger
