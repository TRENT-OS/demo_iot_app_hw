/*
   *  Partition manager init file for the use of the partition manager as a component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/

#include "api_pm.h"                 // include path to partition_manager must be set in cmakelists.txt
#include "ProxyNVM.h"               // needs seos libs
#include "ChanMux/ChanMuxClient.h"  // needs seos libs
#include "LibDebug/Debug.h"         // needs seos_libs

#include <camkes.h>


static ChanMuxClient chanmux;
static ProxyNVM proxy_NVM;

// Internal message buffer in ProxyNvm that should have the same size as the
// ChanMux dataport
static char proxy_NVM_message_buffer[PAGE_SIZE] = {0};


void api_pm_component__init(void)
{
    seos_err_t pm_stat;

    if (!ChanMuxClient_ctor(&chanmux,
                            0,  // ChanMux will resolve the channel number
                            (void*)chanMuxDataPort,
                            (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("Failed to construct testChanMuxClient!");
        return;
    }

    if ( !ProxyNVM_ctor(&proxy_NVM,
                        &chanmux,
                        proxy_NVM_message_buffer,
                        sizeof(proxy_NVM_message_buffer) ) )
    {
        Debug_LOG_ERROR("Failed to construct ProxyNVM!");
        return;
    }

    pm_stat = api_pm_partition_manager_init(&proxy_NVM);
    if (pm_stat != SEOS_SUCCESS)
    {
        Debug_LOG_DEBUG("Fail to init partition manager, ret: %d", pm_stat);
    }
}
