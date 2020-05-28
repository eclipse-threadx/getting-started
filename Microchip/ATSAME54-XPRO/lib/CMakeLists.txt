# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "gnu")

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "")
CPMAddPackage(
    NAME threadx
    GIT_REPOSITORY https://github.com/azure-rtos/threadx.git
    GIT_TAG master
)

# netX-duo
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo.")
CPMAddPackage(
    NAME netxduo
    GIT_REPOSITORY https://github.com/azure-rtos/netxduo.git
    GIT_TAG master
)

add_subdirectory(netx_driver)
add_subdirectory(atmel_start)