# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/nx_driver_rx_fit.c
)

set(TARGET netx_driver)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        src
)

target_link_libraries(${TARGET} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        rx_driver_package
)
