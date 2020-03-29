/*
 *  MQTT/mbedTLS glue layer
 *
 *  Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "compiler.h"

#include "LibDebug/Debug.h"

#include "SeosTlsApi.h"
#include "SeosCryptoApi.h"

#include "OS_Network.h"

#include <string.h>
#include <camkes.h>

#if defined(MQTTCLIENT_PLATFORM_HEADER)
#define XSTR(s) STR(s)
#define STR(s)  #s
#include XSTR(MQTTCLIENT_PLATFORM_HEADER)
#endif

seos_err_t
glue_tls_init(const char* ipAddress,
              const char* caCert,
              size_t caCertSize,
              uint32_t port);

seos_err_t
glue_tls_handshake(void);

int
glue_tls_mqtt_write(Network* n,
                    const unsigned char* buf,
                    int len,
                    int timeout_ms);

int
glue_tls_mqtt_read(Network* n,
                   unsigned char* buf,
                   int len,
                   int timeout_ms);
