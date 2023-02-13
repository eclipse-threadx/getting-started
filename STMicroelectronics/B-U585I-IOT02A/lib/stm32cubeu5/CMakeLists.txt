# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    netx_driver/mxchip/mx_wifi_azure_rtos.c
    netx_driver/mxchip/nx_driver_emw3080.c
    
    stmcubemx/stm32u5xx_hal_msp.c
    stmcubemx/stm32u5xx_hal_timebase_tim.c
    stmcubemx/stm32u5xx_it.c
    stmcubemx/system_stm32u5xx.c

    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_cortex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_exti.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gpio.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gtzc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_icache.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rng.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rng_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_adc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_crc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_crs.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_dma.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_exti.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_gpio.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_i2c.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_icache.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_pwr.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_rcc.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_rng.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_spi.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_tim.c
    Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_utils.c

    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.c
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.c
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.c
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.c

    Drivers/BSP/Components/mx_wifi/mx_wifi.c
    Drivers/BSP/Components/mx_wifi/core/mx_wifi_hci.c
    Drivers/BSP/Components/mx_wifi/core/mx_wifi_ipc.c
    Drivers/BSP/Components/mx_wifi/io_pattern/mx_wifi_spi.c
    Drivers/BSP/Components/lsm6dso/lsm6dso.c
    Drivers/BSP/Components/lsm6dso/lsm6dso_reg.c
    Drivers/BSP/Components/iis2mdc/iis2mdc.c
    Drivers/BSP/Components/iis2mdc/iis2mdc_reg.c
    Drivers/BSP/Components/hts221/hts221.c
    Drivers/BSP/Components/hts221/hts221_reg.c
    Drivers/BSP/Components/lps22hh/lps22hh.c
    Drivers/BSP/Components/lps22hh/lps22hh_reg.c
)

set(PROJECT stm32cubeu5)

add_library(${PROJECT} OBJECT
    ${SOURCES})

# MXChip NetX Driver require them
target_link_libraries(${PROJECT}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
)

target_compile_definitions(${PROJECT}
    PUBLIC
        DEBUG
        STM32U585xx
        USE_HAL_DRIVER
)

target_include_directories(${PROJECT} 
    PUBLIC
        stmcubemx
        Drivers/BSP/B-U585I-IOT02A
        Drivers/BSP/Components/mx_wifi
        Drivers/CMSIS/Device/ST/STM32U5xx/Include
        Drivers/CMSIS/Include
        Drivers/STM32U5xx_HAL_Driver/Inc
        netx_driver/mxchip
)
