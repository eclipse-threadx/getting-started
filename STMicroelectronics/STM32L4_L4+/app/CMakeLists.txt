# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    
    legacy/mqtt.c
    azure_config.h
    board_init.c
    console.c
    main.c
    nx_client.c
    stm_networking.c
)

function(create_target TARGET STARTUP LINKER)
    add_executable(${TARGET}
        startup/${THREADX_TOOLCHAIN}/${STARTUP}
        ${SOURCES}
    )

    target_link_libraries(${TARGET}
        azrtos::threadx
        azrtos::netxduo

        stm32cubel4
        netx_driver
        app_common
        jsmn
    )

    target_include_directories(${TARGET} 
        PUBLIC 
            .
    )

    if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
        set_target_linker(${TARGET} ${CMAKE_CURRENT_LIST_DIR}/startup/iar/${LINKER}.icf)
    else()
        set_target_linker(${TARGET} ${CMAKE_CURRENT_LIST_DIR}/startup/gnu/${LINKER}.ld)
    endif()

    post_build(${TARGET})

endfunction()

create_target("stm32l475_azure_iot" "startup_stm32l475xx.s" "STM32L475xx_FLASH")
create_target("stm32l4S5_azure_iot" "startup_stm32l4s5xx.s" "STM32L4S5xx_FLASH")
