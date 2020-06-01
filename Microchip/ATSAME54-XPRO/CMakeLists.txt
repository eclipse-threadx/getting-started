# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.14)
project(atsame54_azure_iot C ASM)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

set(CMAKE_C_STANDARD 99)

include(utilities)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
