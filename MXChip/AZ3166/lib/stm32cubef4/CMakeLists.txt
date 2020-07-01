set(STM32_FAMILY "F4")
set(STM32Cube_DIR "./")

find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS dma flash gpio tim uart usart i2c)

list(APPEND STM32HAL_SOURCES
    Drivers/stm_sensor/Src/lps22hb_reg.c
    Drivers/stm_sensor/Src/hts221_reg.c 
    Drivers/stm_sensor/Src/lsm6dsl_reg.c
    Drivers/stm_sensor/Src/lis2mdl_reg.c
    Drivers/stm_sensor/Src/lsm6dsl_read_data_polling.c
    Drivers/stm_sensor/Src/lps22hb_read_data_polling.c
    Drivers/stm_sensor/Src/hts221_read_data_polling.c
    Drivers/stm_sensor/Src/lis2mdl_read_data_polling.c
)

set(TARGET stm32cubef4)

add_library(${TARGET} OBJECT 
    ${STM32HAL_SOURCES})

target_compile_definitions(${TARGET}
    PUBLIC 
        STM32F412Rx
        USE_HAL_DRIVER
)
        
target_include_directories(${TARGET}
    PUBLIC
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        Drivers/stm_sensor/Inc
        config
)
