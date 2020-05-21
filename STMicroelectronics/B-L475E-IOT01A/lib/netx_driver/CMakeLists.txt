# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/wifi.c
    src/es_wifi.c
    src/es_wifi_io.c
)

set(PROJECT netx_driver)

add_library(${PROJECT} INTERFACE)

target_sources(${PROJECT}
    INTERFACE
        ${SOURCES})

target_include_directories(${PROJECT}
    INTERFACE
        src
)

target_link_libraries(${PROJECT}
    INTERFACE
        azrtos::threadx
        azrtos::netxduo
        stm32cubel4
)