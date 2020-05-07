set(SOURCES
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_cortex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_dma_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_dma.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_flash_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_flash_ramfunc.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_flash.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_gpio.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_i2c_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_i2c.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_pwr_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_pwr.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_rcc_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_rcc.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_tim_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_tim.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_uart.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_uart_ex.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_spi.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_rng.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_rtc.c
    Drivers/STM32L4xx_HAL_Driver/src/stm32l4xx_hal_rtc_ex.c

    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_accelero.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_gyro.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_hsensor.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_magneto.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_psensor.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_tsensor.c
    Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01.c
    Drivers/BSP/Components/lsm6dsl/lsm6dsl.c
    Drivers/BSP/Components/lis3mdl/lis3mdl.c
    Drivers/BSP/Components/hts221/hts221.c
    Drivers/BSP/Components/lps22hb/lps22hb.c
)

set(TARGET stm32cubel4)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET}
    INTERFACE
        ${SOURCES})

target_compile_definitions(${TARGET}
    INTERFACE 
        STM32L475xx
        USE_HAL_DRIVER
        USE_STM32L475E_IOT01
)

target_include_directories(${TARGET} 
    INTERFACE
        Drivers/BSP/B-L475E-IOT01
        Drivers/CMSIS/Device/ST/STM32L4xx/Include
        Drivers/CMSIS/Include
        Drivers/STM32L4xx_HAL_Driver/inc
)
