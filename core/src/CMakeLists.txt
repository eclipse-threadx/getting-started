# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET app_common)

set(SOURCES
    azure/azure_mqtt.c
    azure/cert.c
    azure/sha256.c
    azure/hmac_sha256.c
    azure/sas_token.c

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
)