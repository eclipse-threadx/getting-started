# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Define the CPU architecture for Threadx
set(THREADX_ARCH "cortex_m33")
set(THREADX_TOOLCHAIN "gnu")

# Define ThreadX user configuration
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")

# Core libraries
add_subdirectory(${CORE_LIB_DIR}/threadx threadx)
#add_subdirectory(${CORE_LIB_DIR}/netxduo netxduo)
#add_subdirectory(${CORE_LIB_DIR}/jsmn jsmn)

add_subdirectory(AN524)
add_subdirectory(tf-m)
