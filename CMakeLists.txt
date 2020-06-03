cmake_minimum_required(VERSION 3.7.2)

os_set_config_file(system_config.h)

project(system_config C)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/include/seos_mqtt")

os_use_libs(
    OS_LIBS
    OS_CHANMUX
    OS_CRYPTO
    OS_PARTITION_MANAGER
    OS_FILESYSTEM_CORE
    OS_FILESYSTEM_FAT
    OS_FILESYSTEM_SPIFFS
    OS_CONFIGURATION
    OS_TLS
    OS_LOGGER
)


project(demo_iot_app C)


#-------------------------------------------------------------------------------
# Component Admin
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(Admin
    INCLUDES
        include/util
    SOURCES
        components/Admin/src/Admin.c
        components/Admin/src/set_config_backend.c
        components/common/common.c
        include/util/helper_func.c
    C_FLAGS
        -Wall -Werror
        -DOS_CONFIG_SERVICE_CAMKES_CLIENT     # indicates a camkes configuration client is being built
    LIBS
        system_config
        seos_core_api
        seos_libs
        seos_configuration
        seos_logger
)

#-------------------------------------------------------------------------------
# Component PartitionManager
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(PartitionManager
    SOURCES
        components/PartitionManager/src/PartitionManager.c
    C_FLAGS
        -Wall
        -Werror
        -DOS_PARTITION_MANAGER_BUILD_AS_COMPONENT
    LIBS
        system_config
        seos_libs
        seos_partition_manager
        seos_logger
        proxy_nvm
)

#-------------------------------------------------------------------------------
# Component SensorTemp
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(SensorTemp
    INCLUDES
        include/util
    SOURCES
        components/Sensor/src/SensorTemp.c
        components/common/common.c
        include/util/helper_func.c
    C_FLAGS
        -Wall -Werror
        -DOS_CONFIG_SERVICE_CAMKES_CLIENT    # indicates a camkes configuration server will be used
    LIBS
        system_config
        seos_libs
        seos_core_api
        seos_configuration
        seos_logger
        seos_mqtt
)

#-------------------------------------------------------------------------------
# Component ConfigServer
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(ConfigServer
    SOURCES
        components/ConfigServer/src/ConfigServer.c
        components/ConfigServer/src/create_config_backend.c
        components/common/common.c
    C_FLAGS
        -Wall
        -Werror
        # filesystem
        -DOS_CONFIG_SERVICE_BACKEND_FILESYSTEM
        #-DCONFIG_SERVER_BACKEND_MEMORY
        -DOS_FS_BUILD_AS_LIB_BASIC_HANDLE
        # partition manager
        -DOS_PARTITION_MANAGER_BUILD_AS_COMPONENT
        # config server
        -DOS_CONFIG_SERVICE_CAMKES_SERVER     # indicates a camkes configuration server is being built
    LIBS
        system_config
        seos_libs
        seos_core_api
        seos_configuration
        seos_partition_manager_api
        seos_filesystem_core
        seos_filesystem_fat
        seos_filesystem_spiffs
        seos_logger
)

#-------------------------------------------------------------------------------
# Component LogServer
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(LogServer
    SOURCES
        components/LogServer/src/LogServer.c
    C_FLAGS
        -Wall
        -Werror
        # filesystem
        -DOS_FS_BUILD_AS_LIB_BASIC_HANDLE
        # partition manager
        -DOS_PARTITION_MANAGER_BUILD_AS_COMPONENT
    LIBS
        system_config
        seos_core_api
        seos_libs
        seos_log_server_backend_filesystem
        seos_partition_manager_api
        seos_filesystem_core
        seos_filesystem_fat
        seos_filesystem_spiffs
)

#-------------------------------------------------------------------------------
# Component CloudConnector
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(CloudConnector
    INCLUDES
        include
        include/util
    SOURCES
        components/CloudConnector/src/CloudConnector.c
        components/CloudConnector/src/init_CloudConnector.c
        components/CloudConnector/src/MQTT_net.c
        components/CloudConnector/src/MQTTServer.c
        components/CloudConnector/src/MQTT_client.c
        components/CloudConnector/src/glue_tls_mqtt.c
        components/common/common.c
        include/util/helper_func.c
    C_FLAGS
        -Wall -Werror
        -DOS_CONFIG_SERVICE_CAMKES_CLIENT
    LIBS
        seos_core_api
        seos_libs
        seos_crypto
        seos_mqtt
        seos_tls
        seos_configuration
        seos_logger
        os_network_api
)

#-------------------------------------------------------------------------------
# Component ChanMux
#-------------------------------------------------------------------------------
DeclareCAmkESComponent_ChanMux(
    ChanMux
    components/ChanMux/ChanMux.c
    system_config
)

#-------------------------------------------------------------------------------
# Component NwStack
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(NwStack
    INCLUDES
        include/util
    SOURCES
        components/NwStack/network_stack.c
        components/common/common.c
        include/util/helper_func.c
    C_FLAGS
        -Wall -Werror
        -DOS_NWSTACK_AS_CLIENT
        -DOS_CONFIG_SERVICE_CAMKES_CLIENT
        -DCFG_ETH_ADDR=ETH_ADDR
        -DCFG_ETH_GATEWAY_ADDR=ETH_GATEWAY_ADDR
        -DCFG_ETH_SUBNET_MASK=ETH_SUBNET_MASK
    LIBS
        system_config
        seos_core_api
        seos_libs
        os_network_lib
        seos_logger
        seos_configuration
)

#-------------------------------------------------------------------------------
# Component NwDriver
#-------------------------------------------------------------------------------
DeclareCAmkESComponent_NIC_ChanMux(
    NwDriver
    CHANMUX_CHANNEL_NIC_CTRL
    CHANMUX_CHANNEL_NIC_DATA
)

#-------------------------------------------------------------------------------
# Component UART
#-------------------------------------------------------------------------------
DeclareCAmkESComponent_UART(
    UART
)

#-------------------------------------------------------------------------------
# Component TimerClient
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(TimerClient
    SOURCES
        components/TimerClient/src/Timer_Client.c
    C_FLAGS
        -Wall
        -Werror
    LIBS
        system_config
        seos_core_api
        seos_libs
)

#-------------------------------------------------------------------------------
# Component TimeServer
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(TimeServer
    INCLUDES
        components/TimeServer/include
        components/TimeServer/timestamp
    SOURCES
        components/TimeServer/src/main.c
        components/TimeServer/src/time_server.c
        components/TimeServer/timestamp/timestamp.c
    C_FLAGS
        -Wall
        -Werror
)


DeclareAndCreateCamkESSystem(DemoIotApp.camkes)
GenerateSimulateScript()
