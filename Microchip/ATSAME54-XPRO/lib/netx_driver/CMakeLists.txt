# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET netxdriver)

set(SOURCES
    nx_driver_same54.c
    nx_driver_same54.h
    nx_driver_same54_low_level.S
)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} 
    INTERFACE
        ${SOURCES})

target_include_directories(${TARGET}
    INTERFACE
        .
)

target_link_libraries(${TARGET} 
    INTERFACE
        threadx
        netxduo
)
