// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file asc_span.h
 *
 * @brief An asc_span represents a contiguous byte buffer and is used for string manipulations,
 * HTTP requests/responses, building/parsing JSON payloads, and more.
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _asc_SPAN_H
#define _asc_SPAN_H

#include "asc_security_core/asc/asc_result.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

/**
 * asc_span is a "view" over a byte buffer. It contains a pointer to the
 * start of the byte buffer, the buffer's capacity; and the length (number of
 * bytes) in use from the start of the buffer.
 */
typedef struct
{
  struct
  {
    uint8_t* ptr;
    int32_t length; ///< length must be >= 0
    int32_t capacity; ///< capacity must be >= 0
  } _internal;
} asc_span;

/********************************  SPAN GETTERS */

/**
 * @brief asc_span_ptr returns the span byte buffer's starting memory address
 *
 */
ASC_NODISCARD ASC_INLINE uint8_t* asc_span_ptr(asc_span span) { return span._internal.ptr; }

/**
 * @brief asc_span_length Returns the number of bytes within the span.
 *
 */
ASC_NODISCARD ASC_INLINE int32_t asc_span_length(asc_span span) { return span._internal.length; }

/**
 * @brief asc_span_capacity Returns the maximum number of bytes.
 * Capacity will always be greater than or equal to length.
 *
 */
ASC_NODISCARD ASC_INLINE int32_t asc_span_capacity(asc_span span) { return span._internal.capacity; }

/********************************  CONSTRUCTORS */

/**
 * @brief Creates an empty span literal
 * ptr is NULL
 * length is initialized to 0
 * capacity is initialized to 0
 *
 */
#define ASC_SPAN_LITERAL_NULL \
  { \
    ._internal = {.ptr = NULL, .length = 0, .capacity = 0 } \
  }

/**
 * @brief The ASC_SPAN_NULL macro returns an empty asc_span \see ASC_SPAN_LITERAL_NULL.
 *
 */
#define ASC_SPAN_NULL (asc_span) ASC_SPAN_LITERAL_NULL

// Returns the size (in bytes) of a literal string
// Note: Concatenating "" to S produces a compiler error if S is not a literal string
//       The stored string's length does not include the \0 terminator.
#define _asc_STRING_LITERAL_LEN(S) (sizeof(S "") - 1)

/**
 * @brief The ASC_SPAN_LITERAL_FROM_STR macro returns a literal asc_span over a literal string.
 * An empty ("") literal string results in a span with length/capacity set to 0.
 * The length and capacity of the span is equal to the length of the string.
 * For example:
 *
 * `static const asc_span hw = ASC_SPAN_LITERAL_FROM_STR("Hello world");`
 */
#define ASC_SPAN_LITERAL_FROM_STR(STRING_LITERAL) \
  { \
    ._internal = { \
      .ptr = (uint8_t*)STRING_LITERAL, \
      .length = _asc_STRING_LITERAL_LEN(STRING_LITERAL), \
      .capacity = _asc_STRING_LITERAL_LEN(STRING_LITERAL), \
    }, \
  }

/**
 * @brief The ASC_SPAN_FROM_STR macro returns an asc_span expression over a literal string. For
 * example: `some_function(ASC_SPAN_FROM_STR("Hello world"));` where `void some_function(const
 * asc_span span);`
 */
#define ASC_SPAN_FROM_STR(STRING_LITERAL) (asc_span) ASC_SPAN_LITERAL_FROM_STR(STRING_LITERAL)

/**
 * @brief asc_span_init returns a span over a byte buffer.
 *
 * @param[in] ptr The memory address of the 1st byte in the byte buffer
 * @param[in] length The number of bytes initialized in the byte buffer
 * @param[in] capacity The number of total bytes in the byte buffer
 * @return asc_span The "view" over the byte buffer.
 */
ASC_NODISCARD asc_span asc_span_init(uint8_t* ptr, int32_t length, int32_t capacity);

/**
 * @brief asc_span_from_str returns an asc_span from a 0-terminated array of bytes (chars)
 *
 * @param[in] str The pointer to the 0-terminated array of bytes (chars)
 * @return asc_span An asc_span over the byte buffer; length & capacity are set to the string's
 * length not including the \0 terminator.
 */
ASC_NODISCARD asc_span asc_span_from_str(char* str);

/**
 * @brief ASC_SPAN_LITERAL_FROM_BUFFER returns a literal asc_span over a byte buffer.
 * The length of the resulting asc_span is set to 0.
 * The capacity of the resulting asc_span is set to the count of items the buffer can store based on
 * the sizeof(BYTE_BUFFER). For example:
 *
 * uint8_t buffer[1024];
 * const asc_span buf = ASC_SPAN_LITERAL_FROM_BUFFER(buffer);  // Len=0, Cap=1024
 */
