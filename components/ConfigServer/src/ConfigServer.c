/*
   *  Main file of the ConfigServer component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include <camkes.h>

#include "LibDebug/Debug.h"
#include "create_config_backend.h"

void post_init(void)
{
    Debug_LOG_INFO("Starting ConfigServer...");

    seos_err_t err = create_system_config_backend();
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("create_system_config_backend() failed with:%d", err);
        return;
    }

    Debug_LOG_INFO("Config Server initialized.");

    return;
}
