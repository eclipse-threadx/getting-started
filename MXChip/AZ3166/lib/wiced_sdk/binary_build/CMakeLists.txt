cmake_minimum_required(VERSION 3.13)

project(wiced_sdk C)

set(CMAKE_C_STANDARD 99)

set(GSG_BASE_DIR ${PROJECT_SOURCE_DIR}/../../../../../)
set(CORE_LIB_DIR ${GSG_BASE_DIR}/core/lib)

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../../cmake)

add_subdirectory(../.. MXCHIP_lib)

set(WICED_SDK_SRC
    43xxx_Wi-Fi/43362A2_bin.c
    43xxx_Wi-Fi/host_cm4.c
    43xxx_Wi-Fi/platform.c
    43xxx_Wi-Fi/platform_resource.c
    43xxx_Wi-Fi/wiced_crypto.c
    43xxx_Wi-Fi/wwd_ap.c
    43xxx_Wi-Fi/wwd_ap_common.c
    43xxx_Wi-Fi/wwd_buffer.c
    43xxx_Wi-Fi/wwd_bus_common.c
    43xxx_Wi-Fi/wwd_bus_protocol.c
    43xxx_Wi-Fi/wwd_chip_specific_functions.c
    43xxx_Wi-Fi/wwd_debug.c
    43xxx_Wi-Fi/wwd_eapol.c
    43xxx_Wi-Fi/wwd_internal.c
    43xxx_Wi-Fi/wwd_management.c
    43xxx_Wi-Fi/wwd_network.c
    43xxx_Wi-Fi/wwd_platform.c
    43xxx_Wi-Fi/wwd_platform_separate_mcu.c
    43xxx_Wi-Fi/wwd_resources.c
    43xxx_Wi-Fi/wwd_rtos.c
    43xxx_Wi-Fi/wwd_SDIO.c
    43xxx_Wi-Fi/wwd_sdpcm.c
    43xxx_Wi-Fi/wwd_thread.c
    43xxx_Wi-Fi/wwd_wifi.c
    43xxx_Wi-Fi/wwd_wifi_chip_common.c
    43xxx_Wi-Fi/wwd_wifi_sleep.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/platform_gpio.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/platform_mcu_powersave.c
    # STM32 Lib
    43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/src/stm32f4xx_exti.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/src/stm32f4xx_gpio.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/src/stm32f4xx_rcc.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/src/stm32f4xx_sdio.c
    43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/src/stm32f4xx_syscfg.c
)

set(TARGET wiced_sdk_bin)

# Add a library with the above sources
add_library(${TARGET} 
    ${WICED_SDK_SRC})

# Preprocessor marcos
target_compile_definitions(${TARGET}
    PRIVATE
        WICED_CUSTOM_NX_USER_H
        wifi_firmware_image=resources_firmware_DIR_43362_DIR_43362A2_bin
)

target_include_directories(${TARGET}
    PUBLIC
        43xxx_Wi-Fi
        43xxx_Wi-Fi/internal
        43xxx_Wi-Fi/internal/bus_protocols
        43xxx_Wi-Fi/network
        43xxx_Wi-Fi/platform
        43xxx_Wi-Fi/RTOS
    
    PRIVATE
        43xxx_Wi-Fi/STM32F4xx/peripherals
        43xxx_Wi-Fi/STM32F4xx/peripherals/libraries
        43xxx_Wi-Fi/STM32F4xx/peripherals/libraries/inc
)

target_link_libraries(${TARGET}
    PRIVATE
        azrtos::threadx
        azrtos::netxduo
)
