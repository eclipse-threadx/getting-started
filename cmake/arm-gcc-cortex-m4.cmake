# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "gnu")

set(MCPU_FLAGS "-mthumb -mcpu=cortex-m4")
set(VFP_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")

include(${CMAKE_CURRENT_LIST_DIR}/arm-gcc-cortex-toolchain.cmake)
