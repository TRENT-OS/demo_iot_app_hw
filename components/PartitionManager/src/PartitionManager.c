/*
   *  Partition manager init file for the use of the partition manager as a component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#include "api_pm.h"

#include "RamDisk.h"
#include "DiskData.h"

static RamDisk_Config_t ramCfg = {
    .mode       = RamDisk_Init_COPY_BUFFER,
    .compressed = true,
    .size       = sizeof(diskData),
    .ptr        = diskData,
};
static RamDisk_t* ramDisk;

void api_pm_component__init(void)
{
    OS_Error_t err;

    if ((err = RamDisk_ctor(&ramDisk, &ramCfg)) != OS_SUCCESS)
    {
        Debug_LOG_ERROR("RamDisk_ctor() failed with %d", err);
    }

    if ((err = api_pm_partition_manager_init(ramDisk)) != OS_SUCCESS)
    {
        Debug_LOG_ERROR("api_pm_partition_manager_init() failed with %d", err);
    }
}