// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef CIPHER_V1I1_H
#define CIPHER_V1I1_H

#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  az_result cipher_v1i1_create(void);
  az_result cipher_v1i1_destroy(void);

  az_result cipher_v1i1_encrypt(
      uint32_t context,
      const char* const src,
      uint32_t src_size,
      uint32_t dst_buffer_size,
      char* dst,
      uint32_t* dst_size);

  az_result cipher_v1i1_decrypt(
      const char* const src,
      uint32_t src_size,
      uint32_t dst_buffer_size,
      char* dst,
      uint32_t* dst_size);

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_V1I1_H */