#define ASC_SPAN_LITERAL_FROM_BUFFER(BYTE_BUFFER) \
  { \
    ._internal = { \
      .ptr = (uint8_t*)BYTE_BUFFER, \
      .length = 0, \
      .capacity = (sizeof(BYTE_BUFFER) / sizeof(BYTE_BUFFER[0])), \
    }, \
  }

/**
 * @brief ASC_SPAN_FROM_BUFFER returns an asc_span expression over an uninitialized byte buffer. For
 * example:
 *
 * uint8_t buffer[1024];
 * some_function(ASC_SPAN_FROM_BUFFER(buffer));  // Len=0, Cap=1024
 */
#define ASC_SPAN_FROM_BUFFER(BYTE_BUFFER) (asc_span) ASC_SPAN_LITERAL_FROM_BUFFER(BYTE_BUFFER)

/**
 * @brief ASC_SPAN_LITERAL_FROM_INITIALIZED_BUFFER returns a literal asc_span over an initialized byte
 * buffer. For example:
 *
 * uint8_t buffer[] = { 1, 2, 3 };
 * const asc_span buf = ASC_SPAN_LITERAL_FROM_INITIALIZED_BUFFER(buffer); // Len=3, Cap=3
 */
#define ASC_SPAN_LITERAL_FROM_INITIALIZED_BUFFER(BYTE_BUFFER) \
  { \
    ._internal = { \
      .ptr = BYTE_BUFFER, \
      .length = (sizeof(BYTE_BUFFER) / sizeof(BYTE_BUFFER[0])), \
      .capacity = (sizeof(BYTE_BUFFER) / sizeof(BYTE_BUFFER[0])), \
    }, \
  }

/**
 * @brief ASC_SPAN_FROM_INITIALIZED_BUFFER returns an asc_span expression over an initialized byte
 * buffer. For example
 *
 * uint8_t buffer[] = { 1, 2, 3 };
 * const asc_span buf = ASC_SPAN_LITERAL_FROM_INITIALIZED_BUFFER(buffer); // Len=3, Cap=3
 */
#define ASC_SPAN_FROM_INITIALIZED_BUFFER(BYTE_BUFFER) \
  (asc_span) ASC_SPAN_LITERAL_FROM_INITIALIZED_BUFFER(BYTE_BUFFER)

/******************************  SPAN MANIPULATION */

/**
 * @brief asc_span_slice returns a new asc_span which is a sub-span of the specified span.
 *
 * @param[in] span The original asc_span.
 * @param[in] start_index An index into the original asc_span indicating where the returned asc_span
 * will start.
 * @param[in] end_index An index into the original asc_span indicating where the returned asc_span
 * should stop. The byte at the high_index is NOT included in the returned asc_span.
 * @return An asc_span into a portion (from \p start_index to \p end_index - 1) of the original
 * asc_span.
 */
ASC_NODISCARD asc_span asc_span_slice(asc_span span, int32_t start_index, int32_t end_index);

/**
 * @brief asc_span_is_content_equal returns `true` if the lengths and bytes referred by \p span1 and
 * \p span2 are identical.
 *
 * @return Returns true if the lengths of both spans are identical and the bytes in both spans are
 * also identical.
 */
ASC_NODISCARD ASC_INLINE bool asc_span_is_content_equal(asc_span span1, asc_span span2)
{
  return asc_span_length(span1) == asc_span_length(span2)
      && memcmp(asc_span_ptr(span1), asc_span_ptr(span2), (size_t)asc_span_length(span1)) == 0;
}

/**
 * @brief asc_span_is_content_equal_ignoring_case returns `true` if the lengths and characters
 * referred to by \p span1 and \p span2 are identical except for case. This function assumes the
 * bytes in both spans are ASCII characters.
 *
 * @return Returns true if the lengths of both spans are identical and the ASCII characters in both
 * spans are also identical except for case.
 */
ASC_NODISCARD bool asc_span_is_content_equal_ignoring_case(asc_span span1, asc_span span2);

