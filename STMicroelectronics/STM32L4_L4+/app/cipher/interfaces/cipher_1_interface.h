// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from cipher v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef CIPHER_1_INTERFACE_H
#define CIPHER_1_INTERFACE_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

/*
 * interface definition
 */
#define CIPHER_1_INTERFACE_NAME "cipher"
#define CIPHER_1_INTERFACE_VERSION 1
#define CIPHER_1_CAPABILITY_SIZE 2

/*
 * Define encrypt command on cipher interface.
 */
#define CIPHER_1_INTERFACE_ENCRYPT_COMMAND (az_ulib_capability_index)0
#define CIPHER_1_INTERFACE_ENCRYPT_COMMAND_NAME "encrypt"
  typedef struct
  {
    uint32_t context;
    const char* const src;
    uint32_t src_size;
    uint32_t dst_buffer_size;
  } cipher_1_encrypt_model_in;
  typedef struct
  {
    char* dst;
    uint32_t* dst_size;
  } cipher_1_encrypt_model_out;

/*
 * Define decrypt command on cipher interface.
 */
#define CIPHER_1_INTERFACE_DECRYPT_COMMAND (az_ulib_capability_index)1
#define CIPHER_1_INTERFACE_DECRYPT_COMMAND_NAME "decrypt"
  typedef struct
  {
    const char* const src;
    uint32_t src_size;
    uint32_t dst_buffer_size;
  } cipher_1_decrypt_model_in;
  typedef struct
  {
    char* dst;
    uint32_t* dst_size;
  } cipher_1_decrypt_model_out;

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_1_INTERFACE_H */
