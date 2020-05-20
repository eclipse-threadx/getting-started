// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file asc_json.h
 *
 * @brief This header defines the types and functions your application uses
 *        to build or parse JSON objects.
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _asc_JSON_H
#define _asc_JSON_H

#include "asc_security_core/asc/asc_result.h"
#include "asc_security_core/asc/asc_span.h"

#include <stdbool.h>
#include <stdint.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

/*
 * @brief asc_json_token_kind is an enum defining symbols for the various kinds of JSON tokens.
 */
typedef enum
{
  ASC_JSON_TOKEN_NULL,
  ASC_JSON_TOKEN_BOOLEAN,
  ASC_JSON_TOKEN_NUMBER,
  ASC_JSON_TOKEN_STRING,
  ASC_JSON_TOKEN_OBJECT,
  ASC_JSON_TOKEN_OBJECT_START,
  ASC_JSON_TOKEN_OBJECT_END,
  ASC_JSON_TOKEN_ARRAY_START,
  ASC_JSON_TOKEN_ARRAY_END,
  // ASC_JSON_TOKEN_SPAN represents a token consisting of nested JSON; the JSON parser never returns
  // a token of this type.
  ASC_JSON_TOKEN_SPAN,
} asc_json_token_kind;

/*
 * @brief An asc_json_token instance represents a JSON token. The kind field indicates the kind of
 * token and based on the kind, you access the corresponding field.
 */
typedef struct
{
  asc_json_token_kind kind;
  union {
    bool boolean;
    double number;
    asc_span string;
    asc_span span;
  } _internal;
} asc_json_token;

/*
 * @brief asc_json_token_null Returns the "null" JSON token.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_null()
{
  return (asc_json_token){ .kind = ASC_JSON_TOKEN_NULL, ._internal = { 0 } };
}

/*
 * @brief asc_json_token_boolean Returns a boolean JSON token representing either "true" or "false".
 *
 * @param value A boolean indicating how the asc_json_token should be initialized.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_boolean(bool value)
{
  return (asc_json_token){
    .kind = ASC_JSON_TOKEN_BOOLEAN,
    ._internal.boolean = value,
  };
}

/*
 * @brief asc_json_token_number returns a asc_json_token containing a number.
 *
 * @param value A double indicating how the asc_json_token should be initialized.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_number(double value)
{
  return (asc_json_token){
    .kind = ASC_JSON_TOKEN_NUMBER,
    ._internal.number = value,
  };
}

/*
 * @brief asc_json_token_string returns a asc_json_token containing a string.
 *
 * @param value A span over a string indicating how the asc_json_token should be initialized.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_string(asc_span value)
{
  return (asc_json_token){
    .kind = ASC_JSON_TOKEN_STRING,
    ._internal.string = value,
  };
}

/*
 * @brief asc_json_token_string returns a asc_json_token containing an object.
 *
 * @param value A span over an object indicating how the asc_json_token should be initialized.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_object(asc_span value)
{
  return (asc_json_token){
    .kind = ASC_JSON_TOKEN_OBJECT,
    ._internal.span = value,
  };
}

/*
 * @brief asc_json_token_object_start returns a asc_json_token representing the start of an object.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_object_start()
{
  return (asc_json_token){ .kind = ASC_JSON_TOKEN_OBJECT_START, ._internal = { 0 } };
}

/*
 * @brief asc_json_token_object_end returns a asc_json_token representing the end of an object.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_object_end()
{
  return (asc_json_token){ .kind = ASC_JSON_TOKEN_OBJECT_END, ._internal = { 0 } };
}

/*
 * @brief asc_json_token_array_start returns a asc_json_token representing the start of an array.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_array_start()
{
  return (asc_json_token){ .kind = ASC_JSON_TOKEN_ARRAY_START, ._internal = { 0 } };
}

/*
 * @brief asc_json_token_array_end returns a asc_json_token representing the end of an array.
 */
ASC_NODISCARD ASC_INLINE asc_json_token asc_json_token_array_end()
{
  return (asc_json_token){ .kind = ASC_JSON_TOKEN_ARRAY_END, ._internal = { 0 } };
}

/*
 * @brief asc_json_token_get_boolean returns the JSON token's boolean.
 *
 * @param token A pointer to an asc_json_token instance.
 * @param out_value A pointer to a variable to receive the value.
 * @return ASC_OK if the boolean is returned.<br>
 * ASC_ERROR_ITEM_NOT_FOUND if the kind is not ASC_JSON_TOKEN_BOOLEAN.
 */
ASC_NODISCARD asc_result asc_json_token_get_boolean(asc_json_token const* token, bool* out_value);

