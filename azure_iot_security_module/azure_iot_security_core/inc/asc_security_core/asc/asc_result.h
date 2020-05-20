// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file asc_result.h
 *
 * @brief asc_result and facilities definition
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _asc_RESULT_H
#define _asc_RESULT_H

#include <stdbool.h>
#include <stdint.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

enum
{
  _asc_FACILITY_CORE = 0x1,
  _asc_FACILITY_PLATFORM = 0x2,
  _asc_FACILITY_JSON = 0x3,
  _asc_FACILITY_HTTP = 0x4,
  _asc_FACILITY_MQTT = 0x5,
  _asc_FACILITY_IOT = 0x6,
};

enum
{
  _asc_ERROR_FLAG = (int32_t)0x80000000,
};

#define _asc_RESULT_MAKE_ERROR(facility, code) \
  ((int32_t)(_asc_ERROR_FLAG | ((int32_t)(facility) << 16) | (int32_t)(code)))

#define _asc_RESULT_MAKE_SUCCESS(facility, code) \
  ((int32_t)(((int32_t)(facility) << 16) | (int32_t)(code)))

#define ASC_RETURN_IF_FAILED(exp) \
  do \
  { \
    asc_result const _result = (exp); \
    if (asc_failed(_result)) \
    { \
      return _result; \
    } \
  } while (0)

#define ASC_RETURN_IF_NOT_ENOUGH_CAPACITY(span, required_space) \
  do \
  { \
    if ((asc_span_capacity(span) - asc_span_length(span)) < required_space) \
    { \
      return ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY; \
    } \
  } while (0)

/**
 * The type represents error conditions.
 * Bits:
 * - 31 Severity (0 - success, 1 - failure).
 * - if failure then
 *   - 16..30 Facility.
 *   -  0..15 Code.
 * - otherwise
 *   -  0..30 Value
 */
typedef enum
{
  // Core: Success results
  ASC_OK = _asc_RESULT_MAKE_SUCCESS(_asc_FACILITY_CORE, 0), ///< Success.
  ASC_CONTINUE = _asc_RESULT_MAKE_SUCCESS(_asc_FACILITY_CORE, 1),

  // Core: Error results
  ASC_ERROR_CANCELED = _asc_RESULT_MAKE_ERROR(
      _asc_FACILITY_CORE,
      0), ///< A context was canceled, and a function had to return before result was ready.

  ASC_ERROR_ARG = _asc_RESULT_MAKE_ERROR(
      _asc_FACILITY_CORE,
      1), ///< Input argument does not comply with the requested range of values.

  ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY = _asc_RESULT_MAKE_ERROR(
      _asc_FACILITY_CORE,
      2), ///< There is not enough capacity in the span provided.

  ASC_ERROR_NOT_IMPLEMENTED
  = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_CORE, 3), ///< Requested functionality is not implemented.

  ASC_ERROR_ITEM_NOT_FOUND
  = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_CORE, 4), ///< Requested item was not found.

  ASC_ERROR_PARSER_UNEXPECTED_CHAR
  = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_CORE, 5), ///< Input can't be successfully parsed.

  ASC_ERROR_EOF = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_CORE, 6), ///< Unexpected end of the input data.

  // Platform
  ASC_ERROR_MUTEX = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_PLATFORM, 1), ///< Mutex operation error.

  ASC_ERROR_OUT_OF_MEMORY = _asc_RESULT_MAKE_ERROR(
      _asc_FACILITY_PLATFORM,
      2), ///< Dynamic memory allocation request was not successful.

  ASC_ERROR_HTTP_PLATFORM = _asc_RESULT_MAKE_ERROR(
      _asc_FACILITY_PLATFORM,
      3), ///< Generic error in the HTTP Client layer implementation.

  // JSON error codes
  ASC_ERROR_JSON_INVALID_STATE = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_JSON, 1),
  ASC_ERROR_JSON_NESTING_OVERFLOW = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_JSON, 2),
  ASC_ERROR_JSON_STRING_END = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_JSON, 3),
  ASC_ERROR_JSON_POINTER_TOKEN_END = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_JSON, 4),

  // HTTP error codes
  ASC_ERROR_HTTP_INVALID_STATE = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 1),
  ASC_ERROR_HTTP_PIPELINE_INVALID_POLICY = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 2),
  ASC_ERROR_HTTP_INVALID_METHOD_VERB = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 3),

  ASC_ERROR_HTTP_AUTHENTICATION_FAILED
  = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 4), ///< Authentication failed.

  ASC_ERROR_HTTP_RESPONSE_OVERFLOW = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 5),
  ASC_ERROR_HTTP_RESPONSE_COULDNT_RESOLVE_HOST = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_HTTP, 6),

  //IoT error codes
  ASC_ERROR_IOT_TOPIC_NO_MATCH = _asc_RESULT_MAKE_ERROR(_asc_FACILITY_IOT, 1),
} asc_result;

/// Checks wheteher the \a result provided indicates a failure.
///
/// @param result Result value to check for failure.
///
/// @retval true \a result indicates a failure.
/// @retval false \a result is successful.
ASC_NODISCARD ASC_INLINE bool asc_failed(asc_result result)
{
  return ((int32_t)result & (int32_t)_asc_ERROR_FLAG) != 0;
}

/// Checks wheteher the \a result provided indicates a success.
///
/// @param result Result value to check for success.
///
/// @retval true \a result indicates success.
/// @retval false \a result is a failure.
ASC_NODISCARD ASC_INLINE bool asc_succeeded(asc_result result) { return !asc_failed(result); }

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_RESULT_H
