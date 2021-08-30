# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    weather_click.c
    src/bme280.c
)

set(TARGET bme280)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        .
    PRIVATE
        src
)

target_link_libraries(${TARGET}
    PUBLIC
        atmel_start
)