/**
 * @brief asc_span_to_str copies a source span containing a string (not 0-terminated) to a
 destination char buffer and appends the 0-terminating byte.
 *
 * The buffer referred to by destination must have a size that is at least 1 byte bigger
 * than the \p source asc_span. The string \p destination is converted to a zero-terminated str.
 Content
 * is copied to \p source buffer and then \0 is added at the end. Then out_result will be created
 out
 * of buffer
 *
 * @param[in] destination A pointer to a buffer where the string should be copied
 * @param[in] destination_max_size The maximum available space within the buffer referred to by
 destination.
 * @param[in] source The asc_span containing the not-0-terminated string
 * @return An #asc_result value indicating the result of the operation.
 *          #ASC_OK If \p source span content is successfully copied to the destination.
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the \p destination buffer is too small to
 copy the string and 0-terminate it
 */
ASC_NODISCARD asc_result
asc_span_to_str(char* destination, int32_t destination_max_size, asc_span source);

/******************************  SPAN PARSING */

/**
 * @brief asc_span_to_uint64 parses an asc_span containing ASCII digits into a uint64 number
 *
 * @param[in] span The asc_span containing the ASCII digits to be parsed.
 * @param[in] out_number The pointer to the variable that is to receive the number
 * @return An #asc_result value indicating the result of the operation.
 *          #ASC_OK if successful
 *          #ASC_ERROR_PARSER_UNEXPECTED_CHAR if a non-ASCII digit is found within the span.
 */

ASC_NODISCARD asc_result asc_span_to_uint64(asc_span span, uint64_t* out_number);

/**
 * @brief asc_span_to_uint32 parses an asc_span containing ASCII digits into a uint32 number
 *
 * @param span The asc_span containing the ASCII digits to be parsed.
 * @param out_number The pointer to the variable that is to receive the number
 * @return An #asc_result value indicating the result of the operation.
 *          #ASC_OK if successful
 *          #ASC_ERROR_PARSER_UNEXPECTED_CHAR if a non-ASCII digit
 * is found within the span.
 */
ASC_NODISCARD asc_result asc_span_to_uint32(asc_span span, uint32_t* out_number);

/**
 * @brief asc_span_find searches for `target` in `source`, returning an #asc_span within `source` if
 * it finds it.
 *
 * @param[in] source The #asc_span with the content to be searched on.
 * @param[in] target The #asc_span containing the token to be searched in `source`.
 * @return The position of `target` in `source` if `source` contains `target`,
 *         0 if `target` is empty (if its length is equal zero),
 *         -1 if `source` is empty (if its length is equal zero) and `target` is non-empty,
 *         -1 if `target` is not found in `source`.
 */
ASC_NODISCARD int32_t asc_span_find(asc_span source, asc_span target);


/******************************  SPAN APPENDING */

/**
 * @brief asc_span_append_uint8 appends the uint8 \p byte to the \p destination starting at the
 * destination span length.
 *
 * @param[in] destination The asc_span where the byte should be appended to.
 * @param[in] byte The uint8 to append to the destination span
 * @return An #asc_span that is a clone of the \p destination span with its length increased by 1.
 *         The method assumes that the \p destination has a large enough capacity to hold one more
 * byte.
 */
asc_span asc_span_append_uint8(asc_span destination, uint8_t byte);

/**
 * @brief asc_span_append appends the bytes referred to by the source span into the
 * destination span starting at the destination span length.
 *
 * @param[in] destination The asc_span where the bytes should be appended to.
 * @param[in] source Refers to the bytes to be appended to the destination
 * @return An #asc_span that is a clone of the \p destination span with its length increased by the
 * \p source length. The method assumes that the \p destination has a large enough capacity to hold
 * the \p source.
 */
asc_span asc_span_append(asc_span destination, asc_span source);

/**
 * @brief asc_span_append_i32toa appends an int32 as digit characters to the destination
 * starting at the destination span length.
 *
 * @param[in] destination The asc_span where the bytes should be appended to
 * @param[in] source The int32 whose number is appended to the destination span as ASCII
 * digits
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation.
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the \p destination is not big enough to
 * contain the appended bytes
 */
ASC_NODISCARD asc_result
asc_span_append_i32toa(asc_span destination, int32_t source, asc_span* out_span);

/**
 * @brief asc_span_append_u32toa appends a uint32 as digit characters to the destination
 * starting at the destination span length.
 *
 * @param[in] destination The asc_span where the bytes should be appended to
 * @param[in] source The uint32 whose number is appended to the destination span as ASCII
 * digits
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation:
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the destination is not big enough to
 * contain the appended bytes
 */
ASC_NODISCARD asc_result
asc_span_append_u32toa(asc_span destination, uint32_t source, asc_span* out_span);

