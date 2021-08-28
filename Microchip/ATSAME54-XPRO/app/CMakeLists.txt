# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    startup/${THREADX_TOOLCHAIN}/startup_same54.c
    legacy/mqtt.c
    azure_config.h
    nx_client.c
    board_init.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
         
        atmel_start
        app_common
        jsmn
        netxdriver
        bme280
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/iar/same54x20_flash.icf)
else()
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/gnu/same54p20a_flash.ld)
endif()

post_build(${PROJECT_NAME})
