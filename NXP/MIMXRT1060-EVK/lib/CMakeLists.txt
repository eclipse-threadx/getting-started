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

add_subdirectory(${CORE_SRC_DIR} core_src)

# Azure RTOS
add_subdirectory(${CORE_LIB_DIR}/threadx threadx)
add_subdirectory(${CORE_LIB_DIR}/netxduo netxduo)

# Azure RTOS IoT 
add_subdirectory(${CORE_LIB_DIR} azure_iot)
add_subdirectory(${CORE_LIB_DIR}/azure-iot-preview/nx_cloud nx_cloud)

# Azure IoT Embedded SDK
set(ENV{AZ_SDK_C_NO_SAMPLES} "TRUE")
add_subdirectory(${CORE_LIB_DIR}/azure-sdk-for-c azure-sdk-for-c)

# Bind the nx_cloud preview into the existing netxduo
target_include_directories(netxduo 
    PUBLIC
        ${CORE_LIB_DIR}/azure-iot-preview/nx_cloud
)

add_subdirectory(netx_driver)
add_subdirectory(MIMXRT1060-evk)
