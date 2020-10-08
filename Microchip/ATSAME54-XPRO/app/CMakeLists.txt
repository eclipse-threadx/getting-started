# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

if(THREADX_TOOLCHAIN STREQUAL "iar")
    set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/iar/same54x20_flash.icf")

    set(SOURCES_COMPILER
        startup/iar/tx_initialize_low_level.S
        startup/iar/startup_same54.c
    )
else()
    set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/gcc/same54p20a_flash.ld")

    set(SOURCES_COMPILER
        startup/gcc/tx_initialize_low_level.S
        startup/gcc/startup_same54.c
    )
endif()

set(SOURCES
    ${SOURCES_COMPILER}
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
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

if(THREADX_TOOLCHAIN STREQUAL "iar")
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            --config ${LINKER_SCRIPT} --map=${PROJECT_NAME}.map)

    add_custom_target(${PROJECT_NAME}.bin ALL 
        DEPENDS ${PROJECT_NAME}
        COMMAND ${CMAKE_ELFTOOL} --bin ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)
else()
    target_link_options(${PROJECT_NAME}
        PRIVATE 
            -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)
    
    create_bin_output(${PROJECT_NAME})
    firmware_size(${PROJECT_NAME})
endif()