/*
 * @brief asc_json_token_get_number returns the JSON token's number.
 *
 * @param token A pointer to an asc_json_token instance.
 * @param out_value A pointer to a variable to receive the value.
 * @return ASC_OK if the number is returned.<br>
 * ASC_ERROR_ITEM_NOT_FOUND if the kind != ASC_JSON_TOKEN_NUMBER.
 */
ASC_NODISCARD asc_result asc_json_token_get_number(asc_json_token const* token, double* out_value);

/*
 * @brief asc_json_token_get_string returns the JSON token's string via an asc_span.
 *
 * @param token A pointer to an asc_json_token instance.
 * @param out_value A pointer to a variable to receive the value.
 * @return ASC_OK if the string is returned.<br>
 * ASC_ERROR_ITEM_NOT_FOUND if the kind != ASC_JSON_TOKEN_STRING.
 */
ASC_NODISCARD asc_result asc_json_token_get_string(asc_json_token const* token, asc_span* out_value);

/************************************ JSON BUILDER ******************/

/*
 * @brief An asc_json_builder allows you to build a JSON object into a buffer.
 */
typedef struct
{
  struct
  {
    asc_span json;
    bool need_comma;
  } _internal;
} asc_json_builder;

/*
 * @brief asc_json_builder_init initializes an asc_json_builder which writes JSON into a buffer.
 *
 * @param json_builder A pointer to an asc_json_builder instance to initialize.
 * @param json_buffer An asc_span over the buffer where th JSON object is to be written.
 * @return ASC_OK if the asc_json_builder is initialized correctly.
 */
ASC_NODISCARD ASC_INLINE asc_result
asc_json_builder_init(asc_json_builder* json_builder, asc_span json_buffer)
{
  *json_builder = (asc_json_builder){ ._internal = { .json = json_buffer, .need_comma = false } };
  return ASC_OK;
}

/*
 * @brief asc_json_builder_span_get returns the asc_span containing the final JSON object.
 *
 * @param json_builder A pointer to an asc_json_builder instance wrapping the JSON buffer.
 * @return an asc_span containing the final JSON object.
 */
ASC_NODISCARD ASC_INLINE asc_span asc_json_builder_span_get(asc_json_builder const* json_builder)
{
  return json_builder->_internal.json;
}

/*
 * @brief asc_json_builder_append_token appends an asc_json_token to the JSON buffer.
 *
 * @param json_builder A pointer to an asc_json_builder instance containing the buffer to append the
 * token to.
 * @param token A pointer to the asc_json_token to append.
 * @return ASC_OK if the token was appended successfully.<br>
 * ASC_ERROR_BUFFER_OVERFLOW if the buffer is too small.
 */
ASC_NODISCARD asc_result
asc_json_builder_append_token(asc_json_builder* json_builder, asc_json_token token);

/*
 * @brief asc_json_builder_append_object appends a JSON to the JSON buffer.
 *
 * @param json_builder A pointer to an asc_json_builder instance containing the buffer to append the
 * object to.
 * @param name A span containing the JSON object's name.
 * @param token A pointer to the asc_json_token to append as the object's value.
 * @return ASC_OK if the token was appended successfully.<br>
 * ASC_ERROR_BUFFER_OVERFLOW if the buffer is too small.
 */
ASC_NODISCARD asc_result
asc_json_builder_append_object(asc_json_builder* json_builder, asc_span name, asc_json_token token);

/*
 * @brief asc_json_builder_append_array_item appends an array item to the JSON buffer.
 *
 * @param json_builder A pointer to an asc_json_builder instance containing the buffer to append the
 * array item to.
 * @param token A pointer to the asc_json_token representing the array item.
 * @return ASC_OK if the token was appended successfully.<br>
 * ASC_ERROR_BUFFER_OVERFLOW if the buffer is too small.
 */
ASC_NODISCARD asc_result
asc_json_builder_append_array_item(asc_json_builder* json_builder, asc_json_token token);

/************************************ JSON PARSER ******************/

typedef uint64_t _asc_json_stack;

/*
 * @brief An asc_json_parser returns the JSON tokens contained within a JSON buffer.
 */
typedef struct
{
  struct
  {
    asc_span reader;
    _asc_json_stack stack;
  } _internal;
} asc_json_parser;

/*
 * @brief An asc_json_token_member represents a JSON element's name and value.
 */
typedef struct
{
  asc_span name;
  asc_json_token token;
} asc_json_token_member;

/*
 * @brief asc_json_parser_init initializes an asc_json_parser to parse the JSON payload contained within the passed in buffer.
 * JSON buffer.
 *
 * @param json_parser A pointer to an asc_json_parser instance to initialize.
 * @param json_buffer A pointer to a buffer containing the JSON document to parse.
 * @return ASC_OK if the token was appended successfully.<br>
 */
