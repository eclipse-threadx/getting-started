cmake_minimum_required(VERSION 3.14)
project(mimxrt1060_azure_iot C ASM)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

set(CMAKE_C_STANDARD 99)

add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(lib)
add_subdirectory(app)

feature_summary(WHAT ALL)