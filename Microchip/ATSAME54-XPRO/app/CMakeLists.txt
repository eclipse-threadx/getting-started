set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -lc -u _printf_float")
set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/Device_Startup/same54p20a_flash.ld")

set(SOURCES
    azure_config.c
    azure_iothub.c
    board_init.c
    Device_Startup/startup_same54.c
    Device_Startup/system_same54.c
    main.c
    newlibc/syscalls.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    threadx
    netxduo
    netxdriver
    atmel_start
    app_common
)

target_link_options(${PROJECT_NAME}
    PRIVATE 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
        ../lib/atmel_start
        ../lib/atmel_start/include
        ../lib/atmel_start/temperature_sensor
        ../lib/atmel_start/temperature_sensor/BME280_WeatherClick
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})
