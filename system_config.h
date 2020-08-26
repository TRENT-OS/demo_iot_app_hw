/**
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 *
 * System libraries configurations
 *
 */
#pragma once


//-----------------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------------
#if !defined(NDEBUG)
#   define Debug_Config_STANDARD_ASSERT
#   define Debug_Config_ASSERT_SELF_PTR
#else
#   define Debug_Config_DISABLE_ASSERT
#   define Debug_Config_NO_ASSERT_SELF_PTR
#endif

#define Debug_Config_LOG_LEVEL                  Debug_LOG_LEVEL_INFO
#define Debug_Config_INCLUDE_LEVEL_IN_MSG
#define Debug_Config_LOG_WITH_FILE_LINE


//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------
#define Memory_Config_USE_STDLIB_ALLOC


//-----------------------------------------------------------------------------
// RamDisk size
//-----------------------------------------------------------------------------
#define RAMDISK_SIZE_BYTES  (2 * 1024 * 1024)


//-----------------------------------------------------------------------------
// StorageServer
//-----------------------------------------------------------------------------
#define CONFIGSERVER_STORAGE_OFFSET 0
#define CONFIGSERVER_STORAGE_SIZE   (128*1024)


//-----------------------------------------------------------------------------
// TimeServer clients
//-----------------------------------------------------------------------------
#define TIMESERVER_CC_ID         1
#define TIMESERVER_LS_ID         2
#define TIMESERVER_ST_ID         3
#define TIMESERVER_FL_ID         4
#define TIMESERVER_TK_ID         5
#define TIMESERVER_ND_ID         6
#define TIMESERVER_NS_ID         7


//-----------------------------------------------------------------------------
// LOGGER
//-----------------------------------------------------------------------------

#if !defined(CAMKES_TOOL_PROCESSING)

// If the belows header is included in the config file and Logger library is
// linked, the LibDebug will forward entries to the LogServer.
#include "Logger/Client/OS_Logger.h"

// api interface name
#define API_LOG_SERVER_EMIT                     log_server_interface_emit
#define API_LOG_SERVER_GET_SENDER_ID            log_server_interface_get_sender_id
#define API_LOG_SERVER_READ_LOG_FILE            log_server_interface_read_log_file

#endif // !defined(CAMKES_TOOL_PROCESSING)

//-----------------------------------------------------------------------------
// Network
//-----------------------------------------------------------------------------
#ifndef OS_NETWORK_MAXIMUM_SOCKET_NO
#define OS_NETWORK_MAXIMUM_SOCKET_NO 16
#endif
#define OS_NETWORK_STACK_USE_CONFIGSERVER

//-----------------------------------------------------------------------------
// IDS
//-----------------------------------------------------------------------------

#define CONFIGSERVER_LOGGER_ID      10
#define CLOUDCONNECTOR_LOGGER_ID    20
#define SENSOR_LOGGER_ID            30
#define NWSTACK_LOGGER_ID           40

#define CONFIGSERVER_STORAGE_ID     1
#define LOGGER_STORAGE_ID           2

#define NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS 16
#define NIC_DRIVER_RINGBUFFER_SIZE                                             \
    (NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS * 4096)

