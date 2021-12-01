# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    
    azure_config.h
    board_init.c
    console.c
    main.c
    nx_client.c
    stm_networking.c
)

add_executable(${PROJECT_NAME}
    startup/${THREADX_TOOLCHAIN}/startup_stm32l4s5xx.s
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo

    stm32cubel4
    netx_driver
    app_common
    jsmn
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/iar/stm32l4s5xx_flash.icf)
else()
    set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/startup/gnu/STM32L4S5VITx_FLASH.ld)
endif()

post_build(${PROJECT_NAME})
