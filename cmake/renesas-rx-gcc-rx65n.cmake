# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "rxv2")
set(THREADX_TOOLCHAIN "gnu")

set(MCPU_FLAGS "-m64bit-doubles -mcpu=rx64m -misa=v2 -mlittle-endian-data")

include(${CMAKE_CURRENT_LIST_DIR}/renesas-rx-gcc-toolchain.cmake)
