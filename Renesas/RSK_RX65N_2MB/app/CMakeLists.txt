# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

#set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/linker_script.ld")

set(SOURCES
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
        rx_driver_package
        netx_driver
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    message(FATAL_ERROR, "IAR is not currently implemented for this device")
else()
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/gnu/linker_script.ld)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,-e_PowerON_Reset)
endif()

post_build(${PROJECT_NAME})
