set(STM32_FAMILY "F4")
set(STM32Cube_DIR "./")

find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS dma flash gpio tim uart usart)

set(TARGET stm32cubef4)

add_library(${TARGET} INTERFACE)

target_sources(${TARGET} INTERFACE ${STM32HAL_SOURCES})

target_compile_definitions(${TARGET}
    INTERFACE 
        STM32F412Rx
        USE_HAL_DRIVER
)
        
target_include_directories(${TARGET}
    INTERFACE
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
)

