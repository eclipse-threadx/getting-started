// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "asc_security_core/asc/asc_json_string_private.h"
#include "asc_security_core/asc/asc_json.h"
#include "asc_security_core/asc/asc_precondition.h"

#include "asc_security_core/asc/_asc_cfg.h"

static ASC_NODISCARD asc_result _asc_span_reader_read_json_pointer_char(asc_span* self, uint32_t* out)
{
////  ASC_PRECONDITION_NOT_NULL(self);
  int32_t reader_current_length = asc_span_length(*self);

  // check for EOF
  if (reader_current_length == 0)
  {
    return ASC_ERROR_ITEM_NOT_FOUND;
  }

  uint8_t const result = asc_span_ptr(*self)[0];
  switch (result)
  {
    case '/':
    {
      return ASC_ERROR_JSON_POINTER_TOKEN_END;
    }
    case '~':
    {
      // move reader to next position
      *self = asc_span_slice(*self, 1, -1);
      // check for EOF
      if (asc_span_length(*self) == 0)
      {
        return ASC_ERROR_EOF;
      }
      // get char
      uint8_t const e = asc_span_ptr(*self)[0];
      // move to next position again
      *self = asc_span_slice(*self, 1, -1);
      switch (e)
      {
        case '0':
        {
          *out = '~';
          return ASC_OK;
        }
        case '1':
        {
          *out = '/';
          return ASC_OK;
        }
        default:
        {
          return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
        }
      }
    }
    default:
    {
      // move reader to next position
      *self = asc_span_slice(*self, 1, -1);

      *out = (uint8_t)result;
      return ASC_OK;
    }
  }
}

/**
 * Returns a next reference token in the JSON pointer.
 *
 * See https://tools.ietf.org/html/rfc6901
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_pointer_token(asc_span* self, asc_span* out)
{
  // read `/` if any.
  {
    // check there is something still to read
    if (asc_span_length(*self) == 0)
    {
      return ASC_ERROR_ITEM_NOT_FOUND;
    }
    // ensure first char of pointer is `/`
    if (asc_span_ptr(*self)[0] != '/')
    {
      return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
    }
  }
  // move forward
  *self = asc_span_slice(*self, 1, -1);
  if (asc_span_length(*self) == 0)
  {
    *out = *self;
    return ASC_OK;
  }

  // What's happening below: Keep reading/scaning until POINTER_TOKEN_END is found or we get to the
  // end of a Json token. var begin will record the number of bytes read until token_end or
  // pointer_end. TODO: We might be able to implement _asc_span_scan_until() here, since we ignore
  // the out of _asc_span_reader_read_json_pointer_char()
  int32_t initial_capacity = asc_span_capacity(*self);
  uint8_t* p_reader = asc_span_ptr(*self);
  while (true)
  {
    uint32_t ignore = { 0 };
    asc_result const result = _asc_span_reader_read_json_pointer_char(self, &ignore);
    switch (result)
    {
      case ASC_ERROR_ITEM_NOT_FOUND:
      case ASC_ERROR_JSON_POINTER_TOKEN_END:
      {
        int32_t current_capacity = initial_capacity - asc_span_capacity(*self);
        *out = asc_span_init(p_reader, current_capacity, current_capacity);
        return ASC_OK;
      }
      default:
      {
        ASC_RETURN_IF_FAILED(result);
      }
    }
  }
}

/**
 * Returns a next character in the given span reader of JSON pointer reference token.
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_pointer_token_char(asc_span* self, uint32_t* out)
{
  uint32_t c;
  asc_result const result = _asc_span_reader_read_json_pointer_char(self, &c);
  if (result == ASC_ERROR_JSON_POINTER_TOKEN_END)
  {
    return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
  }
  ASC_RETURN_IF_FAILED(result);
  *out = c;
  return ASC_OK;
}

ASC_NODISCARD static bool asc_json_pointer_token_eq_json_string(
    asc_span pointer_token,
    asc_span json_string)
{
  while (true)
  {
    uint32_t pt_c = 0;
    asc_result const pt_result = _asc_span_reader_read_json_pointer_token_char(&pointer_token, &pt_c);
    uint32_t js_c = 0;
    asc_result const js_result = _asc_span_reader_read_json_string_char(&json_string, &js_c);
    if (js_result == ASC_ERROR_ITEM_NOT_FOUND && pt_result == ASC_ERROR_ITEM_NOT_FOUND)
    {
      return true;
    }
    if (asc_failed(js_result) || asc_failed(pt_result))
    {
      return false;
    }
    if (pt_c != js_c)
    {
      return false;
    }
  }
}

ASC_NODISCARD static asc_result asc_json_parser_get_by_pointer_token(
    asc_json_parser* json_parser,
    asc_span pointer_token,
    asc_json_token* inout_token)
{
  switch (inout_token->kind)
  {
    case ASC_JSON_TOKEN_ARRAY_START:
    {
      uint64_t i = 0;
      ASC_RETURN_IF_FAILED(asc_span_to_uint64(pointer_token, &i));
      while (true)
      {
        ASC_RETURN_IF_FAILED(asc_json_parser_parse_array_item(json_parser, inout_token));
        if (i == 0)
        {
          return ASC_OK;
        }
        --i;
        ASC_RETURN_IF_FAILED(asc_json_parser_skip_children(json_parser, *inout_token));
      }
    }
    case ASC_JSON_TOKEN_OBJECT_START:
    {
      while (true)
      {
        asc_json_token_member token_member = { 0 };
        ASC_RETURN_IF_FAILED(asc_json_parser_parse_token_member(json_parser, &token_member));
        if (asc_json_pointer_token_eq_json_string(pointer_token, token_member.name))
        {
          *inout_token = token_member.token;
          return ASC_OK;
        }
        ASC_RETURN_IF_FAILED(asc_json_parser_skip_children(json_parser, token_member.token));
      }
    }
    default:
      return ASC_ERROR_ITEM_NOT_FOUND;
  }
}

ASC_NODISCARD asc_result
asc_json_parse_by_pointer(asc_span json_buffer, asc_span json_pointer, asc_json_token* out_token)
{
////  ASC_PRECONDITION_NOT_NULL(out_token);

  asc_json_parser json_parser = { 0 };
  ASC_RETURN_IF_FAILED(asc_json_parser_init(&json_parser, json_buffer));

  ASC_RETURN_IF_FAILED(asc_json_parser_parse_token(&json_parser, out_token));
  while (true)
  {
    asc_span pointer_token = ASC_SPAN_NULL;
    asc_result const result = _asc_span_reader_read_json_pointer_token(&json_pointer, &pointer_token);
    if (result == ASC_ERROR_ITEM_NOT_FOUND)
    {
      return ASC_OK; // no more pointer tokens so we found the JSON value.
    }
    ASC_RETURN_IF_FAILED(result);
    ASC_RETURN_IF_FAILED(
        asc_json_parser_get_by_pointer_token(&json_parser, pointer_token, out_token));
  }
}
