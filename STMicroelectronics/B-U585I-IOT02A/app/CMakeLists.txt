# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/tx_initialize_low_level.S
    
    board_init.c
    console.c
    nx_client.c
    stm_networking.c
    main.c
)

add_executable(${PROJECT_NAME}
    startup/startup_stm32u585aiix.s
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo

    stm32cubeu5
    app_common
    jsmn
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/STM32U585AIIX_FLASH.ld)

post_build(${PROJECT_NAME})
