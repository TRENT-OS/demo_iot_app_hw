/**
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "LibDebug/Debug.h"

#include "OS_ConfigService.h"


OS_Error_t
create_system_config_backend();

OS_Error_t createFileBackends(hPartition_t phandle);

OS_Error_t initializeFileBackends(OS_ConfigServiceLib_t* configLib,
                                  hPartition_t phandle);

OS_Error_t
initializeDomainsAndParameters(OS_ConfigServiceLib_t* configLib);