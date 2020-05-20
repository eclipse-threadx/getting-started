// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "asc_security_core/asc/asc_json.h"

#include "asc_security_core/asc/asc_json_string_private.h"
#include "asc_security_core/asc/asc_span_private.h"
#include "asc_security_core/asc/asc_span.h"

#include "asc_security_core/asc/asc_precondition.h"

#include <ctype.h>
#include <math.h>

#include "asc_security_core/asc/_asc_cfg.h"

enum
{
  ASC_JSON_STACK_SIZE = 63
};

typedef enum
{
  ASC_JSON_STACK_OBJECT = 0,
  ASC_JSON_STACK_ARRAY = 1,
} asc_json_stack_item;

/**
 * @brief check if input @p c is a white space. Utility function that help discarding empty spaces
 * from tokens
 *
 */
ASC_NODISCARD ASC_INLINE bool asc_json_is_white_space(uint8_t c)
{
  switch (c)
  {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      return true;
  }
  return false;
}

/**
 * @brief check if @p c is either an 'e' or an 'E'. This is a helper function to handle exponential
 * numbers like 10e10
 *
 */
ASC_NODISCARD ASC_INLINE bool asc_json_is_e(uint8_t c)
{
  switch (c)
  {
    case 'e':
    case 'E':
      return true;
  }
  return false;
}

ASC_NODISCARD ASC_INLINE bool asc_json_parser_stack_is_empty(asc_json_parser const* json_parser)
{
  return json_parser->_internal.stack == 1;
}

ASC_NODISCARD ASC_INLINE asc_json_stack_item
asc_json_parser_stack_last(asc_json_parser const* json_parser)
{
  return json_parser->_internal.stack & 1;
}

ASC_NODISCARD ASC_INLINE asc_result
asc_json_parser_push_stack(asc_json_parser* json_parser, _asc_json_stack json_stack)
{
  if (json_parser->_internal.stack >> ASC_JSON_STACK_SIZE != 0)
  {
    return ASC_ERROR_JSON_NESTING_OVERFLOW;
  }
  json_parser->_internal.stack = (json_parser->_internal.stack << 1) | json_stack;
  return ASC_OK;
}

ASC_NODISCARD ASC_INLINE asc_result asc_json_stack_pop(_asc_json_stack* json_stack)
{
  if (*json_stack <= 1)
  {
    return ASC_ERROR_JSON_INVALID_STATE;
  }
  *json_stack >>= 1;
  return ASC_OK;
}

ASC_NODISCARD ASC_INLINE asc_result asc_json_parser_pop_stack(asc_json_parser* json_parser)
{
  return asc_json_stack_pop(&json_parser->_internal.stack);
}

ASC_NODISCARD asc_result asc_json_parser_init(asc_json_parser* json_parser, asc_span json_buffer)
{
  *json_parser = (asc_json_parser){ ._internal = { .reader = json_buffer, .stack = 1 } };
  return ASC_OK;
}

static asc_result asc_span_reader_skip_json_white_space(asc_span* self)
{
  if (asc_span_length(*self) == 0)
  {
    return ASC_OK;
  }
  while (asc_json_is_white_space(asc_span_ptr(*self)[0]))
  {
    *self = asc_span_slice(*self, 1, -1);
    if (asc_span_length(*self) == 0)
    {
      return ASC_OK;
    }
  }
  return ASC_OK;
}

// 18 decimal digits. 10^18 - 1.
//                        0         1
//                        012345678901234567
#define ASC_DEC_NUMBER_MAX 999999999999999999ull

typedef struct
{
  int sign;
  uint64_t value;
  bool remainder;
  int16_t exp;
} asc_dec_number;

