set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/startup/B-L475E-IOT01.ld")

set(SOURCES
    libc/sysmem.c
    libc/syscalls.c
    startup/system_stm32l4xx.c
    startup/startup.s
    stm32cubel4_conf/stm32l4xx_it.c
    stm32cubel4_conf/stm32l4xx_hal_msp.c

    azure/azure_mqtt.c
    azure/cert.c
    azure/hmac_sha256.c
    azure/sas_token.c
    azure/sha256.c

    azure_config.c
    board_init.c
    console.c
    main.c
    networking.c
    sntp_client.c
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
        .
        stm32cubel4_conf
)

target_link_libraries(${PROJECT_NAME}
    threadx
    netxduo
    stm32cubel4
    netx_driver
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})