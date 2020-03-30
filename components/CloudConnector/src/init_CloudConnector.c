/*
   *  Init file for the CloudConnector.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include <camkes.h>

#include "LibDebug/Debug.h"

#include "OS_ConfigService.h"

/* Defines -------------------------------------------------------------------*/
#define DATABUFFER_CLIENT       (void *)logServer_dataport_buf


/* Instance variables --------------------------------------------------------*/
OS_ConfigServiceHandle_t serverLibWithMemBackend;

static Log_filter_t filter;
static Log_emitter_callback_t reg;


bool
logServer_init(void)
{
    // Wait until LogServer is ready to process logs.
    logServer_ready_wait();

    // set up registered functions layer
    if (Log_emitter_callback_ctor(&reg, logServer_ready_wait,
                                  API_LOG_SERVER_EMIT) == false)
    {
        Debug_LOG_ERROR("Failed to set up registered functions layer");
        return false;
    }

    // set up log filter layer
    if (Log_filter_ctor(&filter, Debug_LOG_LEVEL_DEBUG) == false)
    {
        Debug_LOG_ERROR("Failed to set up log filter layer");
        return false;
    }

    get_instance_Log_emitter(DATABUFFER_CLIENT, &filter, &reg);

    return true;
}

seos_err_t
init_config_handle(OS_ConfigServiceHandle_t* configHandle)
{
    seos_err_t err = SEOS_SUCCESS;

    err = OS_ConfigService_createHandle(
              OS_CONFIG_HANDLE_KIND_RPC,
              0,
              configHandle);
    if (err != SEOS_SUCCESS)
    {
        return err;
    }

    return SEOS_SUCCESS;
}