# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "iar")

# Compiler flags needed to compile for this CPU
set(MCPU_FLAGS "--endian=little --cpu=Cortex-M4 -e --fpu=VFPv4_sp")

include(${CMAKE_CURRENT_LIST_DIR}/arm-iar-cortex-toolchain.cmake)
