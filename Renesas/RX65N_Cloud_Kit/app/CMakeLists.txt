# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/linker_script.ld")

set(SOURCES
    startup/tx_initialize_low_level.S

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
        netx_driver
)

target_link_options(${PROJECT_NAME}
    PRIVATE 
        -T${LINKER_SCRIPT} 
        -Wl,-Map=${PROJECT_NAME}.map
        -Wl,-e_PowerON_Reset
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})
