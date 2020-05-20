# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.14)
project(mimxrt1060_azure_iot C ASM)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

set(CMAKE_C_STANDARD 99)

include(utilities)

add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(../../nx_cloud ${CMAKE_CURRENT_BINARY_DIR}/nx_cloud)
add_subdirectory(../../azure_iot_security_module ${CMAKE_CURRENT_BINARY_DIR}/azure_iot_security_module)
add_subdirectory(lib)
add_subdirectory(app)
