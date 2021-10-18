# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    nx_driver_rx65n_cloud_kit.c
)

set(TARGET netx_driver)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        .
)

target_link_libraries(${TARGET} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        rx_driver_package
)
