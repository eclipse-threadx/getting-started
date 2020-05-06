set(TARGET_RAM ${PROJECT_NAME}_ram)
set(TARGET_FLASH ${PROJECT_NAME}_flash)
set(LINKER_SCRIPT_RAM "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_ram.ld")
set(LINKER_SCRIPT_FLASH "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_flexspi_nor.ld")

set(SOURCES
    newlibc/syscalls.c
    newlibc/console.c
    startup/startup_mimxrt1062.s

    azure_config.c
    board_init.c

    main.c
)

set(LIBRARIES
    azrtos::threadx
    azrtos::netxduo
    MIMXRT1060-evk
    netx_driver
    app_common
)

#set(BUILD_RAM true)
set(BUILD_FLASH true)

if(BUILD_RAM)
    add_executable(${TARGET_RAM} ${SOURCES})

    target_link_libraries(${TARGET_RAM} ${LIBRARIES})

    target_link_options(${TARGET_RAM}
        PRIVATE 
            -T${LINKER_SCRIPT_RAM} -Wl,-Map=${TARGET_RAM}.map)

    set_target_properties(${TARGET_RAM}
        PROPERTIES 
            LINK_DEPENDS ${LINKER_SCRIPT_RAM}
            SUFFIX ".elf"
    )

    target_include_directories(${TARGET_RAM}
        PUBLIC
            .
    )

    create_bin_output(${TARGET_RAM})
    firmware_size(${TARGET_RAM})
endif()

if(BUILD_FLASH)
    add_executable(${TARGET_FLASH} ${SOURCES})

    target_link_libraries(${TARGET_FLASH} ${LIBRARIES})

    target_link_options(${TARGET_FLASH}
        PRIVATE 
            -T${LINKER_SCRIPT_FLASH} -Wl,-Map=${TARGET_FLASH}.map)

    set_target_properties(${TARGET_FLASH}
        PROPERTIES 
            LINK_DEPENDS ${LINKER_SCRIPT_FLASH}
            SUFFIX ".elf"
    )

    target_include_directories(${TARGET_FLASH}
        PUBLIC
            .
    )

    create_bin_output(${TARGET_FLASH})
    firmware_size(${TARGET_FLASH})
endif()

add_custom_target(invalidate_files ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_LIST_DIR}/main.c)