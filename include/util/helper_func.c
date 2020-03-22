/**
 * Collection of helper functions used by several component.
 *
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#include <string.h>

#include "helper_func.h"

// -----------------------------------------------------------------------------
void initializeName(char* buf, size_t bufSize, char const* name)
{
    memset(buf, 0, bufSize);
    strncpy(buf, name, bufSize - 1);
}

//------------------------------------------------------------------------------
static
seos_err_t
compareDomainName(SeosConfigLib_DomainName const* a,
                  SeosConfigLib_DomainName const* b)
{
    for (unsigned int k = 0; k < SEOS_CONFIG_LIB_DOMAIN_NAME_LEN; ++k)
    {
        if (a->name[k] != b->name[k])
        {
            Debug_LOG_TRACE("Error: function: %s - line: %d\n", __FUNCTION__, __LINE__);
            return SEOS_ERROR_GENERIC;
        }
    }

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
static
seos_err_t
seos_configuration_initializeDomainName(
    SeosConfigLib_DomainName* domainName,
    char const* name)
{
    initializeName(domainName->name, SEOS_CONFIG_LIB_DOMAIN_NAME_LEN, name);

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
static
seos_err_t
seos_configuration_initializeParameterName(
    SeosConfigLib_ParameterName* parameterName,
    char const* name)
{
    initializeName(parameterName->name, SEOS_CONFIG_LIB_PARAMETER_NAME_LEN, name);

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
static
seos_err_t
find_domain(SeosConfigHandle handle,
            SeosConfigLib_DomainEnumerator* enumerator,
            SeosConfigLib_DomainName const* domainName,
            SeosConfigLib_Domain* domain)
{
    seos_err_t ret;

    seos_configuration_domainEnumeratorInit(handle, enumerator);
    for (;;)
    {
        ret = seos_configuration_domainEnumeratorGetElement(
                  handle,
                  enumerator,
                  domain);
        if (0 != ret)
        {
            Debug_LOG_ERROR("seos_configuration_domainEnumeratorGetElement() failed, ret %d",
                            ret);
            return SEOS_ERROR_GENERIC;
        }

        SeosConfigLib_DomainName domainNameTmp;
        seos_configuration_domainGetName(domain, &domainNameTmp);
        if (SEOS_SUCCESS == compareDomainName(&domainNameTmp, domainName))
        {
            // enumerator holds the right domain
            return SEOS_SUCCESS;
        }

        ret = seos_configuration_domainEnumeratorIncrement(handle, enumerator);
        if (0 != ret)
        {
            Debug_LOG_ERROR("seos_configuration_domainEnumeratorIncrement() failed, ret %d",
                            ret);
            return SEOS_ERROR_GENERIC;
        }
    } // end for(;;)
}

//------------------------------------------------------------------------------
static
seos_err_t
get_parameter_enumerator(
    SeosConfigHandle handle,
    const char* DomainName,
    const char* ParameterName,
    SeosConfigLib_ParameterEnumerator* parameterEnumerator)
{
    seos_err_t ret;

    SeosConfigLib_DomainEnumerator domainEnumerator = {0};
    SeosConfigLib_DomainName domainName;
    SeosConfigLib_ParameterName parameterName;
    SeosConfigLib_Domain domain = {0};

    seos_configuration_initializeDomainName(&domainName,
                                            DomainName);

    seos_configuration_initializeParameterName(&parameterName, ParameterName);

    ret = find_domain(handle, &domainEnumerator, &domainName, &domain);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("find_domain() failed, ret %d", ret);
        return SEOS_ERROR_CONFIG_DOMAIN_NOT_FOUND;
    }

    ret = seos_configuration_domainEnumeratorGetElement(handle, &domainEnumerator,
                                                        &domain);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("SeosConfigLib_domainEnumeratorGetElement() failed, ret %d",
                        ret);
        return SEOS_ERROR_GENERIC;
    }

    ret = seos_configuration_domainCreateParameterEnumerator(handle, &domain,
                                                             &parameterName, parameterEnumerator);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("SeosConfigLib_domainCreateParameterEnumerator() failed, ret %d",
                        ret);
        return SEOS_ERROR_GENERIC;
    }

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
static
seos_err_t
get_parameter_element(
    SeosConfigHandle handle,
    const char* DomainName,
    const char* ParameterName,
    SeosConfigLib_DomainName* domainName,
    SeosConfigLib_ParameterName* parameterName,
    SeosConfigLib_Parameter* parameter)
{
    seos_err_t ret;
    SeosConfigLib_Domain domain;
    SeosConfigLib_DomainEnumerator domainEnumerator = {0};

    seos_configuration_initializeDomainName(domainName,
                                            DomainName);
    seos_configuration_initializeParameterName(parameterName, ParameterName);

    ret = find_domain(handle, &domainEnumerator, domainName, &domain);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("find_domain() failed, ret %d", ret);
        return SEOS_ERROR_CONFIG_DOMAIN_NOT_FOUND;
    }

    ret = seos_configuration_domainGetElement(handle, &domain, parameterName,
                                              parameter);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("domainGetElement() failed, ret %d", ret);
        return SEOS_ERROR_CONFIG_PARAMETER_NOT_FOUND;
    }

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
seos_err_t
helper_func_getConfigParameter(SeosConfigHandle* handle,
                               const char* DomainName,
                               const char* ParameterName,
                               void* parameterBuffer,
                               size_t parameterLength)
{
    seos_err_t ret;
    size_t bytesCopied;
    SeosConfigLib_DomainName domainName;
    SeosConfigLib_ParameterName parameterName;
    SeosConfigLib_Parameter parameter;
    SeosConfigHandle configHandle = *handle;

    ret = get_parameter_element(configHandle, DomainName, ParameterName,
                                &domainName, &parameterName, &parameter);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("get_parameter_element() failed, ret %d", ret);
        return ret;
    }

    SeosConfigLib_ParameterType parameterType;
    seos_configuration_parameterGetType(&parameter, &parameterType);

    ret = seos_configuration_parameterGetValue(configHandle,
                                               &parameter,
                                               parameterBuffer,
                                               parameterLength,
                                               &bytesCopied);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("parameterGetValue() failed, ret %d", ret);
        return ret;
    }

    return SEOS_SUCCESS;
}

//------------------------------------------------------------------------------
seos_err_t helper_func_setConfigParameter(SeosConfigHandle* handle,
                                          const char* DomainName,
                                          const char* ParameterName,
                                          const void* parameterValue,
                                          size_t parameterLength)
{
    seos_err_t ret;
    SeosConfigHandle configHandle = *handle;
    SeosConfigLib_ParameterEnumerator parameterEnumerator = {0};
    SeosConfigLib_DomainName domainName;
    SeosConfigLib_ParameterName parameterName;
    SeosConfigLib_Parameter parameter;

    ret = get_parameter_element(configHandle, DomainName, ParameterName,
                                &domainName, &parameterName, &parameter);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("get_parameter_element() failed, ret %d", ret);
        return ret;
    }

    ret = get_parameter_enumerator(configHandle,
                                   DomainName,
                                   ParameterName,
                                   &parameterEnumerator);
    if (SEOS_SUCCESS != ret)
    {
        Debug_LOG_ERROR("get_parameter_enumerator() failed, ret %d", ret);
        return SEOS_ERROR_GENERIC;
    }

    SeosConfigLib_ParameterType parameterType;
    seos_configuration_parameterGetType(&parameter, &parameterType);

    ret = seos_configuration_parameterSetValue(configHandle,
                                               &parameterEnumerator,
                                               parameterType,
                                               parameterValue,
                                               parameterLength);
    if (ret < 0)
    {
        Debug_LOG_ERROR("seos_configuration_parameterSetValue() failed, ret %d", ret);
        return ret;
    }

    return SEOS_SUCCESS;
}