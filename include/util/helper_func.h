/**
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "LibDebug/Debug.h"

#include "SeosConfigLib.h"
#include "SeosConfigBackend.h"

#include "seos_config_client.h"


//------------------------------------------------------------------------------
seos_err_t
helper_func_getConfigParameter(SeosConfigHandle* handle,
                               const char* DomainName,
                               const char* ParameterName,
                               void*       parameterBuffer,
                               size_t      parameterLength);

seos_err_t
helper_func_setConfigParameter(SeosConfigHandle* handle,
                               const char* DomainName,
                               const char* ParameterName,
                               const void* parameterValue,
                               size_t      parameterLength);