#!/bin/sh

ip tuntap del dev tap0 mode tap
ip tuntap del dev tap1 mode tap
ip link delete enp0s31f6 type bridge
ip link delete br0 type bridge
ip link set dev enp0s31f6 up
dhclient -v enp0s31f6

