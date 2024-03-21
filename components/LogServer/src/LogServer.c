/*
 * Main file of the LogServer component.
 *
 * Copyright (C) 2020-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */


#include "lib_debug/Debug.h"
#include "TimeServer.h"

#include "Logger/Server/OS_LoggerFile.h"

#include "Logger/Server/OS_LoggerConsumerChain.h"
#include "Logger/Server/OS_LoggerConsumer.h"

#include "Logger/Server/OS_LoggerOutputConsole.h"
#include "Logger/Server/OS_LoggerOutputFileSystem.h"

#include "Logger/Client/OS_LoggerEmitter.h"

#include "OS_FileSystem.h"

#include "custom_log_format.h"

#include <stdio.h>

#include <camkes.h>


/* Defines -------------------------------------------------------------------*/
#define DATABUFFER_SERVER_01    (void *)configServer_port
#define DATABUFFER_SERVER_02    (void *)cloudConnector_port
#define DATABUFFER_SERVER_03    (void *)sensor_port
#define DATABUFFER_SERVER_04    (void *)nwDriver_port
#define DATABUFFER_SERVER_05    (void *)nwStack_port

// log server id
#define LOG_SERVER_ID               0

// client id's
#define CLIENT_CONFIGSRV_ID         10
#define CLIENT_CLOUDCON_ID          20
#define CLIENT_SENSORTEMP_ID        30
#define CLIENT_NIC_ID               40
#define CLIENT_NWSTACK_ID           50

#define PARTITION_ID                1
#define LOG_FILENAME                "log.txt"

uint32_t API_LOG_SERVER_GET_SENDER_ID(void);

static OS_LoggerFilter_Handle_t filter_configSrv,
       filter_cloudCon, filter_sensorTemp, filter_nic, filter_nwStack;
static OS_LoggerConsumer_Handle_t log_consumer_configSrv,
       log_consumer_cloudCon, log_consumer_sensorTemp, log_consumer_nic, log_consumer_nwStack;
static OS_LoggerConsumerCallback_t log_consumer_callback;
static OS_LoggerSubject_Handle_t subject;
static OS_LoggerOutput_Handle_t filesystem, console;
static OS_LoggerFile_Handle_t log_file;

// Emitter configuration
static OS_LoggerFilter_Handle_t filter_log_server;
static OS_LoggerConsumer_Handle_t log_consumer_log_server;
static OS_LoggerSubject_Handle_t subject_log_server;
static OS_LoggerOutput_Handle_t console_log_server;
static char buf_log_server[DATABUFFER_SIZE];

static OS_FileSystem_Handle_t hFs;
static OS_FileSystem_Config_t cfgFs =
{
    .type = OS_FileSystem_Type_FATFS,
    .size = OS_FileSystem_USE_STORAGE_MAX,
    .storage = IF_OS_STORAGE_ASSIGN(
        storage_rpc,
        storage_port),
};

static const if_OS_Timer_t timer =
    IF_OS_TIMER_ASSIGN(
        timeServer_rpc,
        timeServer_notify);

// Private functions -----------------------------------------------------------

static OS_Error_t
initFileSystem(
    void)
{
    OS_Error_t err = OS_FileSystem_init(&hFs, &cfgFs);
    if (OS_SUCCESS != err)
    {
        printf("OS_FileSystem_init failed with error code %d!", err);
        return err;
    }

    err = OS_FileSystem_mount(hFs);
    if (OS_SUCCESS != err)
    {
        printf("OS_FileSystem_mount failed with error code %d!", err);
        goto err0;
    }

    return OS_SUCCESS;

err0:
    err =  OS_FileSystem_free(hFs);
    if (OS_SUCCESS != err)
    {
        printf("OS_FileSystem_free failed with error code %d!", err);
    }

    return err;
}

static uint64_t
get_time_sec(
    void)
{
    OS_Error_t err;
    uint64_t sec;

    if ((err = TimeServer_getTime(&timer, TimeServer_PRECISION_SEC,
                                  &sec)) != OS_SUCCESS)
    {
        Debug_LOG_ERROR("TimeServer_getTime() failed with %d", err);
        sec = 0;
    }

    return sec;
}

// Public functions ------------------------------------------------------------

