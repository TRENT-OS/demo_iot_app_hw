/*
 *  Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#include "OS_Error.h"

#include "LibMem/Nvm.h"
#include "LibUtil/RleCompressor.h"

#include "RamDisk.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Default value: 64MiB
#if !defined(RAM_DISK_SIZE)
#   define RAM_DISK_SIZE (1024*1024*64)
#endif

static uint8_t disk[RAM_DISK_SIZE];

struct RamDisk
{
    Nvm parent;
    RamDisk_Init_t mode;
    uint8_t* buf;
    size_t size;
};

// Private functions -----------------------------------------------------------

static bool
addrValidate(
    const RamDisk_t* self,
    const size_t offset,
    const size_t size)
{
    size_t addr = size + offset;

    // Check we do not have a wrap-around and also that we do not exceed the
    // amount of space we have available..
    return (addr >= offset) && (addr >= size) && (addr < self->size);
}

static size_t
RamDisk_write(
    Nvm* ctx,
    size_t addr,
    void const* buffer,
    size_t length)
{
    RamDisk_t* self = (RamDisk_t*) ctx;

    if (NULL == ctx || NULL == buffer)
    {
        return 0;
    }
    if (!addrValidate(self, addr, length))
    {
        return 0;
    }

    memcpy(&self->buf[addr], buffer, length);

    return length;
}

static size_t
RamDisk_read(
    Nvm* ctx,
    size_t addr,
    void* buffer,
    size_t length)
{
    RamDisk_t* self = (RamDisk_t*) ctx;

    if (NULL == ctx || NULL == buffer)
    {
        return 0;
    }
    if (!addrValidate(self, addr, length))
    {
        return 0;
    }

    memcpy(buffer, &self->buf[addr], length);

    return length;
}

static size_t
RamDisk_erase(
    Nvm* ctx,
    size_t addr,
    size_t length)
{
    RamDisk_t* self = (RamDisk_t*) ctx;

    if (NULL == ctx)
    {
        return 0;
    }
    if (!addrValidate(self, addr, length))
    {
        return 0;
    }

    memset(&self->buf[addr], 0xff, length);

    return length;
}

static size_t
RamDisk_getSize(
    Nvm* ctx)
{
    RamDisk_t* self = (RamDisk_t*) ctx;

    if (NULL == self)
    {
        return 0;
    }

    return self->size;
}

static void
RamDisk_dtor(
    Nvm* ctx)
{
    if (NULL != ctx)
    {
        free(ctx);
    }
}

static const Nvm_Vtable RamDisk_vtable =
{
    .read       = RamDisk_read,
    .erase      = RamDisk_erase,
    .getSize    = RamDisk_getSize,
    .write      = RamDisk_write,
    .dtor       = RamDisk_dtor
};

// Public functions ------------------------------------------------------------

OS_Error_t
RamDisk_ctor(
    RamDisk_t** ctx,
    RamDisk_Config_t* cfg)
{
    OS_Error_t err;
    RamDisk_t* self;

    if (NULL == ctx || NULL == cfg)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }
    switch (cfg->mode)
    {
    case RamDisk_Init_COPY_BUFFER:
        if (NULL == cfg->ptr)
        {
            return OS_ERROR_INVALID_PARAMETER;
        }
    case RamDisk_Init_EMPTY: // FALLTHROUGH!!
        if (!cfg->size)
        {
            return OS_ERROR_INVALID_PARAMETER;
        }
        if (cfg->size > RAM_DISK_SIZE)
        {
            return OS_ERROR_INSUFFICIENT_SPACE;
        }
        break;
    default:
        return OS_ERROR_NOT_SUPPORTED;
    }

    if ((self = calloc(1, sizeof(RamDisk_t))) == NULL)
    {
        return OS_ERROR_INSUFFICIENT_SPACE;
    }

    *ctx = self;

    // This was supposed to be allocated, but for now we can only use a static
    // buffer if we need a ramdisk with >1MiB size..
    self->buf  = disk;
    self->mode = cfg->mode;
    // It has to live here for the "inheritance-in-C-concept" to work..
    self->parent.vtable = &RamDisk_vtable;

    switch (cfg->mode)
    {
    case RamDisk_Init_COPY_BUFFER:
        if (cfg->compressed)
        {
            // Simply uncompress the image into the exisiting buffer
            if ((err = RleCompressor_decompress(cfg->size, cfg->ptr,
                                                RAM_DISK_SIZE,
                                                &self->size,
                                                &self->buf)) != OS_SUCCESS)
            {
                goto err0;
            }
        }
        else
        {
            self->size = cfg->size;
            memcpy(self->buf, cfg->ptr, self->size);
        }
        break;
    case RamDisk_Init_EMPTY:
        self->size = cfg->size;
        memset(self->buf, 0xff, self->size);
        break;
    default:    // Handeled above
        break;
    }

    return OS_SUCCESS;

err0:
    free(self);
    return err;
}