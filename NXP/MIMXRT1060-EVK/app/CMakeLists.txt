set(TARGET_RAM ${PROJECT_NAME}_ram)
set(TARGET_FLASH ${PROJECT_NAME}_flash)
set(LINKER_SCRIPT_RAM "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_ram.ld")
set(LINKER_SCRIPT_FLASH "${CMAKE_CURRENT_LIST_DIR}/startup/MIMXRT1062xxxxx_flexspi_nor.ld")

set(SOURCES
    startup/startup_mimxrt1062.s
    newlibc/syscalls.c
    newlibc/console.c

    main.c
    board_init.c
    board_init.h
    network.c
    sync_time.c
    azure_central.c
)

set(LIBRARIES
    threadx
    netxduo
    netxdriver
    MIMXRT1060-evk
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

    create_bin_output(${TARGET_FLASH})
    firmware_size(${TARGET_FLASH})
endif()