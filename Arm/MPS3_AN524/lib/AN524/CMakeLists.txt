# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Exceptions required by CMSIS UART driver
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-ignored-qualifiers -Wno-return-type")

set(SOURCES
    src/CMSIS/Driver_USART.c
    src/device/source/system_core_init.c
    src/native_drivers/uart_cmsdk_drv.c
)

set(TARGET AN524)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_compile_definitions(${TARGET}
    PUBLIC 
        # TODO: Add defines
)

target_include_directories(${TARGET}
    PUBLIC
        src/CMSIS/config
        src/CMSIS/include
        src/device/config
        src/device/include
        src/native_drivers
)
