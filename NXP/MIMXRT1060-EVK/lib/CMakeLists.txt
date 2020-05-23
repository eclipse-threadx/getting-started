# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

include(CPM)

# define the cpu architecture for threadx
set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

# threadX
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")
CPMAddPackage(
    NAME threadx
    GIT_REPOSITORY https://github.com/azure-rtos/threadx.git
    GIT_TAG master
)

# netx-duo
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")
CPMAddPackage(
    NAME netxduo
    GIT_REPOSITORY https://github.com/azure-rtos/netxduo.git
    GIT_TAG master
)

# Azure IoT C-SDK
set(ENV{AZ_SDK_C_NO_SAMPLES} "TRUE")
CPMAddPackage(
    NAME az
    GIT_REPOSITORY https://github.com/Azure/azure-sdk-for-c.git
    GIT_TAG master
)
#if(azure_sdk_for_c_ADDED) 
#    add_subdirectory(
#        ${azure_sdk_for_c_SOURCE_DIR}/sdk/iot/common 
#        ${CMAKE_CURRENT_BINARY_DIR}/azure_sdk_for_c_common) 
#    add_subdirectory(
#        ${azure_sdk_for_c_SOURCE_DIR}/sdk/iot/hub 
#        ${CMAKE_CURRENT_BINARY_DIR}/azure_sdk_for_c_hub) 
#endif()

# Azure RTOS IoT C-SDK CL
CPMAddPackage(
    NAME azure_iot
    GIT_REPOSITORY https://github.com/azure-rtos/azure-iot-preview.git
    GIT_TAG master
    DOWNLOAD_ONLY True
)
if(azure_iot_ADDED) 
    add_subdirectory(
        ${azure_iot_SOURCE_DIR}/nx_cloud
        ${CMAKE_CURRENT_BINARY_DIR}/nx_cloud) 
    add_subdirectory(
        ${azure_iot_SOURCE_DIR}/azure_iot
        ${CMAKE_CURRENT_BINARY_DIR}/azure_iot) 
endif()

# Bind the nx_cloud preview into the existing netxduo
target_include_directories(netxduo 
    PUBLIC
        ${azure_iot_SOURCE_DIR}/nx_cloud
)

add_subdirectory(netx_driver)
add_subdirectory(MIMXRT1060-evk)
