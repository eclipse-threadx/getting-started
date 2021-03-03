# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/tx_initialize_low_level.S
    
    stm32cubel4/stm32l4xx_hal_msp.c
    stm32cubel4/stm32l4xx_it.c
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

    set_target_properties(${TARGET}
        PROPERTIES 
            SUFFIX ".elf"
    )

    target_include_directories(${TARGET} 
        PUBLIC 
            .
    )

    if(THREADX_TOOLCHAIN STREQUAL "iar")
        target_link_options(${TARGET}
            PRIVATE 
                --config ${CMAKE_CURRENT_LIST_DIR}/startup/iar/${LINKER}.icf --map=${TARGET}.map)
    else()
        target_link_options(${TARGET}
            PRIVATE 
                -T${CMAKE_CURRENT_LIST_DIR}/startup/gnu/${LINKER}.ld -Wl,-Map=${TARGET}.map)
    endif()

    post_build(${TARGET})

endfunction()

create_target("stm32l475_azure_iot" "startup_stm32l475xx.s" "STM32L475xx_FLASH")
create_target("stm32l4S5_azure_iot" "startup_stm32l4s5xx.s" "STM32L4S5xx_FLASH")
