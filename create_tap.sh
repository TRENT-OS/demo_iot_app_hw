#!/bin/sh

ip tuntap add tap0 mode tap user $SUDO_USER
ip tuntap add tap1 mode tap user $SUDO_USER
ip link add br0 type bridge
ip link set tap0 master br0
ip link set tap1 master br0
ip link set dev enp0s31f6 down
ip addr flush dev enp0s31f6
ip link set enp0s31f6 master br0
ip link set dev br0 up
ip link set tap0 up
ip link set tap1 up
ip link set dev enp0s31f6 up


iptables -A INPUT -i tap0 -j ACCEPT
iptables -A INPUT -i tap1 -j ACCEPT
iptables -A INPUT -i br0 -j ACCEPT
iptables -A FORWARD -i br0 -j ACCEPT

dhclient -v br0
