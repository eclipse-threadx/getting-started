set(SOURCES
    stm_sensor/Src/lps22hb_reg.c
    stm_sensor/Src/hts221_reg.c 
    stm_sensor/Src/lsm6dsl_reg.c
    stm_sensor/Src/lis2mdl_reg.c
    stm_sensor/Src/lsm6dsl_read_data_polling.c
    stm_sensor/Src/lps22hb_read_data_polling.c
    stm_sensor/Src/hts221_read_data_polling.c
    stm_sensor/Src/lis2mdl_read_data_polling.c
    ssd1306/ssd1306.c
    ssd1306/ssd1306_fonts.c
)

set(TARGET mxchip_bsp)

add_library(${TARGET} OBJECT 
    ${SOURCES})

target_link_libraries(${TARGET}
    stm32cubef4
)
        
target_include_directories(${TARGET}
    PUBLIC
        stm_sensor/Inc
        ssd1306
)
