// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file "asc_security_core/asc/asc_precondition_internal.h".h
 *
 * @brief This header defines the types and functions your application uses
 *        to override the default precondition failure behavior.
 *
 *        Public SDK functions validate the arguments passed to them in an effort
 *        to ensure that calling code is passing valid values. The valid value is
 *        called a contract precondition. If an SDK function detects a precondition
 *        failure (invalid argument value), then by default, it calls a function that
 *        places the calling thread into an infinite sleep state; other threads
 *        continue to run.
 *
 *        To override the default behavior, implement a function matching the
 *        asc_precondition_failed_fn function signature and then, in your application's
 *        initialization (before calling any Azure SDK function), call
 *        asc_precondition_failed_set_callback passing it the address of your function.
 *        Now, when any Azure SDK function detects a precondition failure, it will invoke
 *        your callback instead. You might override the callback to attach a debugger or
 *        perhaps to reboot the device rather than allowing it to continue running with
 *        unpredictable behavior.
 *
 *        Also, if you define the NO_PRECONDITION_CHECKING symbol when compiling the SDK
 *        code, all of the Azure SDK precondition checking will be excluding making the
 *        binary code smaller and faster. We recommend doing this before you ship your code.
 */

#ifndef _asc_PRECONDITION_INTERNAL_H
#define _asc_PRECONDITION_INTERNAL_H

#include "asc_security_core/asc/asc_precondition.h"

#include <stdbool.h>
#include <stddef.h>

#include "asc_security_core/asc/asc_span.h"

#include "asc_security_core/asc/_asc_cfg_prefix.h"

asc_precondition_failed_fn asc_precondition_failed_get_callback();

#ifdef NO_PRECONDITION_CHECKING
#define ASC_PRECONDITION(condition)
#else
#define ASC_PRECONDITION(condition) \
  do \
  { \
    if (!(condition)) \
    { \
      asc_precondition_failed_get_callback()(); \
    } \
  } while (0)
#endif

#define ASC_PRECONDITION_RANGE(low, arg, max) ASC_PRECONDITION((low <= arg && arg <= max))

#define ASC_PRECONDITION_NOT_NULL(arg) ASC_PRECONDITION((arg != NULL))
#define ASC_PRECONDITION_IS_NULL(arg) ASC_PRECONDITION((arg == NULL))


ASC_NODISCARD ASC_INLINE bool asc_span_is_valid(asc_span span, int32_t min_length, bool null_is_valid)
{
  int32_t span_length = asc_span_length(span);
  int32_t span_capacity = asc_span_capacity(span);
  /* Valid Span is:
      If the length is greater than or equal to a user defined minimum value AND one of the
     following:
        - If null_is_valid is true and the pointer in the span is null, the length and capacity must
     also be 0. In the case of the pointer not being NULL, two conditions must be met:
        - The length is greater than or equal to zero and the capacity is greater than or equal to
          the length.
  */
  return (
      (((null_is_valid && (asc_span_ptr(span) == NULL) && (span_length == 0) && (span_capacity == 0))
        || (asc_span_ptr(span) != NULL && (span_length >= 0) && (span_capacity >= span_length)))
       && min_length <= span_length));
}

#define ASC_PRECONDITION_VALID_SPAN(span, min, null_is_valid) \
//  ASC_PRECONDITION(asc_span_is_valid(span, min, null_is_valid))

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_PRECONDITION_INTERNAL_H
