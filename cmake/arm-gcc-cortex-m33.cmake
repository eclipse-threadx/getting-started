# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m33)

set(THREADX_ARCH "cortex_m33")
set(THREADX_TOOLCHAIN "gnu")

set(MCPU_FLAGS "-mthumb -mcpu=cortex-m33+nodsp+nofp")
set(VFP_FLAGS "-mfloat-abi=softfp")
set(SPEC_FLAGS "--specs=nosys.specs")

include(${CMAKE_CURRENT_LIST_DIR}/arm-gcc-cortex-toolchain.cmake)
