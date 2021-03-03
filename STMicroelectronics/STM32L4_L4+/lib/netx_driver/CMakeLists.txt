# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    inventek/es_wifi.c
    inventek/es_wifi_io.c
    inventek/wifi.c

    nx_wifi.c
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
        inventek
)
