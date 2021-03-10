# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/${THREADX_TOOLCHAIN}/startup_MIMXRT1062.S
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
        MIMXRT1060-evk
        netx_driver
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(THREADX_TOOLCHAIN STREQUAL "iar")
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            --config ${CMAKE_CURRENT_LIST_DIR}/startup/iar/MIMXRT1062xxxxx_flexspi_nor.icf --map=${PROJECT_NAME}.map)
else()
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            -T${CMAKE_CURRENT_LIST_DIR}/startup/gnu/MIMXRT1062xxxxx_flexspi_nor.ld -Wl,-Map=${PROJECT_NAME}.map)
endif()

post_build(${PROJECT_NAME})
