# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    config/board/board.c
    config/board/dcd.c
    config/board/clock_config.c
    config/board/pin_mux.c

    src/boards/evkmimxrt1060/xip/evkmimxrt1060_flexspi_nor_config.c
    src/components/phyksz8081/fsl_phy.c
    src/components/phyksz8081/fsl_phy.h
    src/components/serial_manager/serial_manager.c
    src/components/serial_manager/serial_port_uart.c
    src/components/uart/lpuart_adapter.c
    src/devices/MIMXRT1062/drivers/fsl_clock.c
    src/devices/MIMXRT1062/drivers/fsl_enet.c
    src/devices/MIMXRT1062/drivers/fsl_gpio.c
    src/devices/MIMXRT1062/drivers/fsl_lpuart.c
    src/devices/MIMXRT1062/system_MIMXRT1062.c
    src/devices/MIMXRT1062/utilities/debug_console/fsl_debug_console.c
    src/devices/MIMXRT1062/utilities/str/fsl_str.c
    src/devices/MIMXRT1062/xip/fsl_flexspi_nor_boot.c
)

set(TARGET MIMXRT1060-evk)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_compile_definitions(${TARGET}
    PUBLIC 
        CPU_MIMXRT1062DVL6A
        FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE
        SKIP_SYSCLK_INIT
        SERIAL_PORT_TYPE_UART=1
        XIP_BOOT_HEADER_DCD_ENABLE=1
        XIP_BOOT_HEADER_ENABLE=1
        XIP_EXTERNAL_FLASH=1
)

target_include_directories(${TARGET}
    PUBLIC
        config/board

        src/components/phyksz8081
        src/components/serial_manager
        src/components/uart
        src/CMSIS/Include
		src/devices/MIMXRT1062
        src/devices/MIMXRT1062/drivers
        src/devices/MIMXRT1062/utilities/debug_console
        src/devices/MIMXRT1062/utilities/str
)
