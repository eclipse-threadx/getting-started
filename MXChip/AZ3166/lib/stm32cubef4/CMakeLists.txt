set(STM32_FAMILY "F4")
set(STM32Cube_DIR "./")

find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS dma flash gpio tim timebase_tim uart usart i2c)

set(TARGET stm32cubef4)

add_library(${TARGET} OBJECT 
    ${STM32HAL_SOURCES})

target_compile_definitions(${TARGET}
    PUBLIC
        STM32F412Rx
        USE_HAL_DRIVER
        STM32F4
)
        
target_include_directories(${TARGET}
    PUBLIC
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        config
)
