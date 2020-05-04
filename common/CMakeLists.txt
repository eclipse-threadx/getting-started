set(TARGET app_common)

set(SOURCES
    azure/azure_mqtt.c
    azure/cert.c
    azure/sha256.c
    azure/hmac_sha256.c
    azure/sas_token.c

    networking.c
    sntp_client.c
)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} 
    INTERFACE
        ${SOURCES})

target_include_directories(${TARGET}
    INTERFACE
        .
)
