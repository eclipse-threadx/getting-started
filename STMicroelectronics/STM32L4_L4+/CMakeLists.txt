# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)

set(GSG_BASE_DIR ${CMAKE_SOURCE_DIR}/../..)
set(CORE_SRC_DIR ${GSG_BASE_DIR}/core/src)
set(CORE_LIB_DIR ${GSG_BASE_DIR}/core/lib)

# Set the toolchain if not defined
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${GSG_BASE_DIR}/cmake/arm-gcc-cortex-m4.cmake")
endif()

include(${GSG_BASE_DIR}/cmake/utilities.cmake)

# Define the Project
project(stm32l4_azure_iot C ASM)

# Disable common networking component, STM has it's own
set(DISABLE_COMMON_NETWORK true)

add_subdirectory(${CORE_SRC_DIR} core_src)
add_subdirectory(lib)
add_subdirectory(app)
