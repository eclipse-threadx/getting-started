# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(TARGET atmel_start)

set(SOURCES
    src/atmel_start.c
    src/driver_init.c
    src/ethernet_phy_main.c
    src/stdio_start.c
    src/ethernet_phy/ethernet_phy.c
    src/hal/src/hal_atomic.c
    src/hal/src/hal_cache.c
    src/hal/src/hal_delay.c
    src/hal/src/hal_gpio.c
    src/hal/src/hal_i2c_m_sync.c
    src/hal/src/hal_init.c
    src/hal/src/hal_io.c
    src/hal/src/hal_mac_async.c
    src/hal/src/hal_sleep.c
    src/hal/src/hal_usart_sync.c
    src/hpl/cmcc/hpl_cmcc.c
    src/hpl/core/hpl_core_m4.c
    src/hpl/core/hpl_init.c
    src/hpl/dmac/hpl_dmac.c
    src/hpl/gclk/hpl_gclk.c
    src/hpl/gmac/hpl_gmac.c
    src/hpl/mclk/hpl_mclk.c
    src/hpl/osc32kctrl/hpl_osc32kctrl.c
    src/hpl/oscctrl/hpl_oscctrl.c
    src/hpl/pm/hpl_pm.c
    src/hpl/ramecc/hpl_ramecc.c
    src/hpl/sercom/hpl_sercom.c
    src/stdio_redirect/stdio_io.c
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    list(APPEND SOURCES
        src/stdio_redirect/iar/read.c
        src/stdio_redirect/iar/write.c
    )
else()
    list(APPEND SOURCES
        src/stdio_redirect/gcc/read.c
        src/stdio_redirect/gcc/write.c
    )
endif()

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_compile_definitions(${TARGET}
    PUBLIC 
        __SAME54P20A__
)

target_compile_options(${TARGET}
    PRIVATE
        -Wno-error
)

target_include_directories(${TARGET}
    PUBLIC
        config
        src
        src/CMSIS/Core/Include
        src/ethernet_phy
        src/hal/include
        src/hal/utils/include
        src/hpl/cmcc
        src/hpl/core
        src/hpl/dmac
        src/hpl/gclk
        src/hpl/gmac
        src/hpl/mclk
        src/hpl/osc32kctrl
        src/hpl/oscctrl
        src/hpl/pm
        src/hpl/port
        src/hpl/ramecc
        src/hpl/sercom
        src/hri
        src/include
        src/stdio_redirect
)

target_link_libraries(${TARGET}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
)
