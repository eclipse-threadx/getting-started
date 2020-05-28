# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/startup/B-L475E-IOT01.ld")

set(SOURCES
    startup/system_stm32l4xx.c
    startup/startup.s
    startup/tx_initialize_low_level.S

    stm32cubel4/stm32l4xx_hal_msp.c
    stm32cubel4/stm32l4xx_it.c

    azure_iothub.c
    board_init.c
    console.c
    main.c
    networking.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/system)

target_link_options(${PROJECT_NAME}
    PUBLIC 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        stm32cubel4
)

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo
    stm32cubel4
    netx_driver
    app_common
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})