set(CMSIS_COMMON_HEADERS
    arm_common_tables.h
    arm_const_structs.h
    arm_math.h
    core_cmFunc.h
    core_cmInstr.h
    core_cmSimd.h
)

message(STATUS "STM32F4xx library: " ${STM32Cube_DIR})

list(APPEND CMSIS_COMMON_HEADERS core_cm4.h)
set(CMSIS_DEVICE_HEADERS stm32f4xx.h system_stm32f4xx.h)

find_path(CMSIS_COMMON_INCLUDE_DIR ${CMSIS_COMMON_HEADERS}
    # PATH_SUFFIXES include stm32${STM32_FAMILY_LOWER} cmsis
    HINTS ${STM32Cube_DIR}/Drivers/CMSIS/Include/
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_path(CMSIS_DEVICE_INCLUDE_DIR ${CMSIS_DEVICE_HEADERS}
    # PATH_SUFFIXES include stm32${STM32_FAMILY_LOWER} cmsis
    HINTS ${STM32Cube_DIR}/Drivers/CMSIS/Device/ST/STM32${STM32_FAMILY}xx/Include
    CMAKE_FIND_ROOT_PATH_BOTH
)

set(CMSIS_INCLUDE_DIRS
    ${CMSIS_DEVICE_INCLUDE_DIR}
    ${CMSIS_COMMON_INCLUDE_DIR}
)

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CMSIS DEFAULT_MSG CMSIS_INCLUDE_DIRS)