ASC_NODISCARD asc_result asc_json_parser_init(asc_json_parser* json_parser, asc_span json_buffer);

/*
 * @brief asc_json_parser_parse_token returns the next token in the JSON document.
 *
 * @param json_parser A pointer to an asc_json_parser instance containing the JSON to parse.
 * @param out_token A pointer to an asc_json_token containing the next parsed JSON token.
 * @return ASC_OK if the token was parsed successfully.<br>
 *         ASC_ERROR_EOF when the end of the JSON document is reached.<br>
 *         ASC_ERROR_PARSER_UNEXPECTED_CHAR when an invalid character is detected.<br>
 *         ASC_ERROR_ITEM_NOT_FOUND when no more items are found.
 */
ASC_NODISCARD asc_result
asc_json_parser_parse_token(asc_json_parser* json_parser, asc_json_token* out_token);

/*
 * @brief asc_json_parser_parse_token_member returns the next token member in the JSON document.
 *
 * @param json_parser A pointer to an asc_json_parser instance containing the JSON to parse.
 * @param out_token_member A pointer to an asc_json_token_member containing the next parsed JSON
 * token member.
 * @return ASC_OK if the token was parsed successfully.<br>
 *         ASC_ERROR_EOF when the end of the JSON document is reached.<br>
 *         ASC_ERROR_PARSER_UNEXPECTED_CHAR when an invalid character is detected.<br>
 *         ASC_ERROR_ITEM_NOT_FOUND when no more items are found.
 */
ASC_NODISCARD asc_result asc_json_parser_parse_token_member(
    asc_json_parser* json_parser,
    asc_json_token_member* out_token_member);

/*
 * @brief asc_json_parser_parse_array_item returns the next array item in the JSON document.
 *
 * @param json_parser A pointer to an asc_json_parser instance containing the JSON to parse.
 * @param out_token A pointer to an asc_json_token containing the next parsed JSON array item.
 * @return ASC_OK if the token was parsed successfully.<br>
 *         ASC_ERROR_EOF when the end of the JSON document is reached.<br>
 *         ASC_ERROR_PARSER_UNEXPECTED_CHAR when an invalid character is detected.<br>
 *         ASC_ERROR_ITEM_NOT_FOUND when no more items are found.
 */
ASC_NODISCARD asc_result
asc_json_parser_parse_array_item(asc_json_parser* json_parser, asc_json_token* out_token);

/*
 * @brief asc_json_parser_skip_children parses and skips over any nested JSON elements.
 *
 * @param json_parser A pointer to an asc_json_parser instance containing the JSON to parse.
 * @param out_token A pointer to an asc_json_token containing the next parsed JSON token.
 * @return ASC_OK if the token was parsed successfully.<br>
 *         ASC_ERROR_EOF when the end of the JSON document is reached.<br>
 *         ASC_ERROR_PARSER_UNEXPECTED_CHAR when an invalid character is detected.<br>
 *         ASC_ERROR_ITEM_NOT_FOUND when no more items are found.
 */
ASC_NODISCARD asc_result
asc_json_parser_skip_children(asc_json_parser* json_parser, asc_json_token token);

/*
 * @brief  asc_json_parser_done validates that there is nothing else to parse in the JSON document.
 *
 * @param json_parser A pointer to an asc_json_parser instance containing the JSON that was parsed.
 * @return ASC_OK if the token was parsed completely.<br>
 *         ASC_ERROR_JSON_INVALID_STATE if not all of the JSON document was parsed.
 */
ASC_NODISCARD asc_result asc_json_parser_done(asc_json_parser* json_parser);

/************************************ JSON POINTER ******************/

/*
 * @brief asc_json_parse_by_pointer parses a JSON document and returns the asc_json_token identified
 * by a JSON pointer.
 *
 * @param json_buffer An asc_span over a buffer containing the JSON document to parse.
 * @param json_pointer An asc_span over a string containing JSON-pointer syntax (see
 * https://tools.ietf.org/html/rfc6901).
 * @param out_token A pointer to an asc_json_token that receives the JSON token.
 * @return ASC_OK if the desired token was found in the JSON document.
 *         ASC_ERROR_EOF when the end of the JSON document is reached.<br>
 *         ASC_ERROR_PARSER_UNEXPECTED_CHAR when an invalid character is detected.<br>
 *         ASC_ERROR_ITEM_NOT_FOUND when no more items are found.
 */
ASC_NODISCARD asc_result
asc_json_parse_by_pointer(asc_span json_buffer, asc_span json_pointer, asc_json_token* out_token);

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_JSON_H
