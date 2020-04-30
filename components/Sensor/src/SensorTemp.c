/**
 * Sensor component that cyclically sends a MQTT message to the CloudConnector.
 *
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#include "LibDebug/Debug.h"

#include "OS_ConfigService.h"

#include "helper_func.h"

#include "MQTTPacket.h"

#include <string.h>
#include <camkes.h>
#include "time.h"

/* Defines -------------------------------------------------------------------*/
#define SECS_TO_SLEEP   5
#define S_IN_MSEC       1000

OS_ConfigServiceHandle_t serverLibWithFSBackend;

static unsigned char payload[128]; // arbitrary max expected length
static char topic[128];

static seos_err_t
initializeSensor(void)
{
    seos_err_t err = SEOS_SUCCESS;

    err = OS_ConfigService_createHandle(
              OS_CONFIG_HANDLE_KIND_RPC,
              0,
              &serverLibWithFSBackend);
    if (err != SEOS_SUCCESS)
    {
        return err;
    }

    return SEOS_SUCCESS;
}

static seos_err_t
CloudConnector_write(unsigned char* msg, void* dataPort, size_t len)
{
    memcpy(dataPort, msg, len);
    seos_err_t err = cloudConnector_interface_write();
    return err;
}


int run()
{
    seos_err_t ret = SEOS_SUCCESS;

    ret = initializeSensor();
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("initializeSensor() failed with:%d", ret);
        return ret;
    }

    // wait for the init of the admin component
    admin_system_config_set_wait();

    Debug_LOG_INFO("Starting TemperatureSensor...");

    ret = helper_func_getConfigParameter(&serverLibWithFSBackend,
                                         DOMAIN_SENSOR,
                                         MQTT_PAYLOAD_NAME,
                                         &payload,
                                         sizeof(payload));
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("helper_func_getConfigParameter() for param %s failed with :%d",
                        MQTT_PAYLOAD_NAME, ret);
        return ret;
    }
    Debug_LOG_INFO("Retrieved MQTT Payload: %s", payload);


    ret = helper_func_getConfigParameter(&serverLibWithFSBackend,
                                         DOMAIN_SENSOR,
                                         MQTT_TOPIC_NAME,
                                         &topic,
                                         sizeof(topic));
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("helper_func_getConfigParameter() for param %s failed with :%d",
                        MQTT_TOPIC_NAME, ret);
        return ret;
    }

    MQTTString mqttTopic = MQTTString_initializer;
    mqttTopic.cstring = topic;
    Debug_LOG_INFO("Retrieved MQTT Topic: %s", mqttTopic.cstring);

    unsigned char serializedMsg[320]; //arbitrary size
    int len = MQTTSerialize_publish(serializedMsg,
                                    sizeof(serializedMsg),
                                    0,
                                    0,
                                    0,
                                    1,
                                    mqttTopic,
                                    (unsigned char*)payload,
                                    strlen((const char*)payload));


    for (;;)
    {
        CloudConnector_write(serializedMsg, (void*)cloudConnectorDataPort,
                             len);
        api_time_server_sleep(SECS_TO_SLEEP * S_IN_MSEC);
    }
    sensor_init_emit();
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("sensor_init_emit() failed with: %d",
                        ret);
        return 0;
    }

    return 0;
}