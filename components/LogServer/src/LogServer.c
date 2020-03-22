/*
   *  Main file of the LogServer component.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include "LibDebug/Debug.h"

#include "seos_log_server_backend_filesystem.h"
#include "seos_fs.h"
#include "seos_pm.h"

#include <stdio.h>

#include <camkes.h>


/* Defines -------------------------------------------------------------------*/
#define DATABUFFER_SERVER_01    (void *)dataport_buf_admin
#define DATABUFFER_SERVER_02    (void *)dataport_buf_configServer
#define DATABUFFER_SERVER_03    (void *)dataport_buf_cloudConnector
#define DATABUFFER_SERVER_04    (void *)dataport_buf_sensorTemp
#define DATABUFFER_SERVER_05    (void *)dataport_buf_nwDriver
#define DATABUFFER_SERVER_06    (void *)dataport_buf_nwStack

// log server id
#define LOG_SERVER_ID               0

// client id's
#define CLIENT_ADMIN_ID             10
#define CLIENT_CONFIGSRV_ID         20
#define CLIENT_CLOUDCON_ID          30
#define CLIENT_SENSORTEMP_ID        40
#define CLIENT_NWDRIVER_ID          50
#define CLIENT_NWSTACK_ID           60

#define PARTITION_ID                1
#define LOG_FILENAME                "log.txt"



uint32_t API_LOG_SERVER_GET_SENDER_ID(void);



static Log_filter_t filter_admin, filter_configSrv, filter_cloudCon,
       filter_sensorTemp, filter_nwDriver, filter_nwStack;
static Log_consumer_t log_consumer_admin, log_consumer_configSrv,
       log_consumer_cloudCon, log_consumer_sensorTemp,
       log_consumer_nwDriver, log_consumer_nwStack;
static Log_consumer_callback_t log_consumer_callback;
static Log_format_t format;
static Log_subject_t subject;
static Log_output_t filesystem, console;
static Log_file_t log_file;
static Log_emitter_callback_t emitter_callback;
// Emitter configuration
static Log_filter_t filter_log_server;
static Log_consumer_t log_consumer_log_server;
static Log_subject_t subject_log_server;
static Log_output_t console_log_server;
static char buf_log_server[DATABUFFER_SIZE];


static bool
filesystem_init(void)
{
    hPartition_t phandle;
    pm_disk_data_t pm_disk_data;
    pm_partition_data_t pm_partition_data;

    if (partition_manager_get_info_disk(&pm_disk_data) != SEOS_PM_SUCCESS)
    {
        printf("Fail to get disk info!\n");
        return false;
    }

    if (partition_manager_get_info_partition(PARTITION_ID,
                                             &pm_partition_data) != SEOS_PM_SUCCESS)
    {
        printf("Fail to get partition info: %d!\n", pm_partition_data.partition_id);
        return false;
    }

    if (partition_init(pm_partition_data.partition_id, 0) != SEOS_FS_SUCCESS)
    {
        printf("Fail to init partition: %d!\n", pm_partition_data.partition_id);
        return false;
    }

    if ( (phandle = partition_open(pm_partition_data.partition_id)) < 0)
    {
        printf("Fail to open partition: %d!\n", pm_partition_data.partition_id);
        return false;
    }

    if (partition_fs_create(
            phandle,
            FS_TYPE_FAT16,
            pm_partition_data.partition_size,
            0,  // default value: size of sector:   512
            0,  // default value: size of cluster:  512
            0,  // default value: reserved sectors count: FAT12/FAT16 = 1; FAT32 = 3
            0,  // default value: count file/dir entries: FAT12/FAT16 = 16; FAT32 = 0
            0,  // default value: count header sectors: 512
            FS_PARTITION_OVERWRITE_CREATE)
        != SEOS_FS_SUCCESS)
    {
        printf("Fail to create filesystem on partition: %d!\n",
               pm_partition_data.partition_id);
        return false;
    }

    if (partition_close(phandle) != SEOS_FS_SUCCESS)
    {
        printf("Fail to close partition: %d!\n", pm_partition_data.partition_id);
        return false;
    }

    return true;
}


