# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET app_common)

set(SOURCES
    azure_iot_mqtt/azure_iot_mqtt.c
    azure_iot_mqtt/azure_iot_dps_mqtt.c
    azure_iot_mqtt/hmac_sha256.c
    azure_iot_mqtt/sas_token.c
    azure_iot_mqtt/sha256.c

    azure_iot_nx/azure_iot_nx_client.c
    azure_iot_nx/nx_azure_iot_pnp_helpers.c

    azure_iot_cert.c
    azure_iot_ciphersuites.c
    json_utils.c
    newlib_nano.c
    sntp_client.c
)

# Allow to disable the common networking component
if(NOT DEFINED DISABLE_COMMON_NETWORK) 
    list(APPEND SOURCES
        networking.c
    )
endif()

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        .
        azure_iot_mqtt
        azure_iot_nx
)

target_link_libraries(${TARGET}
    azrtos::threadx
    azrtos::netxduo
    azrtos::nx_cloud
    azrtos::azure_iot
    jsmn
)