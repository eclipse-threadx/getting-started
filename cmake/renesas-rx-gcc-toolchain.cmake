# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR rx)
set(TARGET_TRIPLET "rx-elf-")

# default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

# do some windows specific logic
if(WIN32)
    set(TOOLCHAIN_EXT ".exe")
else()
    set(TOOLCHAIN_EXT "")
endif(WIN32)

find_program(COMPILER_ON_PATH "${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT}")

if(DEFINED ENV{RX_GCC_PATH}) 
    # use the environment variable first    
    file(TO_CMAKE_PATH $ENV{RX_GCC_PATH} RX_TOOLCHAIN_PATH)
    message(STATUS "Using ENV variable RX_GCC_PATH = ${RX_TOOLCHAIN_PATH}")
elseif(COMPILER_ON_PATH) 
    # then check on the current path
    get_filename_component(RX_TOOLCHAIN_PATH ${COMPILER_ON_PATH} DIRECTORY)
    message(STATUS "Using RX GCC from path = ${RX_TOOLCHAIN_PATH}")
else()
    message(FATAL_ERROR "Unable to find GCC. Either add to your PATH, or define RX_GCC_PATH to the compiler location")
endif()

# Perform compiler test with the static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER    ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT} CACHE STRING "")
set(CMAKE_CXX_COMPILER  ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}g++${TOOLCHAIN_EXT} CACHE STRING "")
set(CMAKE_ASM_COMPILER  ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_LINKER        ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_SIZE_UTIL     ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}size${TOOLCHAIN_EXT})
set(CMAKE_OBJCOPY       ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP       ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}objdump${TOOLCHAIN_EXT})
set(CMAKE_NM_UTIL       ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}nm${TOOLCHAIN_EXT})
set(CMAKE_AR            ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}ar${TOOLCHAIN_EXT})
set(CMAKE_RANLIB        ${RX_TOOLCHAIN_PATH}/${TARGET_TRIPLET}ranlib${TOOLCHAIN_EXT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_COMMON_FLAGS "-g3 -ffunction-sections -fdata-sections -fdiagnostics-parseable-fixits -fno-strict-aliasing -fno-builtin -fno-common -Wuninitialized -Wdouble-promotion -Werror -Wno-unused-function -Wno-unused-parameter -Wno-incompatible-pointer-types")
set(CMAKE_C_FLAGS 	"${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} ${LIBC_INCLUDE}")
set(CMAKE_CXX_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS} --specs=nano.specs -Wl,--gc-sections -nostartfiles")

set(CMAKE_C_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_ASM_FLAGS_DEBUG "-O0")
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")

set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
