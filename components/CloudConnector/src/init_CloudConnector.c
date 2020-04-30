/*
   *  Init file for the CloudConnector.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include <camkes.h>

#include "LibDebug/Debug.h"
#include "common.h"

#include "OS_ConfigService.h"

/* Instance variables --------------------------------------------------------*/
OS_ConfigServiceHandle_t serverLibWithMemBackend;

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