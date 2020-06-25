/*
 *  Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "OS_Error.h"

#include "LibMem/Nvm.h"

typedef struct RamDisk_Context RamDisk_Context_t;

typedef enum {
    RamDisk_Init_ = 0,
    RamDisk_Init_EMPTY,
    RamDisk_Init_COPY_BUFFER
} RamDisk_Init_t;

typedef struct {
    RamDisk_Init_t mode;
    size_t size;
    const void* ptr;
    bool compressed;
} RamDisk_Config_t;

typedef struct RamDisk RamDisk_t;

OS_Error_t
RamDisk_ctor(
    RamDisk_t** self,
    RamDisk_Config_t* cfg);