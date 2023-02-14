# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "cortex_m33")
set(THREADX_TOOLCHAIN "iar")

# Compiler flags needed to compile for this CPU
set(CPU_FLAGS "--cpu Cortex-M33 --fpu VFPv5_sp")

include(${CMAKE_CURRENT_LIST_DIR}/arm-iar-cortex-toolchain.cmake)
