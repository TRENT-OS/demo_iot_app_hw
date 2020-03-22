#!/bin/bash -em

#-------------------------------------------------------------------------------
#
# Script to run a demo using TAP devices
#
# Copyright (C) 2020, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
function create_tap_device()
{
    # create TAP devices. Ethernet connection is required for this step!
    ip tuntap add tap0 mode tap user $SUDO_USER
    ip link add br0 type bridge
    ip link set tap0 master br0
    ip link set dev enp0s31f6 down
    ip addr flush dev enp0s31f6
    ip link set enp0s31f6 master br0
    ip link set dev br0 up
    ip link set tap0 up
    ip link set dev enp0s31f6 up

    iptables -A INPUT -i tap0 -j ACCEPT
    iptables -A INPUT -i br0 -j ACCEPT
    iptables -A FORWARD -i br0 -j ACCEPT

    echo "created TAP0 device"
}

#-------------------------------------------------------------------------------
USAGE_STRING="run_demo.sh <path-to-project-build> <path-to-proxy>\n
This script runs demo applications that require TAP\n"
OPT_INTERACTIVE="-it"


if [ "$1" == "" ]; then
    echo -e "${USAGE_STRING}"
    exit 1
fi

PROJECT_PATH=$1
PROXY_PATH=$2

if [ -z ${PROJECT_PATH} ]; then
    echo "ERROR: missing project path"
    exit 1
fi

# default is the zynq7000 platform
IMAGE_PATH=${PROJECT_PATH}/images/capdl-loader-image-arm-zynq7000
if [ ! -f ${IMAGE_PATH} ]; then
    echo "ERROR: missing project image ${IMAGE_PATH}"
    exit 1
fi

if [ ! -f ${PROXY_PATH}/proxy_app ]; then
    echo "ERROR: proxy application path missing!"
    exit 1
fi

shift 2

# check if TAP device is already created and if not, set one up
if [ ! -d "/sys/class/net/tap0/" ]; then
  echo "TAP devices not created yet. Setting up TAP0..."
  create_tap_device
fi

QEMU_PARAMS=(
    -machine xilinx-zynq-a9
    -m size=512M
    -nographic
    -s
    -S
    -serial pty
    -serial mon:stdio
    -kernel ${IMAGE_PATH}
)

# run QEMU
qemu-system-arm  ${QEMU_PARAMS[@]} $@ 2> qemu_stderr.txt &

# try to retrieve the PTS to be given to the proxy app from qemu
while [ -z "${PTS}" ]; do
    if [ -f qemu_stderr.txt ]; then
        PTS=$(cat qemu_stderr.txt | grep -o "/dev/pts/[0-9]" || continue)
    fi
done

# start proxy app
${PROXY_PATH}/proxy_app -c PTY:${PTS} -t 1  > seos_proxy_app.out &

sleep 1
echo "Please hit 'Ctrl-a+c' an then 'c+Enter' to let the holding instance of qemu to continue."
echo "You can stop qemu with 'Ctrl-a+x', then you can stop the proxy app with 'Ctrl-c".
echo
fg
fg
