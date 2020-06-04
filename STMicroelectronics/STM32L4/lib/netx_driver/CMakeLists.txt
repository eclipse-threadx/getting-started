# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/es_wifi.c
    src/es_wifi_io.c
    src/nx_wifi.c
    src/wifi.c
)

set(PROJECT netx_driver)

add_library(${PROJECT} OBJECT
    ${SOURCES}
)

target_link_libraries(${PROJECT}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        stm32cubel4
)

target_include_directories(${PROJECT}
    PUBLIC
        src
)
