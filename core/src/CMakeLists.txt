# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET app_common)

set(SOURCES
    iot_mqtt/iot_mqtt.c
    iot_mqtt/sha256.c
    iot_mqtt/hmac_sha256.c
    iot_mqtt/sas_token.c
    azure_iot/azure_iot_nx_client.c

    azure_iot_cert.c
    azure_iot_ciphersuites.c
    sntp_client.c
    newlib_nano.c
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
)

target_link_libraries(${TARGET}
    azrtos::threadx
    azrtos::netxduo
    azrtos::nx_cloud
    azrtos::azure_iot
    jsmn
)