void log_server_interface__init()
{
    // set up consumer chain
    get_instance_Consumer_chain();

    // set up log format layer
    Log_format_ctor(&format);

    // register objects to observe
    Log_subject_ctor(&subject);
    // Emitter configuration
    Log_subject_ctor(&subject_log_server);

    // set up log file
    Log_file_ctor(&log_file, PARTITION_ID, LOG_FILENAME);

    // set up backend
    Log_output_filesystem_ctor(&filesystem, &format);
    Log_output_console_ctor(&console, &format);
    // Emitter configuration
    Log_output_console_ctor(&console_log_server, &format);

    // attach observed object to subject
    Log_subject_attach((Subject_t*)&subject, (Observer_t*)&filesystem);
    Log_subject_attach((Subject_t*)&subject, (Observer_t*)&console);
    // Emitter configuration
    Log_subject_attach((Subject_t*)&subject_log_server,
                       (Observer_t*)&console_log_server);

    // Emitter configuration: set up registered functions layer
    Log_emitter_callback_ctor(&emitter_callback, NULL, API_LOG_SERVER_EMIT);

    // set up log filter layer
    Log_filter_ctor(&filter_admin,          Debug_LOG_LEVEL_INFO);
    Log_filter_ctor(&filter_configSrv,      Debug_LOG_LEVEL_INFO);
    Log_filter_ctor(&filter_cloudCon,       Debug_LOG_LEVEL_DEBUG);
    Log_filter_ctor(&filter_sensorTemp,     Debug_LOG_LEVEL_INFO);
    Log_filter_ctor(&filter_nwDriver,       Debug_LOG_LEVEL_INFO);
    Log_filter_ctor(&filter_nwStack,        Debug_LOG_LEVEL_INFO);
    // Emitter configuration
    Log_filter_ctor(&filter_log_server,     Debug_LOG_LEVEL_DEBUG);

    // set up registered functions layer
    Log_consumer_callback_ctor(&log_consumer_callback, logServer_ready_emit,
                               API_LOG_SERVER_GET_SENDER_ID, api_time_server_get_timestamp);

    // set up log consumer layer
    Log_consumer_ctor(&log_consumer_admin,          DATABUFFER_SERVER_01, &filter_admin,          &log_consumer_callback, &subject, NULL, CLIENT_ADMIN_ID, "ADMIN");
    Log_consumer_ctor(&log_consumer_configSrv,      DATABUFFER_SERVER_02, &filter_configSrv,      &log_consumer_callback, &subject, NULL, CLIENT_CONFIGSRV_ID, "CONFIG-SERVER");
    Log_consumer_ctor(&log_consumer_cloudCon,       DATABUFFER_SERVER_03, &filter_cloudCon,       &log_consumer_callback, &subject, NULL, CLIENT_CLOUDCON_ID, "CLOUDCONNECTOR");
    Log_consumer_ctor(&log_consumer_sensorTemp,     DATABUFFER_SERVER_04, &filter_sensorTemp,     &log_consumer_callback, &subject, NULL, CLIENT_SENSORTEMP_ID, "SENSOR-TEMP");
    Log_consumer_ctor(&log_consumer_nwDriver,       DATABUFFER_SERVER_05, &filter_nwDriver,       &log_consumer_callback, &subject, NULL, CLIENT_NWDRIVER_ID, "NWDRIVER");
    Log_consumer_ctor(&log_consumer_nwStack,        DATABUFFER_SERVER_06, &filter_nwStack,        &log_consumer_callback, &subject, NULL, CLIENT_NWSTACK_ID, "NWSTACK");

    // Emitter configuration
    Log_consumer_ctor(&log_consumer_log_server, buf_log_server, &filter_log_server, &log_consumer_callback, &subject_log_server, &log_file, LOG_SERVER_ID, "LOG-SERVER");

    // Emitter configuration: set up log emitter layer
    get_instance_Log_emitter(buf_log_server, &filter_log_server, &emitter_callback);

    // set up consumer chain layer
    Consumer_chain_append(&log_consumer_admin);
    Consumer_chain_append(&log_consumer_configSrv);
    Consumer_chain_append(&log_consumer_cloudCon);
    Consumer_chain_append(&log_consumer_sensorTemp);
    Consumer_chain_append(&log_consumer_nwDriver);
    Consumer_chain_append(&log_consumer_nwStack);
    // Emitter configuration
    Consumer_chain_append(&log_consumer_log_server);

    // create filesystem
    if(filesystem_init() == false){
        printf("Fail to init filesystem!\n");
        return;
    }

    // create log file
    Log_file_create_log_file(&log_file);

    // start polling
    Consumer_chain_poll();
}

