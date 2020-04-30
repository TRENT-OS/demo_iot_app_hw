/*
   *  Admin component that writes the system config file based on the set
   *  parameters.
   *
   *  Copyright (C) 2020, Hensoldt Cyber GmbH
*/


#include <camkes.h>

#include "LibDebug/Debug.h"
#include "set_config_backend.h"

static system_config_t systemConfig =
{
    .network_stack_config = {
        .config_domain = DOMAIN_NWSTACK,
        .dev_addr = {
            .config_param_name = ETH_ADDR,
            .addr = ETH_ADDR_VALUE,
            .len = sizeof(ETH_ADDR_VALUE)
        },
        .gateway = {
            .config_param_name = ETH_GATEWAY_ADDR,
            .addr = ETH_GATEWAY_ADDR_VALUE,
            .len = sizeof(ETH_GATEWAY_ADDR_VALUE)
        },
        .subnet_mask = {
            .config_param_name = ETH_SUBNET_MASK,
            .addr = ETH_SUBNET_MASK_VALUE,
            .len = sizeof(ETH_SUBNET_MASK_VALUE)
        }
    },
    .cloud_server_config = {
        .config_domain = DOMAIN_CLOUDCONNECTOR,
        .server_ip = {
            .config_param_name = SERVER_ADDRESS_NAME,
            .addr = SERVER_ADDRESS_VALUE,
            .len = sizeof(SERVER_ADDRESS_VALUE)
        },
        .server_port = {
            .config_param_name = SERVER_PORT_NAME,
            .port = SERVER_PORT_VALUE,
        },
        .server_cert = {
            .config_param_name = SERVER_CA_CERT_NAME,
            .content = SERVER_CA_CERT_PEM_VALUE,
            .len     = sizeof(SERVER_CA_CERT_PEM_VALUE)
        },
        .iot_hub = {
            .config_param_name = CLOUD_DOMAIN_NAME,
            .hub_name = CLOUD_DOMAIN_VALUE,
            .len  = sizeof(CLOUD_DOMAIN_VALUE),
            .accessKey = {
                .config_param_name = CLOUD_SAS_NAME,
                .key = CLOUD_SAS_VALUE,
                .len = sizeof(CLOUD_SAS_VALUE)
            }
        },
        .iot_device = {
            .config_param_name = CLOUD_DEVICE_ID_NAME,
            .device_name = CLOUD_DEVICE_ID_VALUE,
            .len  = sizeof(CLOUD_DEVICE_ID_VALUE)
        }
    },
    .mqtt_msg = {
        .config_domain             = DOMAIN_SENSOR,
        .config_param_name_payload = MQTT_PAYLOAD_NAME,
        .payload                   = MQTT_PAYLOAD_VALUE,
        .payload_len               = sizeof(MQTT_PAYLOAD_VALUE),
        .config_param_name_topic   = MQTT_TOPIC_NAME,
        .topic                     = MQTT_TOPIC_VALUE,
        .topic_len                 = sizeof(MQTT_TOPIC_VALUE),
    }
};

int run(void)
{
    // wait for the ConfigServer to prepare the configuration backend
    config_backend_ready_wait();

    Debug_LOG_INFO("Setting system configuration...");
    seos_err_t err = init_system_config(&systemConfig);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("create_system_config_backend failed with:%d", err);
        return -1;
    }

    Debug_LOG_INFO("System configuration set.");

    // notify apps that configuration can now be retrieved
    system_config_set_emit();

    return 0;
}
