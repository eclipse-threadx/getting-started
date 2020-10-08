# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

if(THREADX_TOOLCHAIN STREQUAL "iar")
    set(SOURCES_COMPILER
        src/iar/nx_driver_same54_low_level.S
    )
else()
    set(SOURCES_COMPILER
        src/gcc/nx_driver_same54_low_level.S
    )
endif()

set(SOURCES
    ${SOURCES_COMPILER}
    src/nx_driver_same54.c
    src/nx_driver_same54.h
)

set(TARGET netxdriver)

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
        atmel_start
)
