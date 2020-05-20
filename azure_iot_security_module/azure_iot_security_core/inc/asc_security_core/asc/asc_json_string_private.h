// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _asc_JSON_STRING_PRIVATE_H
#define _asc_JSON_STRING_PRIVATE_H

#include "asc_security_core/asc/asc_span_private.h"
#include "asc_security_core/asc/asc_json.h"
#include "asc_security_core/asc/asc_span.h"

#include <stdint.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

/**
 * Encodes the given character into a JSON escape sequence. The function returns an empty span if
 * the given character doesn't require to be escaped.
 */
ASC_NODISCARD asc_span _asc_json_esc_encode(uint8_t c);

/**
 * TODO: this function and JSON pointer read functions should return proper UNICODE
 *       code-point to be compatible.
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_string_char(asc_span* self, uint32_t* out);

/**
 * Returns a next reference token in the JSON pointer. The JSON pointer parser is @var
 * asc_span_reader.
 *
 * See https://tools.ietf.org/html/rfc6901
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_pointer_token(asc_span* self, asc_span* out);

/**
 * Returns a next character in the given span reader of JSON pointer reference token.
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_pointer_token_char(asc_span* self, uint32_t* out);

ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_span(asc_span span)
{
  return (asc_json_token){
    .kind = ASC_JSON_TOKEN_SPAN,
    ._internal.span = span,
  };
}

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_JSON_STRING_PRIVATE_H
