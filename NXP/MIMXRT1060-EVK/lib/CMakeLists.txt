# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

# FIXME tried to optimize dependency and do not include nx_cloud in this repo
# # nx_cloud
# CPMAddPackage(
#     NAME azure_iot_preview
#     GIT_REPOSITORY https://github.com/azure-rtos/azure-iot-preview
#     GIT_TAG master
# )
# if (azure_iot_preview_ADDED)
#     add_subdirectory(azure_iot_preview/nx_cloud)
# #   add_library(azure_iot_preview/nx_cloud INTERFACE IMPORTED)
# #   target_include_directories(nx_cloud INTERFACE ${azure_iot_preview_SOURCE_DIR}/nx_cloud)
# endif()

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")
CPMAddPackage(
    NAME threadx
    GIT_REPOSITORY https://github.com/azure-rtos/threadx.git
    GIT_TAG master
)

# netX-duo
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")
CPMAddPackage(
    NAME netxduo
    GIT_REPOSITORY https://github.com/azure-rtos/netxduo.git
    GIT_TAG master
)

add_subdirectory(netx_driver)
add_subdirectory(MIMXRT1060-evk)
