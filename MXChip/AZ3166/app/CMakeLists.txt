# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/startup/MXChip_AZ3166.ld")

set(SOURCES
    # libc/sysmem.c
    # libc/syscalls.c
    
    startup/system_stm32f4xx.c
    startup/startup_stm32f412rx.s
    startup/tx_initialize_low_level.S

    # azure_iothub.c
    board_init.c
    # console.c
    main.c
    networking.c
    sample_azure_iot.c
    stm32f4xx_hal_msp.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_link_options(${PROJECT_NAME}
    PUBLIC 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        USE_COM_PORT
        WIFI_SSID=${WIFI_SSID}
        WIFI_PASSWORD=${WIFI_PASSWORD}
        WIFI_SECURITY=WICED_SECURITY_WPA2_MIXED_PSK
        WIFI_COUNTRY=WICED_COUNTRY_CHINA
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo
    stm32cubef4
    wiced_sdk
    # app_common
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})