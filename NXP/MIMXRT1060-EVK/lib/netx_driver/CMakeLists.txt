set(TARGET netx_driver)

set(SOURCES
    nx_driver_imxrt1062.c
    nx_driver_imxrt1062.h
    nx_driver_imxrt1062_low_level.S
)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} 
    INTERFACE
        ${SOURCES})

target_include_directories(${TARGET}
    INTERFACE
        .
)

target_link_libraries(${TARGET} 
    INTERFACE
        azrtos::threadx
        azrtos::netxduo
        MIMXRT1060-evk
)
