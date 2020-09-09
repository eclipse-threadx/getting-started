# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_flexspi_nor.ld")

set(SOURCES
    startup/startup_MIMXRT1062.S
    startup/tx_initialize_low_level.S

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

        app_common
        jsmn
        MIMXRT1060-evk
        netx_driver
)

target_link_options(${PROJECT_NAME}
    PRIVATE 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})
