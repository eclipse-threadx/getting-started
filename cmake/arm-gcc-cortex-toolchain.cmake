# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TARGET_TRIPLET "arm-none-eabi-")

# do some windows specific logic
if(WIN32)
    set(TOOLCHAIN_EXT ".exe")
    
    # use the repo version of ninja on Windows as there is no Ninja installer
    set(CMAKE_MAKE_PROGRAM ${CMAKE_CURRENT_LIST_DIR}/ninja CACHE STRING "Ninja location")
    
    # locate if the arm gcc compiler is installed via visual studio
    execute_process(
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/vswhere.exe -latest -requires Component.MDD.Linux.GCC.arm -find **/gcc_arm/bin
        OUTPUT_VARIABLE VSWHERE_PATH
    )
else()
    set(TOOLCHAIN_EXT "")
endif(WIN32)

# default to Release build
if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Choose the type of build, options are: Debug Release."
            FORCE)
endif()

find_program(COMPILER_ON_PATH "${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT}")

if(DEFINED ENV{ARM_GCC_PATH}) 
    # use the environment variable first    
    file(TO_CMAKE_PATH $ENV{ARM_GCC_PATH} ARM_TOOLCHAIN_PATH)
    message(STATUS "Using ENV variable ARM_GCC_PATH = ${ARM_TOOLCHAIN_PATH}")
elseif(COMPILER_ON_PATH) 
    # then check on the current path
    get_filename_component(ARM_TOOLCHAIN_PATH ${COMPILER_ON_PATH} DIRECTORY)
    message(STATUS "Using ARM GCC from path = ${ARM_TOOLCHAIN_PATH}")
elseif(VSWHERE_PATH) 
    # try and find if its installed with visual studio
    file(TO_CMAKE_PATH ${VSWHERE_PATH} ARM_TOOLCHAIN_PATH)
    if(ARM_TOOLCHAIN_PATH)
        string(STRIP ${ARM_TOOLCHAIN_PATH} ARM_TOOLCHAIN_PATH)
        message(STATUS "Using Visual Studio install ${ARM_TOOLCHAIN_PATH}")
    endif()
else() 
    # otherwise just default to the standard installation
    set(ARM_TOOLCHAIN_PATH "C:/Program Files (x86)/GNU Tools Arm Embedded/9 2019-q4-major/bin")
    message(STATUS "Using ARM GCC from default Windows toolchain directory ${ARM_TOOLCHAIN_PATH}")
endif()

# perform compiler test with the static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER    ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT} CACHE STRING "")
set(CMAKE_CXX_COMPILER  ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}g++${TOOLCHAIN_EXT} CACHE STRING "")
set(CMAKE_ASM_COMPILER  ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_LINKER        ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_SIZE_UTIL     ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}size${TOOLCHAIN_EXT})
set(CMAKE_OBJCOPY       ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP       ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}objdump${TOOLCHAIN_EXT})
set(CMAKE_NM_UTIL       ${ARM_TOOLCHAIN_PATH}/${TARGET_TRIPLET}nm${TOOLCHAIN_EXT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_COMMON_FLAGS "-ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall")
set(CMAKE_C_FLAGS 	"${MCPU_FLAGS} ${VFP_FLAGS} ${SPECS_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${SPECS_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${SPECS_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS} -Wl,--gc-sections,-print-memory-usage")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_ASM_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_ASM_FLAGS_DEBUG "-g")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")