void pre_init(void)
{
    // create filesystem
    if (initFileSystem() != OS_SUCCESS)
    {
        printf("Fail to init filesystem!\n");
        return;
    }

    // set up consumer chain
    OS_LoggerConsumerChain_getInstance();

    // register objects to observe
    OS_LoggerSubject_ctor(&subject);
    // Emitter configuration
    OS_LoggerSubject_ctor(&subject_log_server);

    // set up log file
    OS_LoggerFile_ctor(&log_file, hFs, LOG_FILENAME);

    // set up backend
    OS_LoggerOutputFileSystem_ctor(&filesystem, &custom_log_format);
    OS_LoggerOutputConsole_ctor(&console, &custom_log_format);
    // Emitter configuration
    OS_LoggerOutputConsole_ctor(&console_log_server, &custom_log_format);

    // attach observed object to subject
    OS_LoggerSubject_attach(
        (OS_LoggerAbstractSubject_Handle_t*)&subject,
        &filesystem);

    OS_LoggerSubject_attach(
        (OS_LoggerAbstractSubject_Handle_t*)&subject,
        &console);

    // Emitter configuration
    OS_LoggerSubject_attach(
        (OS_LoggerAbstractSubject_Handle_t*)&subject_log_server,
        &console_log_server);

    // set up log filter layer
    OS_LoggerFilter_ctor(&filter_configSrv,      Debug_LOG_LEVEL_INFO);
    OS_LoggerFilter_ctor(&filter_cloudCon,       Debug_LOG_LEVEL_INFO);
    OS_LoggerFilter_ctor(&filter_sensorTemp,     Debug_LOG_LEVEL_INFO);
    OS_LoggerFilter_ctor(&filter_nic,            Debug_LOG_LEVEL_INFO);
    OS_LoggerFilter_ctor(&filter_nwStack,        Debug_LOG_LEVEL_INFO);
    // Emitter configuration
    OS_LoggerFilter_ctor(&filter_log_server,     Debug_LOG_LEVEL_INFO);

    // set up registered functions layer
    OS_LoggerConsumerCallback_ctor(
        &log_consumer_callback,
        API_LOG_SERVER_GET_SENDER_ID,
        get_time_sec);

    // set up log consumer layer
    OS_LoggerConsumer_ctor(&log_consumer_configSrv,      DATABUFFER_SERVER_01,
                           &filter_configSrv,      &log_consumer_callback, &subject, &log_file,
                           CLIENT_CONFIGSRV_ID, "CONFIG-SERVER");
    OS_LoggerConsumer_ctor(&log_consumer_cloudCon,       DATABUFFER_SERVER_02,
                           &filter_cloudCon,       &log_consumer_callback, &subject, &log_file,
                           CLIENT_CLOUDCON_ID, "CLOUDCONNECTOR");
    OS_LoggerConsumer_ctor(&log_consumer_sensorTemp,     DATABUFFER_SERVER_03,
                           &filter_sensorTemp,     &log_consumer_callback, &subject, &log_file,
                           CLIENT_SENSORTEMP_ID, "SENSOR-TEMP");
    OS_LoggerConsumer_ctor(&log_consumer_nic,       DATABUFFER_SERVER_04,
                           &filter_nic,       &log_consumer_callback, &subject, &log_file,
                           CLIENT_NIC_ID, "NIC");
    OS_LoggerConsumer_ctor(&log_consumer_nwStack,        DATABUFFER_SERVER_05,
                           &filter_nwStack,        &log_consumer_callback, &subject, &log_file,
                           CLIENT_NWSTACK_ID, "NWSTACK");

    // Emitter configuration
    OS_LoggerConsumer_ctor(&log_consumer_log_server, buf_log_server,
                           &filter_log_server, &log_consumer_callback, &subject_log_server, &log_file,
                           LOG_SERVER_ID, "LOG-SERVER");

    // Emitter configuration: set up log emitter layer
    OS_LoggerEmitter_getInstance(
        buf_log_server,
        &filter_log_server,
        API_LOG_SERVER_EMIT);

    // set up consumer chain layer
    OS_LoggerConsumerChain_append(&log_consumer_configSrv);
    OS_LoggerConsumerChain_append(&log_consumer_cloudCon);
    OS_LoggerConsumerChain_append(&log_consumer_sensorTemp);
    OS_LoggerConsumerChain_append(&log_consumer_nic);
    OS_LoggerConsumerChain_append(&log_consumer_nwStack);
    // Emitter configuration
    OS_LoggerConsumerChain_append(&log_consumer_log_server);

    // create log file
    OS_LoggerFile_create(&log_file);
}
