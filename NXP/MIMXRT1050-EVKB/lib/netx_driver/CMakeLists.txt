# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/nx_driver_imxrt10xx.c
    src/nx_driver_imxrt10xx.h
    src/${THREADX_TOOLCHAIN}/nx_driver_imxrt10xx_low_level.S
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
        MIMXRT1050-evk
)
