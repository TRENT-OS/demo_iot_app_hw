/*
   *  Main file of the ConfigServer component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include <camkes.h>

#include "LibDebug/Debug.h"

#include "create_config_backend.h"

/* Defines -------------------------------------------------------------------*/
#define DATABUFFER_CLIENT       (void *)logServer_dataport_buf

/* Private types -------------------------------------------------------------*/
static Log_filter_t filter;
static Log_emitter_callback_t reg;

static bool
logServer_init(void)
{
    // Wait until LogServer is ready to process logs.
    logServer_ready_wait();

    // Set up registered functions layer
    if (Log_emitter_callback_ctor(&reg, logServer_ready_wait,
                                  API_LOG_SERVER_EMIT) == false)
    {
        Debug_LOG_ERROR("Failed to set up registered functions layer");
        return false;
    }

    // Set up log filter layer
    if (Log_filter_ctor(&filter, Debug_LOG_LEVEL_DEBUG) == false)
    {
        Debug_LOG_ERROR("Failed to set up log filter layer");
        return false;
    }

    get_instance_Log_emitter(DATABUFFER_CLIENT, &filter, &reg);

    return true;
}


int run(void)
{
    if (logServer_init() == false)
    {
        printf("Failed to init logServer connection!\n");
        return -1;
    }

    Debug_LOG_INFO("Starting ConfigServer...");

    seos_err_t err = create_system_config_backend();
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("create_system_config_backend failed with:%d", err);
        return -1;
    }

    Debug_LOG_INFO("Config Server initialized.");

    return 0;
}
