set(TARGET MIMXRT1060-evk)

set(SOURCES
    boards/evkmimxrt1060/project_template/board.c
    boards/evkmimxrt1060/project_template/clock_config.c
    boards/evkmimxrt1060/project_template/peripherals.c
    boards/evkmimxrt1060/project_template/pin_mux.c
    boards/evkmimxrt1060/xip/evkmimxrt1060_flexspi_nor_config.c

    components/phyksz8081/fsl_phy.c
    components/phyksz8081/fsl_phy.h
    components/serial_manager/serial_manager.c
    components/serial_manager/serial_port_uart.c
    components/uart/lpuart_adapter.c
    
    devices/MIMXRT1062/drivers/fsl_clock.c
    devices/MIMXRT1062/drivers/fsl_enet.c
    devices/MIMXRT1062/drivers/fsl_gpio.c
    devices/MIMXRT1062/drivers/fsl_lpuart.c
    devices/MIMXRT1062/system_MIMXRT1062.c
    devices/MIMXRT1062/utilities/debug_console/fsl_debug_console.c
    devices/MIMXRT1062/utilities/debug_console/fsl_debug_console.h
    devices/MIMXRT1062/utilities/str/fsl_str.c
    devices/MIMXRT1062/utilities/str/fsl_str.h
    devices/MIMXRT1062/xip/fsl_flexspi_nor_boot.c
)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} 
    INTERFACE
        ${SOURCES})

target_compile_definitions(${TARGET}
    INTERFACE 
        CPU_MIMXRT1062DVL6A
        SERIAL_PORT_TYPE_UART=1
        FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE
        SKIP_SYSCLK_INIT
        __USE_CMSIS
        XIP_EXTERNAL_FLASH=1
        XIP_BOOT_HEADER_ENABLE=1
)

target_include_directories(${TARGET}
    INTERFACE
        boards/evkmimxrt1060/project_template/
        components/phyksz8081
        components/serial_manager
        components/uart
        CMSIS/Include
		devices/MIMXRT1062
        devices/MIMXRT1062/drivers/
        devices/MIMXRT1062/utilities/debug_console/
		devices/MIMXRT1062/utilities/str/
)
