# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 99)

set(GSG_BASE_DIR ${CMAKE_SOURCE_DIR}/../../../../..)
set(SHARED_SRC_DIR ${GSG_BASE_DIR}/shared/src)
set(SHARED_LIB_DIR ${GSG_BASE_DIR}/shared/lib)

# use the repo version of ninja on Windows as there is no Ninja installer
if(WIN32)
    set(CMAKE_MAKE_PROGRAM ${GSG_BASE_DIR}/cmake/ninja CACHE STRING "Ninja location")
endif()

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../cmake)

project(wiced_sdk C)

add_subdirectory(../.. MXCHIP_lib)

set(TARGET wiced_sdk_bin)

set(WICED_SDK_SRC
    src/43362A2_bin.c
    43xxx_Wi-Fi/platforms/BCM943362WCD6/platform.c
    43xxx_Wi-Fi/WICED/internal/wiced_crypto.c
    43xxx_Wi-Fi/WICED/network/NetX_Duo/WWD/wwd_buffer.c
    43xxx_Wi-Fi/WICED/network/NetX_Duo/WWD/wwd_network.c
    43xxx_Wi-Fi/WICED/platform/ARM_CM4/host_cm4.c
    43xxx_Wi-Fi/WICED/platform/MCU/platform_resource.c
    43xxx_Wi-Fi/WICED/platform/MCU/wwd_platform_separate_mcu.c
    43xxx_Wi-Fi/WICED/platform/MCU/wwd_resources.c
    43xxx_Wi-Fi/WICED/RTOS/Threadx/WWD/wwd_rtos.c
    43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
    43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO/wwd_bus_protocol.c
    43xxx_Wi-Fi/WICED/WWD/internal/chips/43362/wwd_ap.c
    43xxx_Wi-Fi/WICED/WWD/internal/chips/43362/wwd_chip_specific_functions.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_ap_common.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_debug.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_eapol.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_internal.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_management.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_sdpcm.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_thread.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi_chip_common.c
    43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi_sleep.c
    
    # STM32 Lib
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/WWD/wwd_SDIO.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/WWD/wwd_platform.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/platform_gpio.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/platform_mcu_powersave.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_exti.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_gpio.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_rcc.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_sdio.c
    43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_syscfg.c
)

# Add a library with the above sources
add_library(${TARGET}
    ${WICED_SDK_SRC}
)

# Preprocessor macros
target_compile_definitions(${TARGET}
    PRIVATE
        STM32F412xG
        HSE_VALUE=26000000
        USE_STDPERIPH_DRIVER
        WICED_CUSTOM_NX_USER_H
        wifi_firmware_image=resources_firmware_DIR_43362_DIR_43362A2_bin
)

target_include_directories(${TARGET}
    PUBLIC
        inc
        include

        43xxx_Wi-Fi/include
        43xxx_Wi-Fi/libraries/inputs/gpio_button
        43xxx_Wi-Fi/libraries/filesystems/wicedfs/src
        43xxx_Wi-Fi/libraries/utilities/crc
        43xxx_Wi-Fi/libraries/utilities/linked_list
        43xxx_Wi-Fi/libraries/utilities/ring_buffer
        43xxx_Wi-Fi/libraries/utilities/tlv
        43xxx_Wi-Fi/libraries/utilities/wifi
        43xxx_Wi-Fi/platforms/BCM943362WCD6
        43xxx_Wi-Fi/WICED/network/NetX_Duo/WICED
        43xxx_Wi-Fi/WICED/network/NetX_Duo/WWD
        43xxx_Wi-Fi/WICED/platform/ARM_CM4
        43xxx_Wi-Fi/WICED/platform/ARM_CM4/CMSIS
        43xxx_Wi-Fi/WICED/platform/GCC
        43xxx_Wi-Fi/WICED/platform/include
        43xxx_Wi-Fi/WICED/platform/MCU
        43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx
        43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals
        43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries
        43xxx_Wi-Fi/WICED/platform/MCU/STM32F4xx/peripherals/libraries/inc
        43xxx_Wi-Fi/WICED/RTOS/ThreadX/WICED
        43xxx_Wi-Fi/WICED/RTOS/ThreadX/WWD
        43xxx_Wi-Fi/WICED/RTOS/ThreadX/WWD/CM3_CM4
        43xxx_Wi-Fi/WICED/security/BESL/crypto_internal
        43xxx_Wi-Fi/WICED/security/BESL/host/WICED
        43xxx_Wi-Fi/WICED/security/BESL/include
        43xxx_Wi-Fi/WICED/security/PostgreSQL/include
        43xxx_Wi-Fi/WICED/WWD
        43xxx_Wi-Fi/WICED/WWD/include
        43xxx_Wi-Fi/WICED/WWD/include/network
        43xxx_Wi-Fi/WICED/WWD/include/RTOS
        43xxx_Wi-Fi/WICED/WWD/internal
        43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO
        43xxx_Wi-Fi/WICED/WWD/internal/chips/43362
)

target_link_libraries(${TARGET}
    PRIVATE
        azrtos::threadx
        azrtos::netxduo
)

add_custom_target(${TARGET}.stripped ALL 
    DEPENDS ${TARGET}
    COMMAND ${CMAKE_STRIP} --strip-debug lib${TARGET}.a
    COMMAND ${CMAKE_COMMAND} -E copy lib${TARGET}.a ../../lib/
)
