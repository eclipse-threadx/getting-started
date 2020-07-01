# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.14)

project(mxchip_azure_iot C ASM)

set(CMAKE_C_STANDARD 99)
set(GSG_BASE_DIR ${PROJECT_SOURCE_DIR}/../..)

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(utilities)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(lib)
add_subdirectory(app)
