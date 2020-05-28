# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET netxdriver)

set(SOURCES
    src/nx_driver_same54.c
    src/nx_driver_same54.h
    src/nx_driver_same54_low_level.s
)

add_library(${TARGET} OBJECT
    ${SOURCES})

target_include_directories(${TARGET}
    PUBLIC
        src
)

target_link_libraries(${TARGET} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        atmel_start
)