/**
 * @brief asc_span_append_i64toa appends an int64 as digit characters to the destination
 * starting at the destination span length.
 *
 * @param[in] destination The asc_span where the bytes should be appended to
 * @param[in] source The int64 whose number is appended to the destination span as ASCII
 * digits
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation:
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the destination is not big enough to
 * contain the appended bytes
 */
ASC_NODISCARD asc_result
asc_span_append_i64toa(asc_span destination, int64_t source, asc_span* out_span);

/**
 * @brief asc_span_append_u64toa appends a uint64 as digit characters to the destination
 * starting at the destination span length.
 *
 * @param[in] destination The asc_span where the bytes should be appended to
 * @param[in] source The uint64 whose number is appended to the destination span as ASCII
 * digits
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation:
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the destination is not big enough to
 * contain the appended bytes
 */
ASC_NODISCARD asc_result
asc_span_append_u64toa(asc_span destination, uint64_t source, asc_span* out_span);

/**
 * @brief asc_span_append_dtoa appends a double as digit characters to the destination
 * starting at the destination span length
 *
 * @param[in] destination The asc_span where the bytes should be appended to
 * @param[in] source The double whose number is appended to the destination span as ASCII
 * digits
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation:
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the destination is not big enough to
 * contain the appended bytes
 */
ASC_NODISCARD asc_result asc_span_append_dtoa(asc_span destination, double source, asc_span* out_span);

/******************************  SPAN COPYING */

/**
 * @brief asc_span_set sets all the bytes of the destination span (up to its capacity) to \p
 * fill.
 *
 * @param[in] destination The span whose bytes will be set to \p fill
 * @param[in] fill The byte to be replicated within the destination span
 */
ASC_INLINE void asc_span_set(asc_span destination, uint8_t fill)
{
  memset(asc_span_ptr(destination), fill, (size_t)asc_span_capacity(destination));
}

/**
 * @brief asc_span_copy copies the content of the source span to the destination span
 *
 * @param[in] destination The span whose bytes will be replaced by the source's bytes
 * @param[in] source The span containing the bytes to copy to the destination
 * @return An #asc_span that is a clone of the \p destination span with its length updated to the \p
 * source length. The method assumes that the \p destination has a large enough capacity to hold the
 * \p source.
 */
asc_span asc_span_copy(asc_span destination, asc_span source);

/**
 * @brief asc_span_copy_url_encode Copies a URL in the source span to the destination span by
 * url-encoding the source span characters.
 *
 * @param[in] destination The span whose bytes will receive the url-encoded source
 * @param[in] source The span containing the non-url-encoded bytes
 * @param[out] out_span A pointer to an asc_span that receives the span referring to the
 * destination span with its length updated
 * @return An #asc_result value indicating the result of the operation.
 *          #ASC_OK if successful
 *          #ASC_ERROR_INSUFFICIENT_SPAN_CAPACITY if the \p destination is not big enough to
 * hold the source's length
 */
ASC_NODISCARD asc_result
asc_span_copy_url_encode(asc_span destination, asc_span source, asc_span* out_span);

/******************************  SPAN PAIR  */

/**
 * An asc_pair represents a key/value pair of asc_span instances.
 * This is typically used for HTTP query parameters and headers.
 */
typedef struct
{
  asc_span key;
  asc_span value;
} asc_pair;

/**
 * @brief The ASC_PAIR_NULL macro returns an asc_pair instance whose key and value fields are
 * initialized to ASC_SPAN_NULL
 *
 */
#define ASC_PAIR_NULL \
  (asc_pair) { .key = ASC_SPAN_NULL, .value = ASC_SPAN_NULL }

/**
 * @brief asc_pair_init returns an asc_pair with its key and value fields initialized to the specified
 * key and value parameters.
 *
 * @param[in] key A span whose bytes represent the key
 * @param[in] value A span whose bytes represent the key's value
 * @return  An asc_pair with the field initialized to the parameters' values
 */
ASC_NODISCARD ASC_INLINE asc_pair asc_pair_init(asc_span key, asc_span value)
{
  return (asc_pair){ .key = key, .value = value };
}

/**
 * @brief asc_pair_from_str returns an asc_pair with its key and value fields initialized to span's
 * over the specified key and value 0-terminated string parameters.
 *
 * @param[in] key A string representing the key
 * @param[in] value A string representing the key's value
 * @return  An asc_pair with the field initialized to the asc_span instances over the passed-in
 * strings
 */
ASC_NODISCARD ASC_INLINE asc_pair asc_pair_from_str(char* key, char* value)
{
  return asc_pair_init(asc_span_from_str(key), asc_span_from_str(value));
}

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_SPAN_H
