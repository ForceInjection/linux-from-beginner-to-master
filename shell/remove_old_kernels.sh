#!/bin/bash
# Run this script without any param for a dry run
# Run the script with root and with exec param for removing old kernels after checking
# the list printed in the dry run

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Unable to detect OS"
    exit 1
fi

uname -a
IN_USE=$(uname -r)
echo "Your in use kernel is $IN_USE"

if [[ "$OS" == "ubuntu" || "$OS" == "debian" ]]; then
    # For Debian/Ubuntu
    echo "Detected Debian-based system"

    OLD_KERNELS=$(
        dpkg --list |
            grep -v "$IN_USE" |
            grep -Ei 'linux-image|linux-headers|linux-modules' |
            awk '{ print $2 }'
    )
    echo "Old Kernels to be removed:"
    echo "$OLD_KERNELS"

    if [ "$1" == "exec" ]; then
        for PACKAGE in $OLD_KERNELS; do
            yes | apt purge "$PACKAGE"
        done
    else
        echo "If all looks good, run it again like this: sudo remove_old_kernels.sh exec"
    fi

elif [[ "$OS" == "centos" || "$OS" == "rhel" || "$OS" == "fedora" ]]; then
    # For CentOS/RHEL/Fedora
    echo "Detected RedHat-based system"

    OLD_KERNELS=$(
        rpm -qa | 
        grep -Ei 'kernel-[0-9]' | 
        grep -v "$IN_USE" | 
        sort -V
    )
    echo "Old Kernels to be removed:"
    echo "$OLD_KERNELS"

    if [ "$1" == "exec" ]; then
        for PACKAGE in $OLD_KERNELS; do
            echo "Removing $PACKAGE"
            yum -y remove "$PACKAGE" || dnf -y remove "$PACKAGE"
        done
    else
        echo "If all looks good, run it again like this: sudo remove_old_kernels.sh exec"
    fi
else
    echo "Unsupported OS: $OS"
    exit 1
fi