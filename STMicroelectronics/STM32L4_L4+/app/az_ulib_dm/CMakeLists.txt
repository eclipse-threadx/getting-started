#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_library(az_ulib_dm
  ${CMAKE_CURRENT_LIST_DIR}/az_ulib_dm.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(az_ulib_dm
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(az_ulib_dm
  az::ulib
  cipher
)

add_library(az::adu ALIAS az_ulib_dm)
