# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/startup_MIMXRT1062.S
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    legacy/mqtt.c
    azure_config.h
    nx_client.c
    board_init.c
    console.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo

        app_common
        jsmn
        MIMXRT1060-evk
        netx_driver
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/iar/MIMXRT1062xxxxx_flexspi_nor.icf)
else()
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/gnu/MIMXRT1062xxxxx_flexspi_nor.ld)
endif()

post_build(${PROJECT_NAME})
