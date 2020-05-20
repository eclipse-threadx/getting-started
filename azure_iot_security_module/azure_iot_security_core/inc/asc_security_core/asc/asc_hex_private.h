// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _asc_HEX_PRIVATE_H
#define _asc_HEX_PRIVATE_H

#include <stdint.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

enum
{
  _asc_HEX_LOWER_OFFSET = 'a' - 10,
  _asc_HEX_UPPER_OFFSET = 'A' - 10,
};

/**
 * Converts a number [0..15] into uppercase hexadecimal digit character (base16).
 */
ASC_NODISCARD ASC_INLINE uint8_t _asc_number_to_upper_hex(uint8_t number)
{
  return (uint8_t)(number + (number < 10 ? '0' : _asc_HEX_UPPER_OFFSET));
}

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_HEX_PRIVATE_H
