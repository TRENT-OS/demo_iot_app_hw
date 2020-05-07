/*
 *  MQTT/mbedTLS glue layer
 *
 *  Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#include "glue_tls_mqtt.h"

// decrease send/recv max length to increase robustness
#define MAX_NW_SIZE 2048

//------------------------------------------------------------------------------
static OS_Tls_Handle_t tlsContext;
static OS_Crypto_Handle_t hCrypto;
static OS_NetworkSocket_Handle_t socket;

static int
sendFunc(
    void*                ctx,
    const unsigned char* buf,
    size_t               len);

static int
recvFunc(
    void*          ctx,
    unsigned char* buf,
    size_t         len);

static int
entropy(
    void*          ctx,
    unsigned char* buf,
    size_t         len);


static OS_Tls_Config_t tlsCfg =
{
    .mode = OS_Tls_MODE_LIBRARY,
    .config.library = {
        .socket = {
            .recv   = recvFunc,
            .send   = sendFunc,
        },
        .flags = OS_Tls_FLAG_DEBUG,
        .crypto = {
            .policy = NULL,
            .cipherSuites = {
                OS_Tls_CIPHERSUITE_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
                OS_Tls_CIPHERSUITE_DHE_RSA_WITH_AES_128_GCM_SHA256
            },
            .cipherSuitesLen = 2
        }
    }
};

static OS_Crypto_Config_t cryptoCfg =
{
    .mode = OS_Crypto_MODE_LIBRARY_ONLY,
    .library.rng.entropy = entropy
};

// Private static functions ----------------------------------------------------
static seos_err_t
init_socket_config(OS_Network_Socket_t* socketConfig,
                   const char* serverIpAddress,
                   uint32_t serverPort)
{
    if (NULL == socketConfig || NULL == serverIpAddress)
    {
        return SEOS_ERROR_INVALID_PARAMETER;
    }

    socketConfig->domain = OS_AF_INET;
    socketConfig->type   = OS_SOCK_STREAM;
    socketConfig->port   = serverPort;
    socketConfig->name   = serverIpAddress;

    return SEOS_SUCCESS;
}


static int
sendFunc(
    void*                ctx,
    const unsigned char* buf,
    size_t               len)
{
    seos_err_t err;
    OS_NetworkSocket_Handle_t* sockHandle = (OS_NetworkSocket_Handle_t*) ctx;
    size_t n;

    n = len > MAX_NW_SIZE ? MAX_NW_SIZE : len;
    if ((err = OS_NetworkSocket_write(*sockHandle, buf, &n)) != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("Error during socket write...error:%d", err);
        return -1;
    }

    return n;
}

static int
recvFunc(
    void*          ctx,
    unsigned char* buf,
    size_t         len)
{
    seos_err_t err;
    OS_NetworkSocket_Handle_t* sockHandle = (OS_NetworkSocket_Handle_t*) ctx;
    size_t n;

    n = len > MAX_NW_SIZE ? MAX_NW_SIZE : len;
    if ((err = OS_NetworkSocket_read(*sockHandle, buf, &n)) != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("Error during socket read...error:%d", err);
        return -1;
    }

    return n;
}

static int
entropy(
    void*          ctx,
    unsigned char* buf,
    size_t         len)
{
    // This would be the platform specific function to obtain entropy
    memset(buf, 0, len);
    return 0;
}

//------------------------------------------------------------------------------
seos_err_t
glue_tls_init(const char* serverIpAddress,
              const char* caCert,
              size_t caCertSize,
              uint32_t serverPort)
{

    seos_err_t ret;

    OS_Network_Socket_t socketCfg;

    if (caCertSize > OS_Tls_SIZE_CA_CERT_MAX)
    {
        Debug_LOG_ERROR("Server caCert size not supported!");
        return SEOS_ERROR_INSUFFICIENT_SPACE;
    }

    ret = init_socket_config(&socketCfg, serverIpAddress, serverPort);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("init_socket_config() failed with: %d", ret);
        return ret;
    }

    ret = OS_Crypto_init(&hCrypto, &cryptoCfg);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("OS_Crypto_init() failed with: %d", ret);
        return ret;
    }

    tlsCfg.config.library.socket.context = &socket;
    tlsCfg.config.library.crypto.handle = hCrypto;

    memcpy(tlsCfg.config.server.library.crypto.caCert, caCert,
           sizeof(tlsCfg.config.server.library.crypto.caCert));
    Debug_LOG_DEBUG("Assigned ServerCert: %s",
                    tlsCfg.config.server.library.crypto.caCert);

    ret = OS_Tls_init(&tlsContext, &tlsCfg);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("OS_Tls_init() failed with: %d", ret);
        return ret;
    }

    ret = OS_NetworkSocket_create(NULL, &socketCfg, &socket);

    return ret;
}

//------------------------------------------------------------------------------
seos_err_t
glue_tls_handshake(void)
{
    int ret;

    if ((ret = OS_Tls_handshake(tlsContext)) != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("OS_Tls_handshake() failed with err=%i", ret);
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
int glue_tls_mqtt_write(Network* n,
                        const unsigned char* buf,
                        int len,
                        int timeout_ms)
{
    Debug_ASSERT(buf    != NULL);

    int ret;

    if ((ret = OS_Tls_write(tlsContext, buf, len)) != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("OS_Tls_write() failed with err=%i", ret);
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
int glue_tls_mqtt_read(Network* n,
                       unsigned char* buf,
                       int len,
                       int timeout_ms)
{
    Debug_ASSERT(buf    != NULL);

    seos_err_t ret;
    Timer t;
    size_t lengthRead = len;
    Debug_LOG_TRACE("%s: %d bytes, %d ms", __func__, len, timeout_ms);

    TimerInit(&t);
    TimerCountdownMS(&t, timeout_ms);

    memset ( buf, 0, len );

    if ((ret = OS_Tls_read(tlsContext, buf, &lengthRead)) != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("OS_Tls_read() failed with err=%i", ret);
        return ret;
    }

    return lengthRead;
}
