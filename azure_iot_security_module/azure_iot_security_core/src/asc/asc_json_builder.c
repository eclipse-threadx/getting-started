// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "asc_security_core/asc/asc_hex_private.h"
#include "asc_security_core/asc/asc_json_string_private.h"
#include "asc_security_core/asc/asc_span_private.h"
#include "asc_security_core/asc/asc_json.h"

#include "asc_security_core/asc/_asc_cfg.h"

ASC_NODISCARD static asc_result asc_json_builder_append_str(asc_json_builder* self, asc_span value)
{
////  ASC_PRECONDITION_NOT_NULL(self);

  asc_span* json = &self->_internal.json;

  int32_t required_length = asc_span_length(value) + 2;

  ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, required_length);

  *json = asc_span_append_uint8(*json, '"');
  *json = asc_span_append(*json, value);
  *json = asc_span_append_uint8(*json, '"');
  return ASC_OK;
}

static ASC_NODISCARD asc_result _asc_json_builder_write_span(asc_json_builder* self, asc_span value)
{
////  ASC_PRECONDITION_NOT_NULL(self);

  asc_span* json = &self->_internal.json;

  ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, 1);
  *json = asc_span_append_uint8(*json, '"');

  for (int32_t i = 0; i < asc_span_length(value); ++i)
  {
    uint8_t const c = asc_span_ptr(value)[i];

    // check if the character has to be escaped.
    {
      asc_span const esc = _asc_json_esc_encode(c);
      int32_t escaped_length = asc_span_length(esc);
      if (escaped_length)
      {
        ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, escaped_length);
        *json = asc_span_append(*json, esc);
        continue;
      }
    }
    // check if the character has to be escaped as a UNICODE escape sequence.
    if (c < 0x20)
    {
      uint8_t array[6] = {
        '\\',
        'u',
        '0',
        '0',
        _asc_number_to_upper_hex((uint8_t)(c / 16)),
        _asc_number_to_upper_hex((uint8_t)(c % 16)),
      };

      ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, (int32_t)sizeof(array));
      *json = asc_span_append(*json, ASC_SPAN_FROM_INITIALIZED_BUFFER(array));
      continue;
    }

    ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, 1);
    *json = asc_span_append_uint8(*json, asc_span_ptr(value)[i]);
  }
  ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, 1);
  *json = asc_span_append_uint8(*json, '"');

  return ASC_OK;
}

ASC_NODISCARD asc_result
asc_json_builder_append_token(asc_json_builder* json_builder, asc_json_token token)
{
////  ASC_PRECONDITION_NOT_NULL(json_builder);
  asc_span* json = &json_builder->_internal.json;

  ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, 1);

  switch (token.kind)
  {
    case ASC_JSON_TOKEN_NULL:
    {
      ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, 4);
      json_builder->_internal.need_comma = true;
      *json = asc_span_append(*json, ASC_SPAN_FROM_STR("null"));
      break;
    }
    case ASC_JSON_TOKEN_BOOLEAN:
    {
      asc_span boolean_literal_string
          = token._internal.boolean ? ASC_SPAN_FROM_STR("true") : ASC_SPAN_FROM_STR("false");
      ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, asc_span_length(boolean_literal_string));
      json_builder->_internal.need_comma = true;
      *json = asc_span_append(*json, boolean_literal_string);
      break;
    }
    case ASC_JSON_TOKEN_NUMBER:
    {
      json_builder->_internal.need_comma = true;
      return asc_span_append_dtoa(*json, token._internal.number, json);
    }
    case ASC_JSON_TOKEN_STRING:
    {
      json_builder->_internal.need_comma = true;
      return asc_json_builder_append_str(json_builder, token._internal.string);
    }
    case ASC_JSON_TOKEN_OBJECT:
    {
      ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(*json, asc_span_length(token._internal.span));
      json_builder->_internal.need_comma = true;
      *json = asc_span_append(*json, token._internal.span);
      break;
    }
    case ASC_JSON_TOKEN_OBJECT_START:
    {
      json_builder->_internal.need_comma = false;
      *json = asc_span_append_uint8(*json, '{');
      break;
    }
    case ASC_JSON_TOKEN_OBJECT_END:
    {
      json_builder->_internal.need_comma = true;
      *json = asc_span_append_uint8(*json, '}');
      break;
    }
    case ASC_JSON_TOKEN_ARRAY_START:
    {
      json_builder->_internal.need_comma = false;
      *json = asc_span_append_uint8(*json, '[');
      break;
    }
    case ASC_JSON_TOKEN_ARRAY_END:
    {
      json_builder->_internal.need_comma = true;
      *json = asc_span_append_uint8(*json, ']');
      break;
    }
    case ASC_JSON_TOKEN_SPAN:
    {
      json_builder->_internal.need_comma = true;
      return _asc_json_builder_write_span(json_builder, token._internal.span);
    }
    default:
    {
      return ASC_ERROR_ARG;
    }
  }

  return ASC_OK;
}

ASC_NODISCARD static asc_result asc_json_builder_write_comma(asc_json_builder* self)
{
////  ASC_PRECONDITION_NOT_NULL(self);

  if (self->_internal.need_comma)
  {
    ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(self->_internal.json, 1);
    self->_internal.json = asc_span_append_uint8(self->_internal.json, ',');
  }
  return ASC_OK;
}

ASC_NODISCARD asc_result
asc_json_builder_append_object(asc_json_builder* json_builder, asc_span name, asc_json_token token)
{
////  ASC_PRECONDITION_NOT_NULL(json_builder);

  ASC_RETURN_IF_FAILED(asc_json_builder_write_comma(json_builder));
  ASC_RETURN_IF_FAILED(asc_json_builder_append_str(json_builder, name));

  ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(json_builder->_internal.json, 1);
  json_builder->_internal.json = asc_span_append_uint8(json_builder->_internal.json, ':');

  ASC_RETURN_IF_FAILED(asc_json_builder_append_token(json_builder, token));
  return ASC_OK;
}

ASC_NODISCARD asc_result
asc_json_builder_append_array_item(asc_json_builder* json_builder, asc_json_token token)
{
////  ASC_PRECONDITION_NOT_NULL(json_builder);

  ASC_RETURN_IF_FAILED(asc_json_builder_write_comma(json_builder));
  ASC_RETURN_IF_FAILED(asc_json_builder_append_token(json_builder, token));
  return ASC_OK;
}
