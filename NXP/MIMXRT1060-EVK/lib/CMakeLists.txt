# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

# Define ThreadX user configuration
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")

# Define NetXDuo user configuration
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")

add_subdirectory(${GSG_BASE_DIR}/core/src core_src)
add_subdirectory(${GSG_BASE_DIR}/core/lib/threadx threadx)
add_subdirectory(${GSG_BASE_DIR}/core/lib/netxduo netxduo)

add_subdirectory(netx_driver)
add_subdirectory(MIMXRT1060-evk)
