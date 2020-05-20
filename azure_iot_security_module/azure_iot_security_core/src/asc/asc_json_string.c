// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "asc_security_core/asc/asc_hex_private.h"
#include "asc_security_core/asc/asc_json_string_private.h"
#include "asc_security_core/asc/asc_json.h"
#include "asc_security_core/asc/asc_precondition.h"
#include "asc_security_core/asc/asc_precondition_internal.h"

#include <ctype.h>

#include "asc_security_core/asc/_asc_cfg.h"

ASC_NODISCARD ASC_INLINE asc_result asc_hex_to_digit(uint8_t c, uint8_t* out)
{
  if (isdigit(c))
  {
    *out = (uint8_t)(c - '0');
  }
  else if ('a' <= c && c <= 'f')
  {
    *out = (uint8_t)(c - ('a' - 10));
  }
  else if ('A' <= c && c <= 'F')
  {
    *out = (uint8_t)(c - ('A' - 10));
  }
  else
  {
    return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
  }
  return ASC_OK;
}

ASC_NODISCARD ASC_INLINE asc_result asc_json_esc_decode(uint8_t c, uint8_t* out)
{
  switch (c)
  {
    case '\\':
    case '"':
    case '/':
    {
      *out = c;
      break;
    }
    case 'b':
    {
      *out = '\b';
      break;
    }
    case 'f':
    {
      *out = '\f';
      break;
    }
    case 'n':
    {
      *out = '\n';
      break;
    }
    case 'r':
    {
      *out = '\r';
      break;
    }
    case 't':
    {
      *out = '\t';
      break;
    }
    default:
      return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
  }
  return ASC_OK;
}

/**
 * Encodes the given character into a JSON escape sequence. The function returns an empty span if
 * the given character doesn't require to be escaped.
 */
ASC_NODISCARD asc_span _asc_json_esc_encode(uint8_t c)
{
  switch (c)
  {
    case '\\':
    {
      return ASC_SPAN_FROM_STR("\\\\");
    }
    case '"':
    {
      return ASC_SPAN_FROM_STR("\\\"");
    }
    case '\b':
    {
      return ASC_SPAN_FROM_STR("\\b");
    }
    case '\f':
    {
      return ASC_SPAN_FROM_STR("\\f");
    }
    case '\n':
    {
      return ASC_SPAN_FROM_STR("\\n");
    }
    case '\r':
    {
      return ASC_SPAN_FROM_STR("\\r");
    }
    case '\t':
    {
      return ASC_SPAN_FROM_STR("\\t");
    }
    default:
    {
      return ASC_SPAN_NULL;
    }
  }
}

/**
 * TODO: this function and JSON pointer read functions should return proper UNICODE
 *       code-point to be compatible.
 */
ASC_NODISCARD asc_result _asc_span_reader_read_json_string_char(asc_span* json_string, uint32_t* out)
{
  int32_t reader_length = asc_span_length(*json_string);
  if (reader_length == 0)
  {
    return ASC_ERROR_ITEM_NOT_FOUND;
  }

  uint8_t const result = asc_span_ptr(*json_string)[0];
  switch (result)
  {
    case '"':
    {
      return ASC_ERROR_JSON_STRING_END;
    }
    case '\\':
    {
      // moving reader fw
      *json_string = asc_span_slice(*json_string, 1, -1);
      if (asc_span_length(*json_string) == 0)
      {
        return ASC_ERROR_EOF;
      }
      uint8_t const c = asc_span_ptr(*json_string)[0];
      *json_string = asc_span_slice(*json_string, 1, -1);

      if (c == 'u')
      {
        uint32_t r = 0;
        for (size_t i = 0; i < 4; ++i)
        {
          uint8_t digit = 0;
          if (asc_span_length(*json_string) == 0)
          {
            return ASC_ERROR_EOF;
          }
          ASC_RETURN_IF_FAILED(asc_hex_to_digit(asc_span_ptr(*json_string)[0], &digit));
          r = (r << 4) + digit;
          *json_string = asc_span_slice(*json_string, 1, -1);
        }
        *out = r;
      }
      else
      {
        uint8_t r = 0;
        ASC_RETURN_IF_FAILED(asc_json_esc_decode(c, &r));
        *out = r;
      }
      return ASC_OK;
    }
    default:
    {
      if (result < 0x20)
      {
        return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
      }
      *json_string = asc_span_slice(*json_string, 1, -1);
      *out = (uint16_t)result;
      return ASC_OK;
    }
  }
}
