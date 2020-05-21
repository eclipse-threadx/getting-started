# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET atmel_start)

set(SOURCES
    atmel_start.c
    driver_init.c
    ethernet_phy/ethernet_phy.c
    ethernet_phy_main.c
    hal/src/hal_atomic.c
    hal/src/hal_cache.c
    hal/src/hal_delay.c
    hal/src/hal_gpio.c
    hal/src/hal_i2c_m_sync.c
    hal/src/hal_init.c
    hal/src/hal_io.c
    hal/src/hal_mac_async.c
    hal/src/hal_sleep.c
    hal/src/hal_usart_sync.c
    hal/utils/src/utils_assert.c
    hal/utils/src/utils_event.c
    hal/utils/src/utils_list.c
    hal/utils/src/utils_syscalls.c
    hpl/cmcc/hpl_cmcc.c
    hpl/core/hpl_core_m4.c
    hpl/core/hpl_init.c
    hpl/dmac/hpl_dmac.c
    hpl/gclk/hpl_gclk.c
    hpl/gmac/hpl_gmac.c
    hpl/mclk/hpl_mclk.c
    hpl/osc32kctrl/hpl_osc32kctrl.c
    hpl/oscctrl/hpl_oscctrl.c
    hpl/pm/hpl_pm.c
    hpl/ramecc/hpl_ramecc.c
    hpl/sercom/hpl_sercom.c
    stdio_redirect/read.c
    stdio_redirect/write.c
    stdio_redirect/stdio_io.c
    stdio_start.c
    temperature_sensor/BME280_WeatherClick/Bosch_BME280.c
    temperature_sensor/temperature_sensor.c
)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} 
    INTERFACE
        ${SOURCES})

target_compile_definitions(${TARGET}
    INTERFACE 
        __SAME54P20A__
)

target_include_directories(${TARGET}
    INTERFACE
        .
        CMSIS/Core/Include
        config
        ethernet_phy
        hal/include
        hal/utils/include
        hpl/cmcc
        hpl/core
        hpl/dmac
        hpl/gclk
        hpl/gmac
        hpl/mclk
        hpl/osc32kctrl
        hpl/oscctrl
        hpl/pm
        hpl/port
        hpl/ramecc
        hpl/sercom
        hri
        include
        stdio_redirect
        temperature_sensor
        temperature_sensor/BME280_WeatherClick
)

target_link_libraries(${TARGET}
    INTERFACE
        threadx
        netxduo
)
