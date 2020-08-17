# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)
project(mimxrt1060_azure_iot C ASM)

set(CMAKE_C_STANDARD 99)

set(GSG_BASE_DIR ${PROJECT_SOURCE_DIR}/../..)
set(CORE_SRC_DIR ${GSG_BASE_DIR}/core/src)
set(CORE_LIB_DIR ${GSG_BASE_DIR}/core/lib)

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)

include(utilities)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

add_subdirectory(${CORE_SRC_DIR} core_src)
add_subdirectory(lib)
add_subdirectory(app)