/* Calculate 10 ^ exp with O(log exp) by doing incremental multiplication
 If result goes beyone double limits (Overflow), infinite is returned based on standard IEEE_754
 https://en.wikipedia.org/wiki/IEEE_754.
 If Underflow, 0 is returned
*/
ASC_NODISCARD static double _ten_to_exp(int16_t exp)
{
  double result = 1;
  double incrementing_base = 10;
  int16_t abs_exp = (int16_t)(exp < 0 ? -exp : exp);

  while (abs_exp > 0)
  {
    // odd exp would update result to current incremented base
    if (abs_exp & 1)
    {
      result = exp < 0 ? result / incrementing_base : result * incrementing_base;
    }

    abs_exp /= 2;
    incrementing_base = incrementing_base * incrementing_base;
  }
  return result;
}

// double result follows IEEE_754 https://en.wikipedia.org/wiki/IEEE_754
static ASC_NODISCARD asc_result _asc_json_number_to_double(asc_dec_number const* p, double* out)
{
  *out = (double)p->value * _ten_to_exp(p->exp) * (double)p->sign;
  return ASC_OK;
}

ASC_NODISCARD static asc_result asc_span_reader_get_json_number_int(
    asc_span* self,
    asc_dec_number* p_n,
    int16_t e_offset,
    uint8_t first)
{
  uint8_t c = first;
  // read an integer part of the number
  while (true)
  {
    uint64_t d = (uint64_t)(c - '0');
    if (p_n->value <= (ASC_DEC_NUMBER_MAX - d) / 10)
    {
      p_n->value = p_n->value * 10 + d;
      p_n->exp = (int16_t)((p_n->exp + e_offset) & 0xFFFF);
    }
    else
    {
      if (d != 0)
      {
        p_n->remainder = true;
      }
      p_n->exp = (int16_t)((p_n->exp + e_offset + 1) & 0xFFFF);
    }
    *self = asc_span_slice(*self, 1, -1);
    if (asc_span_length(*self) == 0)
    {
      return ASC_OK; // end of reader is fine. Means int number is over
    }
    c = asc_span_ptr(*self)[0];
    if (!isdigit(c))
    {
      return ASC_OK;
    }
  };
}

ASC_NODISCARD static asc_result asc_span_reader_get_json_number_digit_rest(
    asc_span* self,
    double* out_value)
{
  asc_dec_number i = {
    .sign = 1,
    .value = 0,
    .remainder = false,
    .exp = 0,
  };

  // integer part
  {
    uint8_t o = asc_span_ptr(*self)[0];
    if (o == '-')
    {
      i.sign = -1;
      *self = asc_span_slice(*self, 1, -1);
      if (asc_span_length(*self) == 0)
      {
        return ASC_ERROR_EOF;
      }
      o = asc_span_ptr(*self)[0];
      if (!isdigit(o))
      {
        return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
      }
    }
    if (o != '0')
    {
      ASC_RETURN_IF_FAILED(asc_span_reader_get_json_number_int(self, &i, 0, o));
    }
    else
    {
      *self = asc_span_slice(*self, 1, -1);
    }
  }
  if (asc_span_length(*self) == 0)
  {
    ASC_RETURN_IF_FAILED(_asc_json_number_to_double(&i, out_value));
    return ASC_OK; // it's fine is int finish here (no fraction or something else)
  }

  // fraction
  if (asc_span_ptr(*self)[0] == '.')
  {
    *self = asc_span_slice(*self, 1, -1);
    if (asc_span_length(*self) == 0)
    {
      return ASC_ERROR_EOF; // uncompleted number
    }
    uint8_t o = asc_span_ptr(*self)[0];
    if (!isdigit(o))
    {
      return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
    }
    ASC_RETURN_IF_FAILED(asc_span_reader_get_json_number_int(self, &i, -1, o));
  }

  if (asc_span_length(*self) == 0)
  {
    ASC_RETURN_IF_FAILED(_asc_json_number_to_double(&i, out_value));
    return ASC_OK; // fine if number ends after a fraction
  }

  // exp
  if (asc_json_is_e(asc_span_ptr(*self)[0]))
  {
    // skip 'e' or 'E'
    *self = asc_span_slice(*self, 1, -1);
    if (asc_span_length(*self) == 0)
    {
      return ASC_ERROR_EOF; // mising expo info
    }
    uint8_t c = asc_span_ptr(*self)[0];

    // read sign, if any.
    int8_t e_sign = 1;
    switch (c)
    {
      case '-':
        e_sign = -1;
        ASC_FALLTHROUGH;
      case '+':
        *self = asc_span_slice(*self, 1, -1);
        if (asc_span_length(*self) == 0)
        {
          return ASC_ERROR_EOF; // uncompleted exp data
        }
        c = asc_span_ptr(*self)[0];
    }

    // expect at least one digit.
    if (!isdigit(c))
    {
      return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
    }

    int16_t e_int = 0;
    do
    {
      e_int = (int16_t)((e_int * 10 + (int16_t)(c - '0')) & 0xFFFF);
      *self = asc_span_slice(*self, 1, -1);
      if (asc_span_length(*self) == 0)
      {
        break; // nothing more to read
      }
      c = asc_span_ptr(*self)[0];
    } while (isdigit(c));
    i.exp = (int16_t)((i.exp + (e_int * e_sign)) & 0xFFFF);
  }

  ASC_RETURN_IF_FAILED(_asc_json_number_to_double(&i, out_value));
  return ASC_OK;
}

