# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(MCPU_FLAGS "-m64bit-doubles -mcpu=rx64m -misa=v2 -mlittle-endian-data")

include(${CMAKE_CURRENT_LIST_DIR}/renesas-rx-gcc-toolchain.cmake)
