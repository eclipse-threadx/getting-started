# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define ThreadX user configuration
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")

# Define NetXDuo user configuration
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_AZURE_IOT ON CACHE BOOL "Enable Azure IoT")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")

# Azure Security Center
set(NX_AZURE_DISABLE_IOT_SECURITY_MODULE OFF CACHE BOOL "Security Center")

# Core libraries
add_subdirectory(${CORE_LIB_DIR}/threadx threadx)
add_subdirectory(${CORE_LIB_DIR}/jsmn jsmn)

# Using a local copy of NetXDuo that has been customized for this boards WiFi module
add_subdirectory(netxduo)

add_subdirectory(stm32cubel4)
add_subdirectory(netx_driver)
