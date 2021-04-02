#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_library(cipher
  ${CMAKE_CURRENT_LIST_DIR}/cipher_v1i1/cipher_v1i1.c
  ${CMAKE_CURRENT_LIST_DIR}/cipher_v1i1/interfaces/cipher_v1i1_interface.c
  ${CMAKE_CURRENT_LIST_DIR}/cipher_v2i1/cipher_v2i1.c
  ${CMAKE_CURRENT_LIST_DIR}/cipher_v2i1/interfaces/cipher_v2i1_interface.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(cipher
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/interfaces
    ${CMAKE_CURRENT_LIST_DIR}/cipher_v1i1
    ${CMAKE_CURRENT_LIST_DIR}/cipher_v1i1/interfaces
    ${CMAKE_CURRENT_LIST_DIR}/cipher_v2i1
    ${CMAKE_CURRENT_LIST_DIR}/cipher_v2i1/interfaces
)

target_link_libraries(cipher
  az::ulib
)
