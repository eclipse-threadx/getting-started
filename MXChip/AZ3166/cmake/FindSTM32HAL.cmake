set(HAL_COMPONENTS adc can cec cortex crc cryp dac dcmi dma dma2d eth exti flash
                    flash_ramfunc fmpi2c gpio hash hcd i2c i2s irda iwdg ltdc
                    nand nor pccard pcd pwr qspi rcc rng rtc sai sd sdram
                    smartcard spdifrx spi sram tim timebase_tim uart usart wwdg)

set(HAL_REQUIRED_COMPONENTS cortex pwr rcc)

# Components that have _ex sources
set(HAL_EX_COMPONENTS adc cryp dac dcmi dma flash fmpi2c hash i2c i2s pcd
                        pwr rcc rtc sai tim)

set(HAL_PREFIX stm32f4xx_)


set(HAL_HEADERS
	${HAL_PREFIX}hal.h
	${HAL_PREFIX}hal_def.h
)

set(HAL_SRCS
	${HAL_PREFIX}hal.c
)

if(NOT STM32HAL_FIND_COMPONENTS)
    set(STM32HAL_FIND_COMPONENTS ${HAL_COMPONENTS})
    message(STATUS "No STM32HAL components selected, using all: ${STM32HAL_FIND_COMPONENTS}")
else()
    message(STATUS "STM32HAL components selected, using: ${STM32HAL_FIND_COMPONENTS}")
endif()

foreach(cmp ${HAL_REQUIRED_COMPONENTS})
    list(FIND STM32HAL_FIND_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(${STM32HAL_FOUND_INDEX} LESS 0)
        list(APPEND STM32HAL_FIND_COMPONENTS ${cmp})
    endif()
endforeach()

foreach(cmp ${STM32HAL_FIND_COMPONENTS})
    list(FIND HAL_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(${STM32HAL_FOUND_INDEX} LESS 0)
        message(FATAL_ERROR "Unknown STM32HAL component: ${cmp}. Available components: ${HAL_COMPONENTS}")
	else()
        list(APPEND HAL_HEADERS ${HAL_PREFIX}hal_${cmp}.h)
        list(APPEND HAL_SRCS ${HAL_PREFIX}hal_${cmp}.c)
        endif()
    list(FIND HAL_EX_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(NOT (${STM32HAL_FOUND_INDEX} LESS 0))
        list(APPEND HAL_HEADERS ${HAL_PREFIX}hal_${cmp}_ex.h)
        list(APPEND HAL_SRCS ${HAL_PREFIX}hal_${cmp}_ex.c)
    endif()
endforeach()

list(REMOVE_DUPLICATES HAL_HEADERS)
list(REMOVE_DUPLICATES HAL_SRCS)

find_path(STM32HAL_INCLUDE_DIR ${HAL_HEADERS}
    HINTS ${STM32Cube_DIR}/Drivers/STM32${STM32_FAMILY}xx_HAL_Driver/Inc
    CMAKE_FIND_ROOT_PATH_BOTH
)

foreach(HAL_SRC ${HAL_SRCS})
    string(MAKE_C_IDENTIFIER "${HAL_SRC}" HAL_SRC_CLEAN)
    set(HAL_${HAL_SRC_CLEAN}_FILE HAL_SRC_FILE-NOTFOUND)
    find_file(HAL_${HAL_SRC_CLEAN}_FILE ${HAL_SRC}
        HINTS ${STM32Cube_DIR}/Drivers/STM32${STM32_FAMILY}xx_HAL_Driver/Src
        CMAKE_FIND_ROOT_PATH_BOTH
    )
    list(APPEND STM32HAL_SOURCES ${HAL_${HAL_SRC_CLEAN}_FILE})
endforeach()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(STM32HAL DEFAULT_MSG STM32HAL_INCLUDE_DIR STM32HAL_SOURCES)
