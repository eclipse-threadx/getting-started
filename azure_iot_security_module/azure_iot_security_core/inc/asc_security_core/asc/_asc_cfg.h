// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file _asc_cfg.h
 *
 * @brief Disable warnings.
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifdef _MSC_VER

// warning C4710: '...': function not inlined
#pragma warning(error : 4710)

// warning C4204: nonstandard extension used: non-constant aggregate initializer
#pragma warning(disable : 4204)

// warning C4221: nonstandard extension used: '...': cannot be initialized using address of
// automatic variable '...'
#pragma warning(disable : 4221)

// warning C4996: This function or variable may be unsafe. Consider using ..._s instead.
#pragma warning(disable : 4996)

// warning C4820: '<unnamed-tag>': '4' bytes padding added after data member '...'
#pragma warning(disable : 4820)

// warning C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch
// specified
#pragma warning(disable : 5045)

// warning C4214: nonstandard extension used: bit field types other than int
// https://stackoverflow.com/questions/2280492/bit-fields-of-type-other-than-int
#pragma warning(disable : 4214)

// warning C6011: Dereferencing NULL pointer. Using ASC_PRECONDITION_NOT_NULL
#pragma warning(disable : 6011)

// warning C6387: 'str' could be '0':  this does not adhere to the specification for the function
// 'strlen' Using ASC_PRECONDITION_NOT_NULL
#pragma warning(disable : 6387)

#endif // _MSC_VER

#ifdef __GNUC__

#pragma GCC diagnostic ignored "-Wmissing-braces"

#endif // __GNUC__

#ifdef __clang__

#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wmissing-braces"

#endif // __clang__

#ifndef _asc_CFG_H
#define _asc_CFG_H

#ifdef _MSC_VER
#define ASC_INLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__) // !_MSC_VER
#define ASC_INLINE __attribute__((always_inline)) static inline
#else // !_MSC_VER !__GNUC__ !__clang__
#define ASC_INLINE static inline
#endif // _MSC_VER

#if defined(__GNUC__) && __GNUC__ >= 7
#define ASC_FALLTHROUGH __attribute__((fallthrough))
#else // !__GNUC__ >= 7
#define ASC_FALLTHROUGH \
  do \
  { \
  } while (0)
#endif // __GNUC__ >= 7

#ifdef _MSC_VER
#define ASC_NODISCARD _Check_return_
#elif defined(__GNUC__) || defined(__clang__) // !_MSC_VER
#define ASC_NODISCARD __attribute__((warn_unused_result))
#else // !_MSC_VER !__GNUC__ !__clang__
#define ASC_NODISCARD
#endif // _MSC_VER

// Get the number of elements in an array
#define _asc_COUNTOF(array) (sizeof(array) / sizeof(array[0]))

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#endif // _asc_CFG_H
