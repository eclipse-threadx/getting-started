# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET ${PROJECT_NAME})
set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_flexspi_nor.ld")

set(SOURCES
    startup/startup_MIMXRT1062.S
    startup/tx_initialize_low_level.S

    azure_iothub.c
    board_init.c
    console.c
    main.c
)

add_executable(${TARGET} ${SOURCES})

set_target_properties(${TARGET}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_link_libraries(${TARGET} 
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        MIMXRT1060-evk
        netx_driver
        app_common
)

target_link_options(${TARGET}
    PRIVATE 
        -T${LINKER_SCRIPT} -Wl,-Map=${TARGET}.map)

create_bin_output(${TARGET})
firmware_size(${TARGET})
