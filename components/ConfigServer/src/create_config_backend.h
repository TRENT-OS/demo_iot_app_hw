/**
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "LibDebug/Debug.h"

#include "OS_ConfigService.h"


seos_err_t
create_system_config_backend();

seos_err_t createFileBackends(hPartition_t phandle);

seos_err_t initializeFileBackends(OS_ConfigServiceLib_t* configLib,
                                  hPartition_t phandle);

seos_err_t
initializeDomainsAndParameters(OS_ConfigServiceLib_t* configLib);