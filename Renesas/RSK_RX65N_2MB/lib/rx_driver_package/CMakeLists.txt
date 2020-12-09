# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/r_ether_rx/src/r_ether_rx.c
)

set(TARGET rx_driver_package)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_compile_definitions(${TARGET}
    PUBLIC 
)

target_include_directories(${TARGET}
    PUBLIC
        src/Config_SCI8
        src/general
        src/r_bsp
        src/r_cmt_rx
        src/r_config
        src/r_ether_rx
        src/r_pincfg
)