ASC_NODISCARD static asc_result asc_span_reader_get_json_string_rest(asc_span* self, asc_span* string)
{
  // skip '"'
  int32_t reader_initial_length = asc_span_capacity(*self);
  uint8_t* p_reader = asc_span_ptr(*self);
  while (true)
  {
    uint32_t ignore = { 0 };
    asc_result const result = _asc_span_reader_read_json_string_char(self, &ignore);
    switch (result)
    {
      case ASC_ERROR_JSON_STRING_END:
      {
        int32_t read_count = reader_initial_length - asc_span_capacity(*self);
        *string = asc_span_init(p_reader, read_count, read_count);
        *self = asc_span_slice(*self, 1, -1);
        return ASC_OK;
      }
      case ASC_ERROR_ITEM_NOT_FOUND:
      {
        return ASC_ERROR_EOF;
      }
      default:
      {
        ASC_RETURN_IF_FAILED(result);
      }
    }
  }
}

// _value_
ASC_NODISCARD static asc_result asc_json_parser_get_value(
    asc_json_parser* json_parser,
    asc_json_token* out_token)
{
  asc_span* p_reader = &json_parser->_internal.reader;

  if (asc_span_length(*p_reader) == 0)
  {
    return ASC_ERROR_EOF;
  }

  uint8_t c = asc_span_ptr(*p_reader)[0];
  if (isdigit(c))
  {
    out_token->kind = ASC_JSON_TOKEN_NUMBER;
    return asc_span_reader_get_json_number_digit_rest(p_reader, &out_token->_internal.number);
  }
  switch (c)
  {
    case 't':
      out_token->kind = ASC_JSON_TOKEN_BOOLEAN;
      out_token->_internal.boolean = true;
      return _asc_is_expected_span(p_reader, ASC_SPAN_FROM_STR("true"));
    case 'f':
      out_token->kind = ASC_JSON_TOKEN_BOOLEAN;
      out_token->_internal.boolean = false;
      return _asc_is_expected_span(p_reader, ASC_SPAN_FROM_STR("false"));
    case 'n':
      out_token->kind = ASC_JSON_TOKEN_NULL;
      return _asc_is_expected_span(p_reader, ASC_SPAN_FROM_STR("null"));
    case '"':
      out_token->kind = ASC_JSON_TOKEN_STRING;
      *p_reader = asc_span_slice(*p_reader, 1, -1);
      return asc_span_reader_get_json_string_rest(p_reader, &out_token->_internal.string);
    case '-':
      out_token->kind = ASC_JSON_TOKEN_NUMBER;
      return asc_span_reader_get_json_number_digit_rest(p_reader, &out_token->_internal.number);
    case '{':
      out_token->kind = ASC_JSON_TOKEN_OBJECT_START;
      *p_reader = asc_span_slice(*p_reader, 1, -1);
      return asc_json_parser_push_stack(json_parser, ASC_JSON_STACK_OBJECT);
    case '[':
      out_token->kind = ASC_JSON_TOKEN_ARRAY_START;
      *p_reader = asc_span_slice(*p_reader, 1, -1);
      return asc_json_parser_push_stack(json_parser, ASC_JSON_STACK_ARRAY);
  }
  return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
}

