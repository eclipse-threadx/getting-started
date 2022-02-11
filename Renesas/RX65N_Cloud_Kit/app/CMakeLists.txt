# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S

    legacy/mqtt.c
    azure_config.h
    nx_client.c
    board_init.c
    console.c
    main.c
    rx_networking.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo

        app_common
        jsmn
        rx_driver_package
        sensorlib
        netx_driver
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC 
       startup/${THREADX_TOOLCHAIN}    
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set_target_linker(${PROJECT_NAME} linker_script.icf)
else()
    set_target_linker(${PROJECT_NAME} linker_script.ld)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,-e_PowerON_Reset)
endif()

post_build(${PROJECT_NAME})
