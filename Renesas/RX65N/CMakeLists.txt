# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)
project(rx65n_azure_iot C ASM)

set(CMAKE_C_STANDARD 99)

# set common option commands
SET(COMMON_OPTIONS "-isa=rxv1 -debug -nofpu" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_OPTIONS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_OPTIONS}")
SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_OPTIONS}")

# set the C/C++/ASM compiler and linker commands
SET (CMAKE_ASM_COMPILE_OBJECT "${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS} -output=<OBJECT> <SOURCE>")
SET (CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -output=obj=<OBJECT> <SOURCE>")
SET (CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -output=obj=<OBJECT> <SOURCE>")
SET (CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} ${LINKER_OPTIONS} -form=absolute <OBJECTS> <LINK_LIBRARIES> -output=<TARGET>")
SET (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} ${LINKER_OPTIONS} -form=absolute <OBJECTS> -output=<TARGET>")

set(GSG_BASE_DIR ${PROJECT_SOURCE_DIR}/../..)
set(CORE_SRC_DIR ${GSG_BASE_DIR}/core/src)
set(CORE_LIB_DIR ${GSG_BASE_DIR}/core/lib)

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)

#include(utilities)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

#add_subdirectory(${CORE_SRC_DIR} core_src)
#add_subdirectory(lib)
add_subdirectory(app)