ASC_NODISCARD static asc_result asc_json_parser_get_value_space(
    asc_json_parser* p_state,
    asc_json_token* out_token)
{
  ASC_RETURN_IF_FAILED(asc_json_parser_get_value(p_state, out_token));
  if (asc_span_length(p_state->_internal.reader) > 0)
  {
    ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(&p_state->_internal.reader));
  }
  return ASC_OK;
}

ASC_NODISCARD asc_result
asc_json_parser_parse_token(asc_json_parser* json_parser, asc_json_token* out_token)
{
////  ASC_PRECONDITION_NOT_NULL(json_parser);
////  ASC_PRECONDITION_NOT_NULL(out_token);

  if (!asc_json_parser_stack_is_empty(json_parser))
  {
    return ASC_ERROR_JSON_INVALID_STATE;
  }
  asc_span* p_reader = &json_parser->_internal.reader;
  ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(p_reader));
  ASC_RETURN_IF_FAILED(asc_json_parser_get_value_space(json_parser, out_token));
  bool const is_empty = asc_span_length(*p_reader) == 0; // everything was read
  switch (out_token->kind)
  {
    case ASC_JSON_TOKEN_ARRAY_START:
    case ASC_JSON_TOKEN_OBJECT_START:
      return is_empty ? ASC_ERROR_EOF : ASC_OK;
    default:
      break;
  }
  return is_empty ? ASC_OK : ASC_ERROR_PARSER_UNEXPECTED_CHAR;
}

ASC_NODISCARD ASC_INLINE uint8_t asc_json_stack_item_to_close(asc_json_stack_item item)
{
  return item == ASC_JSON_STACK_OBJECT ? '}' : ']';
}

ASC_NODISCARD static asc_result asc_json_parser_read_comma_or_close(asc_json_parser* json_parser)
{
  asc_span* p_reader = &json_parser->_internal.reader;
  uint8_t const c = asc_span_ptr(*p_reader)[0];
  if (c == ',')
  {
    // skip ',' and read all whitespaces.
    *p_reader = asc_span_slice(*p_reader, 1, -1);
    ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(p_reader));
    return ASC_OK;
  }
  uint8_t const close = asc_json_stack_item_to_close(asc_json_parser_stack_last(json_parser));
  if (c != close)
  {
    return ASC_ERROR_PARSER_UNEXPECTED_CHAR;
  }
  return ASC_OK;
}

ASC_NODISCARD static asc_result asc_json_parser_check_item_begin(
    asc_json_parser* json_parser,
    asc_json_stack_item stack_item)
{
  if (asc_json_parser_stack_is_empty(json_parser)
      || asc_json_parser_stack_last(json_parser) != stack_item)
  {
    return ASC_ERROR_JSON_INVALID_STATE;
  }
  asc_span* p_reader = &json_parser->_internal.reader;
  if (asc_span_length(*p_reader) == 0)
  {
    return ASC_ERROR_EOF;
  }
  uint8_t const c = asc_span_ptr(*p_reader)[0];
  if (c != asc_json_stack_item_to_close(stack_item))
  {
    return ASC_OK;
  }
  // c == close
  ASC_RETURN_IF_FAILED(asc_json_parser_pop_stack(json_parser));
  *p_reader = asc_span_slice(*p_reader, 1, -1);
  ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(p_reader));
  if (!asc_json_parser_stack_is_empty(json_parser))
  {
    ASC_RETURN_IF_FAILED(asc_json_parser_read_comma_or_close(json_parser));
  }
  return ASC_ERROR_ITEM_NOT_FOUND;
}

