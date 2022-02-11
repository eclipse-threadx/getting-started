# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set the EW installation root directory
set(EWARM_ROOT_DIR "C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.4/arm")

# default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

# Set up the CMake variables for compiler and assembler
set(CMAKE_C_COMPILER "${EWARM_ROOT_DIR}/bin/iccarm.exe" "${CPU_FLAGS} -e")
set(CMAKE_CXX_COMPILER "${EWARM_ROOT_DIR}/bin/iccarm.exe" "${CPU_FLAGS} --c++")
set(CMAKE_ASM_COMPILER "${EWARM_ROOT_DIR}/bin/iasmarm.exe" "${CPU_FLAGS}")

set(CMAKE_C_LINK_FLAGS "--semihosting")
set(CMAKE_CXX_LINK_FLAGS "--semihosting")

set(CMAKE_C_FLAGS_DEBUG_INIT "-On")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-On")
