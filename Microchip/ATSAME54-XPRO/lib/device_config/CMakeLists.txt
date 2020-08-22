# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    device_config.c
)

set(TARGET device_config)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC 
        ./
)

target_link_libraries(${TARGET}
    PUBLIC
        azrtos::threadx
        atmel_start
)
