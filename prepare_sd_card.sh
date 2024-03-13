#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# SD-Card Partitioning script that can prepare SD cards with the partition table
# layout required by the IoT Demo
#
# Copyright (C) 2020-2021, HENSOLDT Cyber GmbH
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
FS_LABELS=(
    "BOOT"
    "CONFIGSRV"
    "LOGSRV"
)

#-------------------------------------------------------------------------------
function create_partition_table()
{
    local DEVICE_DIR=$1

    PARTED_COMMANDS=(
        mklabel msdos
        # create a 128MiB Boot partition
        mkpart primary 1MiB 129MiB
        # create a 128MiB ConfigServer partition
        mkpart primary 129MiB 257MiB
        # create a 1GiB LogServer partition
        mkpart primary 257MiB 1281MiB
    )

    parted --script ${DEVICE_DIR} ${PARTED_COMMANDS[@]}

    # if RPi 3 B+
    parted --script ${DEVICE_DIR} set 1 lba on
}

#-------------------------------------------------------------------------------
function format_fat_filesystem()
{
    local FS_LABEL=$1
    local DEVICE_DIR=$2

    mkfs.vfat -n ${FS_LABEL} ${DEVICE_DIR} > /dev/null 2>&1
}

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
DEVICE_DIR=${1:-}

if [ -z ${DEVICE_DIR} ]; then
    echo "ERROR: missing path to device"
    echo "Usage: ./create_partition_table.sh <path-to-device>"
    exit 1
fi

# Unmount all currently mounted partitions for the device before creating
# the new partition layout
umount -f ${DEVICE_DIR}?* > /dev/null 2>&1 || /bin/true
create_partition_table ${DEVICE_DIR}
sync

echo "Formatting filesystems"
PART_NUM=0
for FS_LABEL in ${FS_LABELS[@]}
do
   let "PART_NUM+=1"
   # Increase robustness by calling unmount again in case one of the partitions
   # potentially got remounted again in the meantime by some background service
   umount -f ${DEVICE_DIR}$PART_NUM > /dev/null 2>&1 || /bin/true
   format_fat_filesystem ${FS_LABEL} ${DEVICE_DIR}$PART_NUM
done

sync

# Remount all partitions of the device again
ls ${DEVICE_DIR}?* | xargs -n1 sudo -u $SUDO_USER udisksctl mount -b

echo "Created partition layout on device: ${DEVICE_DIR}"
