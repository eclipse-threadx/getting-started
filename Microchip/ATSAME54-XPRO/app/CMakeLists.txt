# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    startup/${THREADX_TOOLCHAIN}/startup_same54.c
    azure_config.h
    nx_client.c
    mqtt.c
    board_init.c
    console.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        azrtos::nx_cloud
        azrtos::azure_iot
        
        atmel_start
        app_common
        jsmn
        netxdriver
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        SUFFIX ".elf"
)

post_build(${PROJECT_NAME})

if(THREADX_TOOLCHAIN STREQUAL "iar")
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            --config ${CMAKE_CURRENT_LIST_DIR}/startup/iar/same54x20_flash.icf --map=${PROJECT_NAME}.map)
else()
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            -T${CMAKE_CURRENT_LIST_DIR}/startup/gnu/same54p20a_flash.ld -Wl,-Map=${PROJECT_NAME}.map)
endif()