ASC_NODISCARD static asc_result asc_json_parser_check_item_end(
    asc_json_parser* json_parser,
    asc_json_token value)
{
  switch (value.kind)
  {
    case ASC_JSON_TOKEN_OBJECT_START:
    case ASC_JSON_TOKEN_ARRAY_START:
      return ASC_OK;
    default:
      break;
  }
  return asc_json_parser_read_comma_or_close(json_parser);
}

ASC_NODISCARD asc_result asc_json_parser_parse_token_member(
    asc_json_parser* json_parser,
    asc_json_token_member* out_token_member)
{
////  ASC_PRECONDITION_NOT_NULL(json_parser);
////  ASC_PRECONDITION_NOT_NULL(out_token_member);

  asc_span* p_reader = &json_parser->_internal.reader;
  ASC_RETURN_IF_FAILED(asc_json_parser_check_item_begin(json_parser, ASC_JSON_STACK_OBJECT));
  ASC_RETURN_IF_FAILED(_asc_is_expected_span(p_reader, ASC_SPAN_FROM_STR("\"")));
  ASC_RETURN_IF_FAILED(asc_span_reader_get_json_string_rest(p_reader, &out_token_member->name));
  ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(p_reader));
  ASC_RETURN_IF_FAILED(_asc_is_expected_span(p_reader, ASC_SPAN_FROM_STR(":")));
  ASC_RETURN_IF_FAILED(asc_span_reader_skip_json_white_space(p_reader));
  ASC_RETURN_IF_FAILED(asc_json_parser_get_value_space(json_parser, &out_token_member->token));
  return asc_json_parser_check_item_end(json_parser, out_token_member->token);
}

ASC_NODISCARD asc_result
asc_json_parser_parse_array_item(asc_json_parser* json_parser, asc_json_token* out_token)
{
////  ASC_PRECONDITION_NOT_NULL(json_parser);
////  ASC_PRECONDITION_NOT_NULL(out_token);

  ASC_RETURN_IF_FAILED(asc_json_parser_check_item_begin(json_parser, ASC_JSON_STACK_ARRAY));
  ASC_RETURN_IF_FAILED(asc_json_parser_get_value_space(json_parser, out_token));
  return asc_json_parser_check_item_end(json_parser, *out_token);
}

ASC_NODISCARD asc_result asc_json_parser_done(asc_json_parser* json_parser)
{
////  ASC_PRECONDITION_NOT_NULL(json_parser);

  if (asc_span_length(json_parser->_internal.reader) > 0
      || !asc_json_parser_stack_is_empty(json_parser))
  {
    return ASC_ERROR_JSON_INVALID_STATE;
  }
  return ASC_OK;
}

ASC_NODISCARD asc_result
asc_json_parser_skip_children(asc_json_parser* json_parser, asc_json_token token)
{
////  ASC_PRECONDITION_NOT_NULL(json_parser);

  switch (token.kind)
  {
    case ASC_JSON_TOKEN_OBJECT_START:
    case ASC_JSON_TOKEN_ARRAY_START:
    {
      break;
    }
    default:
    {
      return ASC_OK;
    }
  }

  _asc_json_stack target_stack = json_parser->_internal.stack;
  ASC_RETURN_IF_FAILED(asc_json_stack_pop(&target_stack));

  while (true)
  {
    // asc_json_parser_get_stack
    switch (asc_json_parser_stack_last(json_parser))
    {
      case ASC_JSON_STACK_OBJECT:
      {
        asc_json_token_member member = { 0 };
        asc_result const result = asc_json_parser_parse_token_member(json_parser, &member);
        if (result != ASC_ERROR_ITEM_NOT_FOUND)
        {
          ASC_RETURN_IF_FAILED(result);
        }
        break;
      }
      default:
      {
        asc_json_token element = { 0 };
        asc_result result = asc_json_parser_parse_array_item(json_parser, &element);
        if (result != ASC_ERROR_ITEM_NOT_FOUND)
        {
          ASC_RETURN_IF_FAILED(result);
        }
        break;
      }
    }
    if (json_parser->_internal.stack == target_stack)
    {
      return ASC_OK;
    }
  }
}
